# Formal environment assumptions

These constraints define the environment used by the Milestone 3 checks. They
do not model a complete RISC-V execution environment and they do not state that
any DUT output is correct. The corresponding proof records are in
`formal/results/results.json`.

| Assumption | Why required | What it excludes | Spec match | Bugs it could hide |
| --- | --- | --- | --- | --- |
| Instruction and data interface addresses have `addr[31:5] == 0`. | The formal memories contain eight 32-bit words (32 bytes) so the solver can retain stable symbolic contents and store/load consistency at low cost. | Programs and data accesses outside byte addresses 0–31. | This is a harness limitation, not a processor address-map claim. The M1 core itself has no address-map restriction. | High-address truncation, upper address-bit propagation, wraparound, and bugs only triggered by accesses outside the window. |
| Instruction-memory words are unconstrained `anyconst` values and do not change during a proof. | Models a stable combinational instruction ROM while keeping instructions symbolic instead of selecting one concrete test program. | Self-modifying code and nondeterministic instruction fetch. | Matches the M1 separate instruction-memory interface and no-cache contract. M1 does not define instruction-memory writes. | Bugs requiring self-modifying code or unstable fetch data. |
| Each data word starts as an independent unconstrained symbolic 32-bit value; enabled byte stores update that word and later reads observe the updated bytes. | Gives arbitrary initial data while preserving read-after-write consistency. | Variable-latency responses, bus errors, caches, external agents, and concurrent memory writers. | Matches M1's single-cycle, little-endian, single-core data-memory contract. | Bugs involving wait states, faults, races, or a more complex memory hierarchy. |
| ISA checks cover only naturally aligned control-flow destinations. For a taken branch, the target must be word aligned; JAL and JALR targets must be word aligned. | Pinned `riscv-formal` models the architectural instruction-address-misaligned trap. M1 explicitly does not implement that trap and freezes misaligned transfers as inert sequential instructions. | The M1 no-effect behavior for misaligned transfers and architectural misalignment traps. | This is an explicit restriction to the non-trapping, aligned subset of M1's supported instruction semantics. | A defect in alignment calculation or behavior that appears only for a misaligned target. The condition is derived from the instruction/source values and is called out because it narrows the checked executions. |
| ISA checks cover only naturally aligned halfword and word loads/stores. Byte accesses remain unrestricted within the memory window. | The upstream ISA model expects a misalignment trap, while M1 specifies no trap and suppresses misaligned halfword/word accesses. | M1's inert behavior for misaligned halfword/word accesses and architectural alignment traps. | This matches the explicitly supported aligned memory-operation subset; no trap compliance is claimed. | A defect in low address bits, subword mask logic, or suppression logic that appears only on misaligned accesses. |
| The internal-property harness asserts reset initially, then deasserts it after the first modeled core clock edge. | Initializes pipeline valid state, PC, and architectural registers before checking sequential invariants. | Arbitrary unreset power-up state and reset glitches. | Matches the documented synchronous active-high reset sequence used by ProofCore. | Reset sequencing defects or behavior before the reset edge. |

No assumption constrains forwarding selections, branch outcomes, retirement data,
register values, destination writes, store masks, or the result of any asserted
property. The instruction/data alignment constraints are limited to behaviors
M1 does not model with architectural traps.

## Memory model summary

`formal/harness/formal_memory.sv` creates eight symbolic instruction words and
eight symbolic initial data words. Stores update only byte lanes enabled by the
core mask. Reads are combinational from the selected current word. Instruction
words remain unchanged. This environment therefore proves instruction behavior
for arbitrary stable nearby code/data values, but says nothing about latency,
faults, cache coherence, concurrent agents, or full address-space behavior.
