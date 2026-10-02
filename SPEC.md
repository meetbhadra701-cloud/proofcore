# ProofCore Milestone 1 Specification

Status: frozen for Milestone 1 implementation per project owner instruction, 2026-09-30.

## Purpose and scope

Milestone 1 builds a small, in-order, single-issue, five-stage RV32I processor that can run aligned bare-metal programs in a simulation environment. The design favors explicit validity, understandable control, and a clean retirement observation point. This document is the behavioral contract for M1. A change must be recorded in `DECISIONS.md` with its technical reason and reflected here.

Milestone 1 does not claim formal correctness. It does not add RVFI, ProofTrace, internal assertion infrastructure, automated failure localization, mutation testing, witness minimization, synthesis equivalence, PassWitness, physical-design/PPA flows, or automated RTL optimization.

## Architectural contract

- XLEN is 32 bits. Instructions are fixed-width 32-bit words; `IALIGN=32`.
- Integer architectural registers are `x0`–`x31`. `x0` always reads as zero and ignores writes.
- The machine is single-issue, in-order, and non-privileged. It has no operating-system or privilege-mode support.
- The pipeline stages are IF, ID, EX, MEM, and WB. Each stage carries an explicit `valid` bit. Invalid stage contents have no architectural effect.
- Instructions retire in program order at WB. A valid instruction that has not been flushed retires once. A bubble never retires.
- The initial fetch PC after reset is `0x00000000`. Sequential execution advances by four bytes.
- The supported instruction set is the RV32I integer subset listed below. All other encodings, including SYSTEM/CSR, FENCE, and ECALL/EBREAK, are unsupported.

### Supported instructions

| Family | Instructions |
| --- | --- |
| Register ALU | ADD, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND |
| Immediate ALU | ADDI, SLTI, SLTIU, XORI, ORI, ANDI, SLLI, SRLI, SRAI |
| Loads | LB, LH, LW, LBU, LHU |
| Stores | SB, SH, SW |
| Conditional branches | BEQ, BNE, BLT, BGE, BLTU, BGEU |
| Control transfer | JAL, JALR |
| Upper immediate | LUI, AUIPC |

M-extension arithmetic, floating point, compressed instructions, atomics, privilege modes, virtual memory, caches, interrupts, and out-of-order execution are out of scope.

### Immediate construction and operation details

All instruction fields below are bit slices of the 32-bit instruction word. `sext(value)` means sign extension to 32 bits. The implemented immediate forms are:

| Format | Constructed 32-bit immediate |
| --- | --- |
| I | `sext({insn[31:20]})` |
| S | `sext({insn[31:25], insn[11:7]})` |
| B | `sext({insn[31], insn[7], insn[30:25], insn[11:8], 1'b0})` |
| U | `{insn[31:12], 12'b0}` |
| J | `sext({insn[31], insn[19:12], insn[20], insn[30:21], 1'b0})` |

The B and J forms are signed byte offsets with bit 0 equal to zero. Shift-immediate instructions use `insn[24:20]` as a five-bit shift amount; the upper immediate bits must match the RV32I encoding for SLLI, SRLI, or SRAI. Arithmetic right shift sign-fills from operand bit 31; logical right shift fills with zero. Signed comparisons interpret operands as two's-complement 32-bit values; unsigned comparisons interpret the same bits as values from 0 through `2^32-1`. ADD/SUB and ADDI use 32-bit modular results. SLT/SLTI are signed; SLTU/SLTIU are unsigned (the SLTIU immediate is first sign-extended as an I-immediate, then compared unsigned).

### Instruction validity and unsupported encodings

An instruction enters IF/ID as valid when fetched. A valid encoding that is not in the supported table is treated as an inert sequential no-op: it produces no register or memory write, retires as a valid instruction, and advances to `PC+4`. Unsupported encodings do not raise a trap or halt the core in M1. This deterministic policy is a project subset choice, not full privileged-architecture compliance.

Instruction fetch is expected to provide a 32-bit instruction for an aligned address. JALR clears target bit 0 as required by the base ISA. If a taken branch/jump would target an address with bit 1 set, the transfer is suppressed: the instruction retires as an inert sequential no-op, with no link-register write and `next_pc=pc+4`. No instruction-address-misaligned trap is implemented.

## Stage responsibilities

