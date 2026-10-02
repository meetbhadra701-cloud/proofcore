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

## Milestone 3 tasks

| Task | Scope | Acceptance evidence | Command | State |
| --- | --- | --- | --- | --- |
| M3.0 | Verify M2 baseline and pin formal dependencies | `make verify`, commit/tag and exact submodule SHAs recorded | `make verify` | Complete before M3 edits |
| M3.1 | Specify RVFI and formal environment contract | Optional interface, trap/alignment boundary, assumptions reviewed | `formal/ASSUMPTIONS.md`, `formal/rvfi/README.md` | Complete |
| M3.2 | Implement standard optional RVFI fields and order | RTL lint plus retirement lockstep for directed and random cases | `make diff-rvfi-directed diff-rvfi-random` | Complete |
| M3.3 | Add pinned upstream ISA/RVFI checks | All 37 supported-instruction checks plus applicable reg/PC/order checks report individually | `make formal-rvfi` | Complete: 41/41 upstream checks pass |
| M3.4 | Add internal architectural/pipeline properties | x0, retirement, forwarding, stall, redirect, and memory effect properties pass at recorded bound | `make formal-properties` | Complete: bounded BMC depth 16 passes |
| M3.5 | Add cover reachability and trace retention | All listed trigger scenarios reach within depth 24; traces preserved | `make formal-cover` | Complete: 13 cover statements reached |
| M3.6 | Audit assumptions and vacuity | Every constraint enumerated and each major assertion family paired with reachable scenarios | `docs/formal/assumption-audit.md`, `docs/formal/VACUITY_AUDIT.md` | Complete |
| M3.7 | Test formal failure plumbing | RVFI-result and x0-property mutations each produce an expected formal FAIL | `make formal-negative` | Complete: both mutants detected |
| M3.8 | Protect M1/M2 and synthesis gates | M1 lint/simulation/synthesis and M2 model/directed/random gates pass with RVFI off/on | `make verify-m3-sim`, `make verify-m1`, `make synth-check` | Complete: baseline `make verify`, M1, M2 and RVFI-on/off regressions pass |
| M3.9 | Freeze evidence and professor-mode learning pack | Machine-readable results, verification record, report, all requested lessons and 70 unassessed questions | Documentation review | Complete |

## M3 validation commands

```sh
make verify
make verify-m3-sim
make formal-rvfi
make formal-properties
PATH=/path/to/z3/bin:$PATH make formal-cover
PATH=/path/to/z3/bin:$PATH make formal-negative
make verify-m3
```

The ISA and internal BMC checks use ABC `bmc3` supplied with pinned Yosys; they
are bounded, not induction. Cover and expected-failure trace runs use Z3 and
need `z3` on `PATH`. Keep the long formal campaign separate from `make verify`
so the familiar regression stays quick; `make verify-m3` is the full M3 gate.

## Final M3 status

All individual validation gates are recorded in `docs/verification/milestone-3.md`;
machine-readable per-check results are in `formal/results/results.json`. The
aggregate `make verify-m3` target is provided for reproduction, while its
component gates were run and recorded individually. No M4 work has started.
