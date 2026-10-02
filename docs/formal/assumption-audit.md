# Milestone 3 assumption audit

This review follows `formal/ASSUMPTIONS.md` and inspects both the ProofCore
wrapper and the selected upstream checks. It asks what each constraint buys,
what it removes from the state space, and whether it could mask a fault.

| Constraint family | Location | Necessary? | Audit result and possible blind spot |
| --- | --- | --- | --- |
| `imem_addr[31:5] == 0` | `formal/rvfi/proofcore_rvfi_wrapper.sv` | Yes for the eight-word instruction model. | Restricts proof addresses to 0–31. A fetch-address bug in upper bits or an execution requiring a larger image is outside this run. The check still allows any stable instruction bits at each word. |
| `dmem_addr[31:5] == 0` | RVFI wrapper and the same address check in the internal harness | Yes for the eight-word data model. | Restricts effective accesses to the finite model. It can hide high-address arithmetic/address-bit defects; it does not prescribe load values or store results. |
| Stable symbolic instruction words | `formal/harness/formal_memory.sv` (`anyconst`) | Yes to model a ROM rather than a new random instruction on every fetch. | Excludes self-modifying code and unstable fetch. It is consistent with M1's separate instruction interface. |
| Arbitrary symbolic initial data plus persistent masked stores | `formal/harness/formal_memory.sv` | Yes for meaningful memory semantics. | This is a model, not an assume: arbitrary initial words are retained and each write updates selected bytes. No bus errors, timing, or external agents are modeled. |
| Naturally aligned branch/JAL/JALR targets | `formal/rvfi/proofcore_rvfi_wrapper.sv` | Required to compare with upstream trap expectations while preserving the M1 no-trap contract. | Excludes misaligned-target behavior. The conditions use the retiring PC/instruction/source values, which narrows allowed traces and may hide a fault that only manifests by corrupting one of those values into an unaligned target. No condition assumes the expected PC result or branch outcome. |
| Naturally aligned halfword/word accesses | RVFI wrapper | Required because M1 suppresses these accesses without a trap but the upstream ISA model expects a trap. | Excludes M1's no-effect misalignment cases. Address is formed from the RVFI source value and instruction immediate, so a source/address defect that only creates a misaligned access can be excluded. Aligned byte/halfword/word semantics remain checked. |
| Reset sequencing | `formal/harness/formal_top.sv`; upstream generated check wrapper | Required to initialize synchronous state and start checking after reset. | Excludes glitches and arbitrary pre-reset behavior. The core-property harness asserts reset for its first core clock edge and then deasserts it. The upstream framework controls its own check reset. |
| Target instruction selection | Pinned framework `rvfi_insn_check.sv` (`assume(spec_valid)` while `check`) | Required to ask a per-instruction semantic question. | Restricts the selected retirement to the instruction model under test. It does not constrain that instruction's operands/result to the expected answer; those remain assertions. Other in-flight instructions remain symbolic and can affect dependencies/control flow. |
| Chosen retirement/order observation | Pinned `rvfi_reg_check.sv`, `rvfi_pc_*_check.sv`, `rvfi_unique_check.sv` | Required by these framework checkers to sample an RVFI retirement and relate the selected order to the trace. | Checks a selected retirement context rather than all executions in one monolithic proof. Per-instruction BMC, register consistency, PC consistency, and unique order are independent records in the manifest. |

### Minimization decision

The address window and alignment restrictions are the only ProofCore-specific
`assume` statements in the ISA wrapper (seven source-level assume sites: two
address bounds, three control-target cases, and two memory-width cases). They
remain because the finite memory and no-trap M1 contract are explicit. No
assumption constrains the expected ALU result, forwarding mux, load data, branch
comparison result, destination data, masks, retirement validity, or next PC.
Removing the bounds would require a larger/unbounded memory abstraction and
does not match this first formal model. Removing the alignment cases would test
trap behavior that the M1 interface intentionally does not implement.

The formal records must continue to say “bounded and aligned supported subset.”
Passing these checks does not prove the excluded misaligned/no-trap policy.
