# ProofCore Milestone 1 Engineering Report

## Result

**MILESTONE 1: ENGINEERING COMPLETE**

**MILESTONE 1: HUMAN LEARNING UNASSESSED**

The five-stage RV32I subset is implemented and passed the required simulation, lint, and synthesis gates. The human has not yet answered the teach-back questions; no understanding is inferred from the generated material.

## Architecture

ProofCore is a single-issue, in-order IF/ID/EX/MEM/WB core. Pipeline registers carry explicit validity and stage-specific payloads. Branches and jumps resolve in EX; EX/MEM forwarding has priority over MEM/WB; a load-use dependency holds PC/IF-ID and injects one invalid ID/EX slot. WB is the architectural write and retirement point.

Instruction and data memories are separate, fixed-latency interfaces. Instruction reads are combinational. Data memory presents a containing aligned 32-bit word; stores commit masked byte lanes on a rising edge. Only naturally aligned halfword/word data accesses are supported.

## Supported ISA

The supported subset is the exact list in `SPEC.md`: RV32I register and immediate ALU operations; LB/LH/LW/LBU/LHU; SB/SH/SW; BEQ/BNE/BLT/BGE/BLTU/BGEU; JAL/JALR; LUI/AUIPC. RV32M, floating point, compressed instructions, atomics, privilege, traps, interrupts, caches, virtual memory, prediction, superscalar, and out-of-order execution are excluded.

## Repository map

| Path | Purpose |
| --- | --- |
| `rtl/proofcore_pkg.sv` | Enums and packed decode/pipeline/retirement types |
| `rtl/alu.sv`, `rtl/imm_gen.sv` | ALU operations and I/S/B/U/J immediate construction |
| `rtl/decoder.sv`, `rtl/regfile.sv` | ISA control decode and architectural registers |
| `rtl/forwarding.sv`, `rtl/hazard_unit.sv` | Bypass selection and load-use interlock |
| `rtl/pipeline/*.sv` | Valid-aware IF/ID, ID/EX, EX/MEM, MEM/WB state |
| `rtl/core.sv` | Core integration, branch/target, memory lanes, writeback, retirement |
| `tb/cocotb/test_core.py` | Nine directed program scenarios with format encoders and memory model |
| `scripts/run_cocotb.py`, `Makefile` | Reproducible build, test, lint, synthesis commands |
| `docs/architecture/` | Pipeline diagram and decoder table |
| `docs/verification/` | Test coverage and observed gate results |
| `docs/learning/milestone-1/` | Complete human learning pack and teach-back |

The RTL contains 11 SystemVerilog modules across 12 RTL source files, including one shared type package.

## Verification inventory and results

- Cocotb 2.1.0: one regression test runs nine directed program scenarios and checks 90 expected valid retirement events across dynamic traces. It checks all listed ALU/load/store/branch/jump operations, x0, forwarding priority, stalls, flushes, retirement, and subset policies.
- Verilator 5.032: `make lint` passes with `-Wall` and no warnings.
- Yosys 0.52 with ABC: `make synth-check` reads, checks, and synthesizes the core; final `check` reports zero problems. Generic output is 8,556 cells, including 992 resettable register-file bits.
- Icarus 12.0: exploratory direct compile emitted packed-struct width warnings; Cocotb was not run. It is NOT counted as a pass.
- Formal verification: NOT RUN. No formal correctness claim is made.

## Important decisions

- Use an explicit-valid five-stage pipeline and retire in WB.
- Resolve branches/jumps in EX and flush only younger IF/ID and ID/EX entries.
- Use EX/MEM then MEM/WB forwarding; exclude x0 and do not forward a load's address.
- Use a one-cycle load-use interlock for fixed single-cycle memory.
- Use split instruction/data memory without backpressure.
- Reset synchronously, set PC to zero, clear pipeline validity and x1-x31, and leave memory unchanged.
- Make unsupported encodings and misaligned accesses deterministic inert no-ops because M1 omits traps; this is not complete RISC-V exception compliance.
- Keep named package structs internally while using packed vectors at module boundaries for the installed Yosys frontend.

Full alternatives, rationale, and tradeoffs are in `DECISIONS.md`.

## Bugs discovered and fixes

- The first store-data implementation shifted the whole 32-bit source for SB/SH. Although masks protected architectural memory, the retirement data contained unrelated source bits. The RTL now masks the source to 8 or 16 bits before lane shifting, and the regression checks exact lane-positioned data.
- The testbench initially sampled the retirement record after the writeback edge. It now observes the WB record before the edge that commits it, matching the synchronous register-file model.
- Directed expected values for unsigned comparison and load-use cycle counts were corrected after checking the ISA semantics; no correctness checks were weakened.
- Yosys compatibility required vector module boundaries and extraction of its separate ABC package. The RTL contract was preserved and the final synthesis completed.

## Reproduction commands

```sh
make test
make lint
make synth-check
make verify
```

Tool versions and detailed outcomes are in `docs/verification/milestone-1.md`. In the authoring environment, tools were extracted under `/tmp/proofcore-tools/root` because administrator authentication prevented system installation; on another machine install Verilator, Yosys/ABC, Python, and Cocotb normally before running these commands.

## Known limitations

There are no traps, interrupts, privilege modes, variable-latency or shared memory, split misaligned accesses, compressed instructions, RV32M, atomics, floating point, caches, branch prediction, out-of-order execution, RVFI, ProofTrace, formal proof infrastructure, PPA, or synthesis equivalence. The deterministic no-op behavior for unsupported and misaligned cases is a project subset rule. Generic Yosys cell totals are not a physical design result.

## Read first

1. `docs/learning/milestone-1/00-big-picture.md`
2. `docs/learning/milestone-1/01-riscv-basics.md`
3. `docs/learning/milestone-1/06-five-stage-pipeline.md`
4. `docs/learning/milestone-1/08-forwarding.md` and `09-hazards-and-stalls.md`
5. `docs/learning/milestone-1/13-how-an-instruction-flows.md`
6. `docs/learning/milestone-1/16-important-code-walkthrough.md`
7. `docs/learning/milestone-1/FINAL_TEACHBACK.md`

## Recommended Milestone 2 starting point

Choose a versioned RV32I reference source and define the exact retirement-to-reference comparison contract. Then design an RVFI adapter around the existing WB retirement record and build one instruction-level differential test before adding formal properties. Keep M1's subset deviations explicit when defining the reference boundary.
