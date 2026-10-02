# ProofCore Status

## Current position

- Milestone 1: **ENGINEERING COMPLETE; HUMAN LEARNING UNASSESSED.** Preserved at commit `54500b0a10da917249087fe246179b71066c1ced` (`milestone-1` tag).
- Milestone 2: **ENGINEERING COMPLETE; HUMAN LEARNING UNASSESSED.** Preserved at commit `456744b7a06a9492e3b498e0a56a6ec418c71a6a` (`milestone-2` tag).
- Milestone 3: **ENGINEERING COMPLETE; HUMAN LEARNING UNASSESSED.** Optional RVFI, pinned upstream ISA checks, internal bounded properties, covers, assumption/vacuity review, and formal negative controls are implemented and validated.
- Current task: Milestone 3 final evidence and learning pack are complete. Milestone 4 has not started.
- The processor top remains `core` in `rtl/core.sv`. RVFI is optional at compile time; the M1/M2 interface remains available when disabled.

## Milestone 3 implementation

- Optional RVFI retirement interface with standard RVFI signals and monotonically increasing retirement order.
- M2 independent reference model and lockstep driver compare RVFI retirement fields in directed and deterministic random campaigns.
- Pinned `riscv-formal` and SymbiYosys submodules, per-instruction configurations, a bounded formal wrapper and symbolic finite memories.
- Internal assertions cover x0 reads, retirement/order validity, forwarding value and priority, load-use stall behavior, redirect/flush behavior, and memory side-effect qualification.
- Reachability covers and two mutation-based negative controls demonstrate that the formal environment reaches important behaviors and detects injected faults.

## Final validation evidence

- `make verify`: PASS after M3 changes; preserves the ordinary M1/M2 regression with RVFI disabled.
- `make verify-m1`: PASS; nine Cocotb directed scenarios, 498 ns, lint, and synthesis checks.
- `make verify-m3-sim`: PASS; RVFI-directed nine programs / 90 retirements / 148 cycles and RVFI-random 500 programs / seeds 12345–12844 / 11,188 retirements / 15,521 cycles, alongside M2 comparisons.
- `make formal-rvfi`: 41/41 applicable upstream instruction and consistency checks PASS at bounded depth 12 (37 supported instruction checks plus four PC/register/order checks).
- `make formal-properties`: PASS, ABC `bmc3` bounded depth 16.
- `make formal-cover`: 13 cover statements reached in 12 retained traces, bounded depth 24, Z3 4.13.3.
- `make formal-negative`: both injected faults detected as expected: corrupted RVFI destination value and broken x0 reads.
- `make lint` and `make lint-rvfi`: PASS with Verilator 5.032, zero warnings.
- `make synth-check` and `make synth-check-rvfi`: PASS with Yosys 0.52; 8,556 and 9,296 generic cells respectively. These counts are not PPA results.
- Detailed commands, tools, assumptions, runtimes, and result files are in `docs/verification/milestone-3.md` and `formal/results/results.json`.

## Limitations and known issues

- Formal ISA checks cover the implemented 37-instruction subset and selected consistency checks, not all RV32I architectural features.
- Bounded BMC results are not induction or unbounded proofs. No claim of complete formal correctness is made.
- Formal models use finite symbolic instruction/data memories and assume bounded addresses plus aligned control-flow and halfword/word accesses. M1 does not implement architectural traps for misalignment, illegal instructions, or other exceptions.
- No caches, wait states, external memory agents, privilege behavior, interrupts, or full address-space behavior are modeled.
- Generic Yosys cell counts do not establish timing, area, or physical-design quality.
- No RTL architectural bug was found in this milestone. Early failures were in a formal property/clock harness and a mismatch between M1's no-trap alignment policy and upstream trap expectations; these are documented with evidence.

## Learning state

Milestone 1, 2, and 3 teach-back materials are present, but all remain **UNASSESSED**. Reading the material does not establish human mastery. See `docs/learning/milestone-3/FINAL_TEACHBACK.md` for the M3 questions.
