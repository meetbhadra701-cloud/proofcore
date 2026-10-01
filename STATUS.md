# ProofCore Status

## Current position

- Milestone 1: **ENGINEERING COMPLETE; HUMAN LEARNING UNASSESSED.** Preserved at `54500b0a10da917249087fe246179b71066c1ced` (`milestone-1` tag); `make verify` passed before M2 changes.
- Milestone 2: **ENGINEERING COMPLETE; HUMAN LEARNING UNASSESSED.** All requested M2 systems, final evidence, and learning materials are present.
- Current task: none. Recommended next work is the human teach-back and then Milestone 3 planning.
- RTL top remains `core` in `rtl/core.sv`; M2 has not modified processor RTL.

## Implemented M2 components

- Independent C++20 single-step reference state, decoder, byte memory, and retirement events under `reference/`.
- Verilator driver and semantic retirement comparator under `sim/diff/`.
- Separate, identically seeded RTL and reference memories; independent store application; per-load metadata/value comparison; final memory-byte comparison.
- Python deterministic xorshift64* generator with arithmetic, dependency, memory, load-use, control, and mixed tiers.
- Nine differential cases ported from the M1 Cocotb scenarios.
- Structured mismatch artifacts and explicit test-only comparator corruption mode.

## Final validation evidence

- `make verify`: PASS after final M2 code changes; includes M1 lint/simulation/synthesis and all M2 gates.
- `make reference-test`: PASS, four C++ unit suites under `-Wall -Wextra -Wpedantic -Werror`.
- `make diff-directed`: PASS, nine M1-derived programs, 90 retirements, 148 RTL cycles.
- `make comparator-selftest`: PASS as a Make target; injected x6 14→7 at retirement 2 produced the required mismatch/artifact and replayed.
- `make diff-random`: PASS, 500 programs, seeds 12345–12844, 11,188 retirement comparisons, 15,521 RTL cycles, no failures/timeouts.
- Exact tier counts, static instruction totals, tool versions, and commands are in `docs/verification/milestone-2.md`.
- Formal verification: NOT RUN. M2 differential testing is empirical and is not proof of correctness.

## Known tool and project limitations

- C++ compiler is GCC 15.2.0; Verilator 5.032; Python 3.14.4 in the current environment. The M1 Yosys 0.52 result is recorded separately.
- The installed Verilator 5.032 support source emits two `%p` format warnings under GCC 15.2; the Verilator harness build uses `-Wno-error=format` for that third-party runtime issue. The reference model and its tests still compile with full `-Werror`.
- The reference implements exactly the M1 subset and deterministic no-effect unsupported/misalignment policies. It does not model traps or privilege architecture.
- Random generation is structured and bounded, not exhaustive. The failure system does not minimize witnesses or localize root cause.

## Learning state

Milestone 1 and Milestone 2 teach-back questions remain `UNASSESSED`. Reading AI-generated explanations is not evidence of human mastery.