| Stage | Responsibility |
| --- | --- |
| IF | Present the current PC to instruction memory; form `PC+4`; accept a redirect from EX; hold the PC during a front-end stall. |
| ID | Read instruction fields, produce decode controls and immediate, read two register operands, and identify unsupported encodings. |
| EX | Select forwarded operands, perform ALU and branch comparisons, calculate effective addresses and control-transfer targets, and decide taken redirects. |
| MEM | Issue the data-memory operation for loads/stores; select and extend returned load bytes/halfwords; carry non-memory results forward. |
| WB | Select the architectural write value, write `rd` when enabled, and publish the Milestone-1 retirement record. |

The pipeline is edge-triggered: stage state advances on the active rising edge except when a documented stall holds it or a flush invalidates it. A bubble is represented by `valid=0`, never by relying on an all-zero instruction encoding.

### Pipeline register payloads

| Register | Payload carried with valid state |
| --- | --- |
| IF/ID | instruction PC, instruction bits |
| ID/EX | PC, PC+4, instruction bits, rs1/rs2/rd indices, both register values, decoded immediate, ALU operation, operand-source controls, register-write control, branch/jump control, memory operation/size, writeback selection, decode legality |
| EX/MEM | PC, PC+4, instruction bits, destination index, destination-write control, ALU result/effective address, store source value, memory operation/size, branch/jump decision and architectural next PC, writeback selection |
| MEM/WB | PC, instruction bits, destination index, final destination value/write control, effective memory address, memory read/write masks and write data, architectural next PC |

Each stage register resets to `valid=0`. Advance captures the incoming payload and validity. Hold preserves current state. Flush sets validity to zero and clears control fields; payload bits may be retained because invalid entries cannot cause effects.

## External memory behavior

The core uses separate instruction and data interfaces, allowing one fetch and one data access to be presented in the same cycle. The interfaces are simulation-friendly, single-cycle, and have no ready/valid backpressure:

- Instruction memory is combinational-read: an aligned byte PC selects a 32-bit instruction word. The environment supplies stable contents for the current PC during the cycle.
- Data memory is byte-addressed, little-endian, and word organized. The core calculates an effective byte address, then presents the containing word's aligned byte address (`effective_address[31:2]` with low bits zero), a read enable for loads, a 4-bit byte write mask, and 32-bit write data. When read enable is asserted, a load receives the aligned 32-bit word combinationally; the core selects the addressed lane(s) using the effective address and performs sign/zero extension. Non-load instructions have read enable deasserted.
- Stores update the selected byte lanes on the active edge at the MEM boundary. `SB`, `SH`, and `SW` generate masks from `effective_address[1:0]`; write data is shifted into the addressed byte lanes. The memory port sees the containing word address, while the retirement record retains the architectural effective byte address.
- Naturally aligned byte, halfword, and word accesses are supported. A misaligned halfword/word load or store is suppressed as an inert sequential no-op: no destination write, no memory access, and `next_pc=pc+4`. No split-access mechanism or misalignment trap is provided.
- Memory contents are owned by the testbench/system, not reset by the core. The instruction and data memories may be separate arrays in simulation.

## Reset and register-file semantics

- Reset is synchronous, active-high, and sampled on the rising edge.
- While reset is asserted at a rising edge, the PC becomes zero, all pipeline valid bits become zero, and all architectural register bits become zero (with `x0` remaining hardwired to zero). Reset does not modify memory.
- The register file has two combinational read ports and one rising-edge write port. Reads of `x0` return zero. Writes to `x0` are discarded.
- A same-cycle WB write is visible to an ID read of the same register (write-through/read-new semantics), so the instruction captured into ID/EX sees the value retiring on that edge. The forwarding network still handles older in-flight dependencies.

## Control flow, stalls, and flushes

- Conditional branches, JAL, and JALR resolve in EX. The branch base is the branch's own PC; branch target is `PC + sign_extended_B_imm`. JAL target is `PC + sign_extended_J_imm`. JALR target is `(rs1 + sign_extended_I_imm) & 0xFFFF_FFFE`.
- Branch comparisons use equality for BEQ/BNE, signed 32-bit comparison for BLT/BGE, and unsigned 32-bit comparison for BLTU/BGEU. Signedness is explicit in the implementation contract.
- JAL and JALR write `PC+4` to `rd`. Branches do not write a register.
- A taken EX redirect selects the target PC and invalidates younger instructions in IF/ID and ID/EX before they can have effects. The redirecting instruction and older instructions remain valid. A not-taken branch does not flush.
- EX/MEM forwarding has priority over MEM/WB forwarding when both match an EX source. A match is eligible only when the producer is valid, writes a nonzero destination register, and has a value available from that stage. A load result is not considered available from EX/MEM; it is forwarded from MEM/WB once returned.
- A load-use dependency is detected when the instruction in ID/EX is a load whose nonzero `rd` is needed by the valid IF/ID instruction. Insert one bubble into ID/EX and hold PC and IF/ID for one cycle. On the following cycle, MEM/WB forwarding supplies the loaded value to EX. The design has no variable-latency memory, so one stall is sufficient.
- Branch operands use the same EX forwarding paths. A branch dependent on a load receives the load-use stall before its EX comparison.
- If stall and redirect conditions coincide, a taken EX redirect wins: discard the younger front-end instruction and set PC to the redirect target. A load-use stall cannot suppress resolution of an older instruction already in EX.

