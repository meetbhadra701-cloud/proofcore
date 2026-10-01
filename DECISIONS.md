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

## Milestone 2 decisions

### D-010 — Snapshot M1 before adding the architectural oracle

- **Problem:** The repository had a fully tested M1 tree but no commit, so M2 changes could not be cleanly distinguished from the verified processor foundation.
- **Alternatives:** Start adding files without a baseline; recreate history; commit and tag the current tested snapshot.
- **Decision:** Commit the full M1 tree as `54500b0a10da917249087fe246179b71066c1ced` and tag it `milestone-1` before M2 implementation.
- **Rationale:** The commit is a reproducible rollback/reference point; no history was rewritten and no remote push was made.
- **Tradeoffs:** The snapshot author is recorded as Codex because the repository had no local Git author configuration.

### D-011 — Use a one-step architectural model with independent decode

- **Problem:** A checker that reproduces RTL pipeline and control decisions could share the same bug and miss it.
- **Alternatives:** Translate decoder RTL into software; bind to RTL package constants; independently decode the public ISA encodings and execute one complete instruction per step.
- **Decision:** `reference/src/rv32i_decode.cpp` and `rv32i_model.cpp` implement a sequential architectural interpreter in C++20, with no RTL includes or pipeline state.
- **Rationale:** State and execution rules differ structurally from the five-stage implementation, so forwarding/stall/flush errors can show up as retirement disagreement.
- **Shared assumptions:** The public RISC-V opcode and field bit positions are necessarily the same. The independent decoder hard-codes those ISA values and unit tests them; it does not import ProofCore control enums or tables. M1 unsupported/misaligned policies are read from `SPEC.md` and mirrored explicitly.
- **Tradeoffs:** The two implementations can still share an ISA interpretation mistake. Directed boundary tests and later an optional independently trusted ISA simulator/formal checks can reduce that risk.

### D-012 — Keep RTL and reference memories separate

- **Problem:** Sharing one host memory object can hide store timing errors or accidentally apply a store twice.
- **Alternatives:** One shared memory updated by either side; update the reference on the RTL store pin; give each implementation a separate copy and synchronize only their initial byte image.
- **Decision:** Build two independent byte memories from the same deterministic initializer and per-case byte overrides. RTL memory changes only from `dmem_write_mask/data` on an active edge. Reference memory changes only inside the architectural store step. Compare load results/metadata and all final memory bytes.
- **Rationale:** Each implementation produces its own state evolution after a common initial condition; disagreement cannot be repaired by sharing a store update.
- **Tradeoffs:** The harness needs an explicit memory adapter and final-state comparison, and the fixed single-cycle M1 port is assumed.

### D-013 — Compare meaningful retirement fields, not interface don't-cares

- **Problem:** Raw bitwise comparison of every flattened output would flag irrelevant values and could obscure architectural mismatches.
- **Alternatives:** Compare all fields unconditionally; compare only final registers; compare each valid retirement with field relevance based on write/memory masks.
- **Decision:** Compare PC, instruction, register-write enable/destination, destination value when written, next PC, all masks, effective address when a memory mask is active, and store data for stores. Compare final byte-memory state too.
- **Rationale:** The contract captures every architectural operation while avoiding non-semantic output bits.
- **Tradeoffs:** The relevancy rules are part of the verification contract and must be updated if the retirement interface changes.

### D-014 — Use deterministic xorshift64* cases and stop-PC bounds

- **Problem:** Random tests need replayable seeds, broad hazard coverage, and bounded control flow.
- **Alternatives:** Unseeded host randomness; Python's evolving default helpers; a small specified xorshift64* generator plus instruction templates with a known stop PC.
- **Decision:** Each case gets a recorded 64-bit seed and tier; xorshift64* selects template values. Backward loops use an initialized small counter; every run stops at a declared architectural PC and has separate instruction/cycle limits.
- **Rationale:** A failing machine-code image can be recreated exactly and hangs are reported as timeouts.
- **Tradeoffs:** The generator explores constrained structured programs, not arbitrary binary-space coverage.

### D-015 — Treat comparator corruption as explicit test-only mode

- **Problem:** A passing checker alone does not show that failure plumbing works.
- **Alternatives:** Trust normal passes; mutate production RTL; inject a deliberate difference only into the expected event behind an explicit CLI flag.
- **Decision:** `--selftest-corrupt-retire N` flips an expected field at retirement N, triggers the normal mismatch artifact, and exits nonzero. The standard Make target checks that expected failure and artifact exist.
- **Rationale:** It exercises comparator and artifact plumbing without modifying RTL/model semantics or enabling corruption in normal runs.
- **Tradeoffs:** Self-test failure artifacts are not hardware failures and identify themselves as test-only corruption.
