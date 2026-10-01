# ProofCore Milestone 1 Plan

## Workflow and status

The project owner authorized autonomous completion of engineering before teaching. All M1 implementation and verification tasks are complete. The learning pack is also created; its teach-back remains unassessed until the human responds. Correctness checks were not weakened to obtain a pass.

## Completed stages

| Task | Scope | Acceptance evidence | Reproduction command | State |
| --- | --- | --- | --- | --- |
| M1.0 | Freeze architecture/specification | `SPEC.md`, architecture, decisions, pipeline payloads, unsupported/misaligned rules, and retirement contract agree | Manual consistency review | Complete |
| M1.1 | ISA enums, controls, pipeline/retirement types | `proofcore_pkg.sv` lint and elaboration as part of full core | `make lint` | Complete |
| M1.2 | ALU and I/S/B/U/J immediates | Directed operation, sign extension, shift, and branch-offset tests | `make test` | Complete |
| M1.3 | Register file | x0, synchronous write, two reads, WB write-through | `make test` | Complete |
| M1.4 | Decoder/control | Decode table and directed supported/unsupported instructions | `make test` | Complete |
| M1.5 | Pipeline state | Explicit valid, hold, bubble, reset and flush exercised by core programs | `make test` | Complete |
| M1.6 | Basic core | Independent and dependent instruction sequences retire correctly | `make test` | Complete |
| M1.7 | Forwarding | EX/MEM, MEM/WB, priority, two sources and x0 | `make test` | Complete |
| M1.8 | Load-use | Both source operands, load-to-branch, x0 false dependency, cycle checks | `make test` | Complete |
| M1.9 | Branches, jumps, flush | Both outcomes for each branch, JAL/JALR, wrong-path writes, negative offset | `make test` | Complete |
| M1.10 | Loads/stores | All widths and signedness, aligned byte lanes, masks and store data | `make test` | Complete |
| M1.11 | Retirement | Ordered PC/value/memory events and no bubble/wrong-path retirement | `make test` | Complete |
| M1.12 | Integration regression | Nine directed program scenarios pass | `make test` | Complete |
| M1.13 | Lint cleanup | Verilator 5.032 `-Wall`, no warnings | `make lint` | Complete |
| M1.14 | Synthesis sanity | Yosys 0.52 elaborates and synthesizes; final `check` reports zero problems | `make synth-check` | Complete |
| M1.15 | Final engineering audit | Spec/status/decisions/report and full teaching pack reflect the tested implementation | `make verify` plus documentation consistency review | Complete |

## Validation commands

```sh
make test
make lint
make synth-check
make verify
```

`make verify` runs lint, the Cocotb regression, and Yosys synthesis in sequence. The regression is one Cocotb test containing nine independently reset directed programs. It checks 90 expected valid retirement events over its traces. An Icarus 12.0 compile was explored but not counted because the locally extracted frontend emitted packed-struct width warnings; Verilator is the accepted simulator for this milestone.

## Remaining work

No engineering task remains in Milestone 1. Human learning is still unassessed; all 48 questions in `docs/learning/milestone-1/FINAL_TEACHBACK.md` are marked `UNASSESSED`. Milestone 2 should start by choosing and reviewing an ISA-level reference model and an RVFI adapter contract before adding formal infrastructure.