## Memory instruction behavior

Effective address is `rs1 + sign_extended_I_imm` for loads and `rs1 + sign_extended_S_imm` for stores. The two low address bits select the byte lane within the 32-bit word.

- LB selects one byte and sign-extends bit 7; LBU zero-extends it.
- LH selects the addressed two-byte halfword and sign-extends bit 15; LHU zero-extends it.
- LW returns the full 32-bit word.
- SB writes one selected byte; SH writes the two selected bytes; SW writes all four bytes.
- Load retirement reports a read mask for the accessed byte lanes. Store retirement reports the byte write mask and lane-positioned write data. Non-memory instructions report zero memory masks and no memory access.

The decoder must only enable memory writes for valid supported store instructions. A bubble or unsupported encoding has no memory effect.

## Milestone-1 retirement interface

This is an internal observation record, not RVFI. It describes one instruction at WB and must be zero/invalid when no instruction retires:

| Field | Meaning |
| --- | --- |
| `valid` | A valid instruction retires this cycle. |
| `pc` | Address of the retiring instruction. |
| `insn` | Original 32-bit instruction word. |
| `rd` | Destination register number; zero when no architectural destination is used. |
| `rd_value` | Value written to `rd`; zero when `rd_write=0`. |
| `rd_write` | Architectural register write enable after excluding `x0`. |
| `mem_addr` | Effective byte address for a load/store; zero otherwise. |
| `mem_read_mask` | Four byte-lane read mask for a load; zero otherwise. |
| `mem_write_mask` | Four byte-lane write mask for a store; zero otherwise. |
| `mem_write_data` | Store data positioned in the addressed lanes; zero otherwise. |
| `next_pc` | Architectural next PC: sequential PC or the taken branch/jump target. |

For a valid unsupported/no-op instruction, `valid=1`, `rd_write=0`, both memory masks are zero, and `next_pc=pc+4`.

The same no-effect retirement convention applies to a misaligned data access and a suppressed misaligned control transfer. Its reported destination register is zero, destination value is zero, memory address/masks/data are zero, and `next_pc=pc+4`.

The top-level SystemVerilog module exposes these record members as individual packed ports rather than a struct port. The internal named `retire_t` type keeps the record definition centralized; the flattened boundary works with the supported simulator and synthesis frontends.

## M1 acceptance boundary and limitations

M1 is complete only after representative instruction programs, directed hazard/control/memory tests, lint, simulation regression, and a basic Yosys synthesis check pass, and all required learning records exist. Simulation success is not a formal proof. M1 supports aligned accesses, single-cycle memories, fixed-width instructions, and no architectural trap/interrupt behavior. These limits must remain visible in status and the final report.

## Milestone-3 verification addendum

M3 does not change the M1 architectural contract. When `PROOFCORE_RVFI` is
defined, the core additionally exposes the standard RVFI fields for XLEN=32,
ILEN=32, and one retirement per cycle. `rvfi_valid` coincides with the valid
MEM/WB retirement event; `rvfi_order` starts at zero after reset and increments
only for that event. Source data represents the architectural values used by
the retiring instruction after forwarding. Memory masks use aligned 32-bit
little-endian lanes, and load read data is the aligned word read before any
store update at that edge. The ordinary M1 retirement record and behavior stay
unchanged when the define is absent.

RVFI reports M-mode (`rvfi_mode=2'b11`) and RV32 (`rvfi_ixl=2'b01`) to match
the upstream RVFI encoding, but this does not add privilege state. Trap, halt,
and interrupt outputs are zero because M1 has no such architecture. M3 does not
claim illegal-instruction or misalignment-trap compliance. The exact formal
subset and the assumptions that exclude these unsupported trap cases are in
`formal/ASSUMPTIONS.md`.
