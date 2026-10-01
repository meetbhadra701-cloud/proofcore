# ProofCore Milestone Plan

## Baseline

Milestone 1 is preserved at commit `54500b0a10da917249087fe246179b71066c1ced`, tag `milestone-1`. Its full baseline command `make verify` passed before M2 work began. M1 acceptance is retained and remains required through `make verify-m1`.

## Milestone 1

M1 processor implementation, directed cocotb regression, Verilator lint, Yosys synthesis check, specification, and learning pack are complete. Exact baseline evidence is in `docs/verification/milestone-1.md` and `docs/MILESTONE_1_REPORT.md`.

## Milestone 2 tasks

| Task | Scope | Acceptance evidence | Command | State |
| --- | --- | --- | --- | --- |
| M2.0 | Preserve and rerun M1 baseline | Commit/tag captured; all M1 gates pass | `make verify-m1` | Complete |
| M2.1 | Independent C++ architectural state, memory, decoder, and step model | C++20 strict-warning build and unit tests for all supported instruction families | `make reference-test` | Complete |
| M2.2 | Specify retirement comparison and memory synchronization | Independent memories, explicit meaningful-field rules, final byte-state comparison | Review `docs/architecture/milestone-2.md` | Complete |
| M2.3 | Verilator driver and retirement lockstep comparator | RTL runs from matching program/memory image; every valid retirement compared | `make diff-directed` | Complete |
| M2.4 | M1 directed differential corpus | M1 scenario translations pass and preserve original regression | `make diff-directed` and `make verify-m1` | Complete |
| M2.5 | Deterministic program generator | Six tiers, fixed seeds, bounded control flow, manifest | `tools/programgen/generate.py` | Complete |
| M2.6 | Structured failure artifact | Seed, machine code, disassembly, memory image, states/events, recent trace | Automatic on mismatch | Complete |
| M2.7 | Comparator self-test | Injected expected-field corruption fails and emits ordinary artifact | `make comparator-selftest` | Complete |
| M2.8 | Random campaign | 500 seeded programs, exact counts and distribution recorded | `make diff-random` | Complete |
| M2.9 | Final gates and report | All M1/M2 gates pass; counts and limitations recorded | `make verify` | Complete |
| M2.10 | Professor-mode learning pack | All requested guides, code walkthrough, end-to-end example, 50 unassessed questions | Documentation review | Complete |

## Validation commands

```sh
make verify-m1
make reference-test
make diff-directed
make comparator-selftest
make diff-random
make verify
```

`make verify` is the top-level gate. It runs M1 lint/simulation/synthesis plus the independent model tests, directed differential tests, self-test, and the configured random campaign. Random seeds and tier distribution are stored in the generated corpus manifest; use `M2_SEED` and `M2_PROGRAMS` to choose a reproducible campaign.

## Current status

Milestone 2 engineering and teaching materials are complete. The final integrated `make verify` passed, and actual outcomes are frozen in `docs/verification/milestone-2.md` and `docs/MILESTONE_2_REPORT.md`. Human learning remains unassessed until the teach-back is answered.
