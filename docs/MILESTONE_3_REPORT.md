# ProofCore Milestone 3 Report

## Result

**MILESTONE 3: ENGINEERING COMPLETE.** The optional RVFI interface, supported-instruction ISA checks, internal bounded properties, covers, assumption audit, and fault-detection controls are implemented and validated. **MILESTONE 3: HUMAN LEARNING UNASSESSED.** The teach-back remains for the human to answer; this report makes no claim about mastery.

Milestone 4 has not started. This milestone does not claim unbounded proof or complete RV32I compliance.

## Architecture and implementation

ProofCore remains a single-issue, in-order five-stage RV32I pipeline (IF, ID, EX, MEM, WB). Milestone 3 adds an optional standard-shaped RVFI observation interface at architectural retirement, without changing the default M1/M2 core interface. RVFI retirement fields are produced from the instruction committing at WB, and the 64-bit order counter advances only for a valid retirement.

The interface is checked against the independent Milestone 2 C++ reference model. The formal environment wraps the core with bounded instruction/data memories, reset sequencing, explicit environmental assumptions, upstream `riscv-formal` checks, and internal pipeline properties. Core RTL and test-only fault injection are in `rtl/`; formal harnesses and configurations are in `formal/`; scripts and pinned source dependencies are at `scripts/` and `third_party/`.

The RVFI signal contract and framework integration are documented in `formal/rvfi/README.md`; the full M3 architecture choices are in `docs/architecture/milestone-3.md` and `DECISIONS.md`.

## Supported ISA checks

The 37 checked instructions are ADD, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND; ADDI, SLTI, SLTIU, XORI, ORI, ANDI, SLLI, SRLI, SRAI; LB, LH, LW, LBU, LHU; SB, SH, SW; BEQ, BNE, BLT, BGE, BLTU, BGEU; JAL, JALR, LUI, AUIPC. Four additional upstream consistency checks cover register, forward PC, backward PC, and unique ordering behavior.

No claim is made for SYSTEM/CSR/FENCE, privilege architecture, illegal-instruction traps, misalignment traps, compressed instructions, or other unsupported RV32I features.

## Validation results

| Validation | Result |
| --- | --- |
| Baseline `make verify` after M3 changes | PASS; M1/M2 regression with RVFI disabled |
| `make verify-m1` | PASS; nine Cocotb scenarios, 498 ns, lint, synthesis/check |
| `make verify-m3-sim` | PASS; M2 comparisons plus RVFI directed and random campaigns |
| RVFI directed | 9 programs, 90 retirements, 148 cycles; PASS |
| RVFI random | 500 programs, seeds 12345–12844, 11,188 retirements, 15,521 cycles; PASS |
| `make formal-rvfi` | 41/41 applicable upstream checks PASS at depth 12: 37 instruction checks and four consistency checks |
| `make formal-properties` | PASS, ABC `bmc3`, bounded depth 16, 14.16 s |
| `make formal-cover` | 13 cover statements reached across 12 traces, Z3 4.13.3, depth 24, 17.70 s |
| `make formal-negative` | PASS as a self-test: both injected bugs caused their expected formal failures |
| `make lint`, `make lint-rvfi` | PASS, Verilator 5.032, zero warnings |
| `make synth-check` | PASS, Yosys 0.52, 8,556 generic cells |
| `make synth-check-rvfi` | PASS, Yosys 0.52, 9,296 generic cells |

The 37 instruction checks took a total of 540.73 seconds; the four consistency checks took 50.87 seconds. The negative RVFI result-value mutant was caught by `insn_addi_ch0` at step 12 (`addi x16,x0,8`, expected `0x8`, observed `0x9`). The x0 read mutant violated both x0 read-port assertions at step 3. These establish that the checker can detect those representative faults, not that every possible fault is detectable.

`make verify-m3` is provided as a single reproduction target. At finalization, its constituent validation gates were run and recorded individually rather than duplicating the roughly ten-minute upstream ISA campaign through the aggregate target. The individual command outcomes above and full check list are in `docs/verification/milestone-3.md` and `formal/results/results.json`.

