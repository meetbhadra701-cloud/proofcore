# Milestone 1 verification record

## Environment

- Verilator 5.032
- Cocotb 2.1.0
- Yosys 0.52 with ABC
- Python 3.14.0
- Tools were extracted from Ubuntu packages under `/tmp/proofcore-tools/root` because installing system packages required administrator authentication.

## Commands and results

| Command | Result | Evidence |
| --- | --- | --- |
| `make test` | PASS | Cocotb: `rv32i_foundation_regression` PASS, 0 failed, 498 ns; 9 directed programs |
| `make lint` | PASS | Verilator `--lint-only --sv -Wall`; zero warnings |
| `make synth-check` | PASS | Yosys hierarchy/process/synth/check; final check reports 0 problems |
| `make verify` | PASS | Full sequence of lint, simulation, and synthesis |

The 9 scenarios cover 90 valid retirement events across their executed traces, including repeated loop instructions. The tests check values and observable architectural effects rather than relying only on internal mux selections.

## Directed coverage

| Scenario | Directed cases checked |
| --- | --- |
| ALU/immediates | ADD/SUB, shifts, signed/unsigned comparisons, bitwise ops, I/S/B/U/J layouts, negative values, shift amount 0/31 |
| Register/forwarding | x0, x0 attempted write, EX/MEM forwarding, MEM/WB forwarding, EX/MEM priority, both operands, chains |
| Hazards | load-use on both sources, immediate consumer, load-to-branch, x0 false dependency, exact stall counts |
| Control | BEQ/BNE/BLT/BGE/BLTU/BGEU taken and not taken, negative B offset, JAL/JALR, link values, JALR bit0 clearing |
| Flush | Wrong-path register write and store suppressed; retirement PC stream excludes wrong-path instructions |
| Memory | LB/LBU/LH/LHU/LW, SB/SH/SW, lane offsets, sign/zero extension, byte masks and lane-positioned data, negative S offset |
| Subset behavior | Unsupported instruction and misaligned load/store/target cause no architectural effect |

## Synthesis notes

Yosys reported zero problems in both checks and synthesized the full `core` hierarchy. The final generic netlist reported 8,556 cells and 992 resettable register-file bits. This is generic logic information, not a technology-mapped area/timing report.

## Icarus status

Icarus 12.0 was locally extracted and a direct compile was attempted. It emitted packed-struct field width warnings and did not run the Cocotb regression. No Icarus pass is claimed. The required simulation evidence is from Verilator.

## Formal status

No formal property or equivalence proof was run. Simulation, lint, and synthesis do not establish formal correctness.
