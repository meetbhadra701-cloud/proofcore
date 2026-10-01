# M1.0 — Architecture and Behavioral Contract

## A. What did we build?

We wrote the contract for a small processor before writing processor hardware. It has five stages—IF, ID, EX, MEM, WB—and each stage says explicitly whether it contains a real instruction. The contract also defines how instructions reach memory, when branches change the PC, what reset does, and what counts as an instruction retiring.

## B. Why does a processor need it?

Without a shared contract, two modules can make incompatible assumptions. For example, a load unit might return an aligned word while the decoder assumes memory returns the selected byte. The architecture document says who selects the byte and when. That gives RTL authors, testbench authors, and the person reviewing results one agreed behavior to implement and check.

## C. Inputs and outputs

M1.0 creates no RTL ports. It defines the future external and stage-level signals:

| Signal/field | Width | Meaning and when it matters |
| --- | ---: | --- |
| `reset` | 1 | Synchronous active-high reset, sampled on a rising edge. |
| `pc` | 32 | Byte address of the instruction being fetched or carried with an instruction. |
| `instruction` / `insn` | 32 | Fixed-width RV32I instruction bits from instruction memory. |
| `valid` | 1 per pipeline stage | Says that stage holds an instruction allowed to cause effects. |
| `rs1`, `rs2`, `rd` | 5 each | Architectural register indices encoded in the instruction. |
| `rs1_value`, `rs2_value` | 32 each | Register operands, possibly replaced by forwarding in EX. |
| `immediate` | 32 | Sign- or zero-extended immediate according to instruction format and operation. |
| `effective_address` | 32 | Byte address used by a load/store; low bits select lanes. |
| `write_mask` | 4 | Selects the byte lanes written in the containing 32-bit memory word. |
| `write_data` / `rd_value` | 32 | Store lane data or architectural result, depending on the record. |
| retirement `mem_read_mask`, `mem_write_mask` | 4 each | Byte lanes accessed by the retiring load or store. |
| retirement `next_pc` | 32 | Sequential PC or control-transfer target of the retiring instruction. |

## D. Design decision

The contract uses separate, single-cycle instruction and data memories. This lets IF fetch while MEM accesses data, and keeps the first implementation free of bus arbitration and wait-state machinery. Conditional branches and jumps resolve in EX, where forwarded operands are available. A one-cycle interlock handles the one dependency that bypassing cannot solve immediately: a load whose data has not returned yet.

The document also freezes synchronous active-high reset, cleared registers, aligned halfword/word accesses, and a valid unsupported instruction that behaves as a sequential no-op. These are choices for this bounded milestone, not claims that they are the only valid CPU design. Their alternatives and costs are in `DECISIONS.md`.

## E. Important architecture walkthrough

In `docs/architecture/milestone-1.md`, the instruction path runs from instruction memory through IF and the IF/ID register into ID. The stage register carries the PC and instruction alongside a valid bit so later logic can attribute results to the original instruction.

The EX block has two jobs that must meet in the same cycle: choose the newest correct operand (register-file value or bypassed result) and calculate an ALU result, branch decision, or target. When a branch is taken, EX redirects IF and clears the younger IF/ID and ID/EX entries. Clearing validity prevents wrong-path operations from becoming register or memory writes even if their stored instruction bits are nonzero.

For a load-use pair, the consumer is still in ID when the load is in EX. The load's address can be calculated, but its data is not yet available. Holding PC and IF/ID while setting ID/EX valid low gives the load one cycle to move forward. The consumer then enters EX when the load value can be selected from MEM/WB.

The retirement record comes from WB, the point where architectural state changes. Its PC and instruction identify the event; write and memory fields show the effect; `next_pc` records the architectural control-flow result. This creates an observation seam for later work but has no RVFI compliance claim.

## F. Concrete worked example

For `addi x5, x0, 7` followed by `add x6, x5, x5`, the producer's result is not yet in the register file when the dependent instruction reaches EX:

| Cycle | `addi` | `add` | Key event |
| ---: | --- | --- | --- |
| 1 | IF | — | Fetch producer. |
| 2 | ID | IF | Read x0 and decode producer. |
| 3 | EX | ID | Producer computes 7; dependent instruction reads old x5. |
| 4 | MEM | EX | EX/MEM forwards 7 to both add operands; result is 14. |
| 5 | WB | MEM | x5 receives 7. |
| 6 | — | WB | x6 receives 14; both retire in order. |

Validity distinguishes the empty stages before cycle 1 and after each instruction leaves WB from actual instruction entries.

## G. Failure example

If a taken branch changes PC but leaves IF/ID valid, a wrong-path store could continue to MEM and corrupt memory. If a stage uses instruction bits equal to zero as its only bubble convention, that encoding may later be assigned an architectural meaning and the “empty” slot can accidentally act like an instruction. Explicit validity prevents both classes of ambiguity.

## H. How we checked it

M1.0 is a document-only task. The required check is a human-readable consistency review across `SPEC.md`, `docs/architecture/milestone-1.md`, `DECISIONS.md`, `PLAN.md`, and `STATUS.md`. The reset, memory, redirect, stall, flush, retirement, and no-op policies were compared across those files. RTL compilation and simulation do not apply because there is no RTL yet.

## I. Interview check

1. Why does every pipeline stage carry a valid bit?
2. Why can a normal ALU dependency use forwarding but a load-use dependency needs a stall?
3. Which instructions are younger than an EX-stage taken branch, and what must happen to them?
4. Why does retirement happen at WB in this design?
5. What does separate instruction/data memory simplify, and what does it leave out?
6. Why must the PC travel with an instruction through the pipeline?

<details><summary>Answer key</summary>

1. Validity distinguishes a real instruction from an empty slot, allowing stalls and flushes without accidental side effects.
2. An ALU result is available in EX/MEM; load data arrives later from memory, so the immediate consumer must wait until MEM/WB can forward it.
3. Instructions in IF and ID are younger. Their validity must be cleared before they can update architectural state.
4. WB is where register state is updated, giving one in-order point to observe committed effects.
5. It permits simultaneous fetch and data access; it omits shared-bus arbitration and wait-state behavior.
6. The PC is needed to calculate PC-relative targets and link values and to identify the retiring instruction.

</details>

## J. Mini exercise

Draw the stage occupancy for `lw x5, 0(x1)` followed immediately by `add x6, x5, x5`. Mark the cycle in which the pipeline inserts its invalid ID/EX entry and identify the stage that supplies x5 to the add.

## K. AI contribution disclosure

- **Agent implemented:** M1.0 project documents and architecture, then the Milestone-1 RTL, cocotb regression, and final learning pack.
- **Human implemented:** No RTL in this milestone. The human has not yet completed teach-back.
- **Agent reviewed:** Cross-document consistency, directed simulation behavior, Verilator lint, and Yosys synthesis.
- **Uncertain:** No formal proof or human teach-back has been completed. Unsupported/misaligned no-op behavior and the fixed-latency memory boundary are explicit project subset decisions, not full RISC-V exception compliance.