## Formal scope and assumptions

The upstream campaign used pinned YosysHQ `riscv-formal` commit `c992aa61fdfe0846c5ed90324c596202a1c69b76` and SymbiYosys commit `b1a1e98cba941ec8433f8dc27f416cd7bb7f14be`. Yosys was 0.52 (`fee39a3284c90249e1d9684cf6944ffbbcbb8f90`). Instruction and internal assertion checks use ABC `bmc3`; the internal properties are bounded to depth 16, and the upstream checks to depth 12. Covers and negative-control traces use Z3 4.13.3.

The formal memories are finite: eight instruction words and eight data words. The harness bounds addresses and assumes aligned control-flow targets and naturally aligned halfword/word memory accesses. This is necessary because the M1 design's frozen behavior suppresses misaligned effects without implementing traps, while the upstream checker expects architectural trap behavior for such cases. No assumption constrains the expected RVFI result to match the implementation. Assumptions, rationale, and possible hidden-bug risks are enumerated in `formal/ASSUMPTIONS.md` and reviewed in `docs/formal/assumption-audit.md` and `docs/formal/VACUITY_AUDIT.md`.

Bounded PASS means no counterexample was found within the checked bound under the stated model and assumptions. It is not induction, an unbounded proof, or a claim that the core is fully ISA-correct. Reachable covers are reported separately from assertion proofs.

## Important decisions and bugs found

- RVFI remains optional at compile time so existing M1/M2 users and regressions keep their original interface.
- Retirement is observed at WB, where architectural register/memory effects commit; order advances only on valid events.
- Misaligned accesses are excluded from this formal campaign rather than silently changing the frozen M1 architecture to add traps.
- The harness uses a global formal timestep to advance the actual core clock. An initial property attempt incorrectly addressed nonexistent `regs[0]`; the property was corrected to assert the architectural x0 read ports. An earlier solver-witness conversion failed, so the campaign was rerun with a supported solver path. Neither issue was an RTL defect. Evidence and diagnosis are in `docs/formal/bugs/001-formal-harness-property-and-clock.md` and `artifacts/formal/m3-core-properties-first-attempt/`.

Full architectural decisions are recorded as D-016 through D-020 in `DECISIONS.md`.

## Known limitations

- No unbounded proof or induction was performed.
- Formal memory is finite and has no wait states, faults, cache behavior, or external agents.
- Misalignment, illegal instructions, privilege, traps, interrupts, and unsupported ISA extensions are not proven or implemented as full architectural behavior.
- Generic cell counts are synthesis sanity evidence only, not area, timing, or PPA.
- Mutation tests represent two targeted failure modes, not a complete sensitivity analysis.
- Human learning remains unassessed until the teach-back is completed.

## Reproduction commands

From the repository root, with Verilator, Yosys, C++20, Python, and the repository's pinned submodules available:

```sh
make verify
make verify-m1
make verify-m3-sim
make formal-rvfi
make formal-properties
PATH=/path/to/z3/bin:$PATH make formal-cover
PATH=/path/to/z3/bin:$PATH make formal-negative
make lint-rvfi
make synth-check-rvfi
make verify-m3
```

The Z3 path above must point to Z3 4.13.3 or another supported compatible version. Individual upstream check outcomes are stored in `formal/results/results.json`; preserved witnesses and traces are in `artifacts/formal/`.

## Learning materials

The complete M3 learning pack is under `docs/learning/milestone-3/`. Start with `00-big-picture.md`, then read `05-rvfi.md`, `08-assumptions.md`, `11-pipeline-invariants.md`, `18-important-code-walkthrough.md`, and `19-real-formal-trace.md`. The 70-question `FINAL_TEACHBACK.md` is deliberately marked UNASSESSED.

## Recommended next step

First complete the M3 teach-back with the implementation open, correcting any misunderstandings in the learning notes. Only then plan Milestone 4, beginning with a human-reviewed scope for formal property expansion or witness localization. Do not treat this milestone's bounded checks as unbounded correctness evidence.
