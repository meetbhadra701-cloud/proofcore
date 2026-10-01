# Architecture Decision Log

Decisions apply to Milestone 1 and are reflected in `SPEC.md`. Any change requires a technical reason, validation evidence where possible, and a corresponding spec update.

## D-001 — Five-stage, single-issue, in-order pipeline

- **Problem:** The project needs a nontrivial teaching core with clear commit ordering and manageable future formal state.
- **Alternatives:** Single-cycle/multicycle core, deeper pipeline, superscalar or out-of-order design.
- **Decision:** IF/ID/EX/MEM/WB, single issue, in order, explicit validity at each boundary.
- **Rationale:** This architecture makes operand hazards, recovery, and retirement visible while keeping control simple enough to inspect.
- **Tradeoffs:** One instruction per cycle is the best-case throughput; no speculation or variable latency.

## D-002 — Split, fixed-latency memory interfaces

- **Problem:** Fetch and load/store timing must be concrete without adding a bus subsystem.
- **Alternatives:** Shared port, ready/valid bus, caches, variable wait states.
- **Decision:** Separate combinational-read instruction and data ports; stores commit on a rising edge; no backpressure.
- **Rationale:** IF and MEM can work in parallel and testbenches can supply a small deterministic memory.
- **Tradeoffs:** No arbitration, wait states, or realistic external bus timing.

## D-003 — Deterministic no-op policy for unsupported instructions and misalignment

- **Problem:** M1 excludes trap/privilege machinery, but software and tests still need deterministic behavior outside the supported subset.
- **Alternatives:** Illegal-instruction/access trap, sticky halt, unspecified behavior, inert sequential no-op.
- **Decision:** Unsupported encodings retire without side effects and advance by four. Misaligned halfword/word accesses are suppressed with no destination or memory effect. A taken branch/jump to a non-4-byte-aligned target is suppressed; it has no link write and retires sequentially.
- **Rationale:** Avoids adding trap state while making all out-of-subset behavior testable and safe from partial updates.
- **Tradeoffs:** These policies differ from full RISC-V exception behavior. Programs must not depend on the omitted traps; the final report must state this prominently.

## D-004 — Synchronous active-high reset and cleared integer registers

- **Problem:** Startup state must be deterministic for early simulations.
- **Alternatives:** Asynchronous reset, uninitialized x1-x31, reset control state only.
- **Decision:** Reset on rising edge, PC=0, all stage valid bits=0, x1-x31 cleared; x0 hardwired zero; memories not reset.
- **Rationale:** Simple simulation startup and explicit contract.
- **Tradeoffs:** Resetting 992 register bits can inhibit compact memory inference; physical optimization is outside M1.

## D-005 — Branch/jump resolution in EX

- **Problem:** Control flow must share correct forwarded operands and use one clear redirect point.
- **Alternatives:** Resolve in ID, prediction/recovery, or multicycle control.
- **Decision:** Resolve all conditional branches, JAL, and JALR in EX.
- **Rationale:** EX has forwarded operand values and can reuse comparison/adder logic.
- **Tradeoffs:** Younger IF/ID and ID/EX work must be flushed on redirect, with a fixed control penalty.

## D-006 — EX/MEM over MEM/WB forwarding and one load-use bubble

- **Problem:** The register file may not yet contain values needed by a younger instruction.
- **Alternatives:** Stall all dependencies; extra bypasses; compiler scheduling.
- **Decision:** Independently forward each source from the newest eligible producer, EX/MEM first, then MEM/WB; exclude x0 and invalid producers. Stall one cycle when a valid ID instruction depends on an EX load.
- **Rationale:** ALU results are available from EX/MEM; load data is only available after MEM and can then come from MEM/WB.
- **Tradeoffs:** Every producer and source-use condition must be decoded correctly; load-use remains a one-cycle penalty.

## D-007 — Separate retirement record, not RVFI

- **Problem:** Architectural effects need one stable observation point now and a clean adapter point later.
- **Alternatives:** Inspect internal state in tests; implement RVFI immediately.
- **Decision:** Publish one record at WB with valid, PC, instruction, destination/value/write-enable, memory address/masks/data, and architectural next PC.
- **Rationale:** Tests can check committed behavior independently of internal pipeline encoding.
- **Tradeoffs:** The record is project-specific and is not a formal interface or proof.

## D-008 — Tool harness and repository structure

- **Problem:** Initial remote was empty and the shell lacked EDA tools.
- **Alternatives:** One monolithic source file; no reproducible build; external hosted tools.
- **Decision:** Keep reusable RTL modules in `rtl/`, cocotb regression in `tb/cocotb/`, and commands in a root Makefile. Use locally extracted open-source tool packages where system installation is blocked.
- **Rationale:** Keeps source responsibilities clear and makes test commands reproducible without modifying system packages.
- **Tradeoffs:** Local tool extraction may reveal missing shared libraries; those failures must be reported rather than mistaken for test failures.

## D-009 — Packed vectors at module boundaries for frontend compatibility

- **Problem:** Yosys 0.52 accepts package-defined types internally but rejects package imports in module headers and typed struct ports. That blocked the requested synthesis check when all helper-module ports used package types.
- **Alternatives:** Drop Yosys validation; abandon shared enums/structs throughout; generate a separate translated RTL fork; keep named structs internally and flatten only module interfaces.
- **Decision:** Keep the canonical enums and packed pipeline/retirement structs in `proofcore_pkg.sv` and use fixed-width packed-vector ports for helper and pipeline-register modules. Convert back to named structs in `core.sv`.
- **Rationale:** One source remains the source for simulation, lint, and synthesis; names remain available for internal logic, while the open-source tool frontend can elaborate the boundaries.
- **Tradeoffs:** Fixed port widths must stay synchronized with the package struct definitions. Verilator lint checks connections; changes to payload structures must update the corresponding vector widths and be revalidated in Yosys.

## External processor implementation consulted

None. No third-party CPU RTL was copied.
