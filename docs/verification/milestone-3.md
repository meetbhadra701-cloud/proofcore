# Milestone 3 Verification Record

## Baseline

- M2 baseline commit: `456744b7a06a9492e3b498e0a56a6ec418c71a6a`.
- Tag: `milestone-2` (also `origin/main` at the start of M3).
- Pre-change `make verify`: PASS, including M1, M2 reference/directed/self-test/random gates.
- Final post-M3 `make verify`: PASS.
- Final `make verify-m1`: PASS; one Cocotb regression, nine scenarios, 498 ns; Verilator lint; Yosys synthesis/check.

## RVFI simulation validation

- RVFI-disabled M1/M2 regression: `make verify` passed after M3 RTL changes.
- RVFI-enabled directed: nine M1-derived programs, 90 retirements, 148 cycles; PASS.
- RVFI-enabled random: 500 programs, seeds 12345–12844, 11,188 retirements, 15,521 cycles; PASS.
- Compared instruction, PC/next PC, source IDs/values, destination, and meaningful memory fields against M2's independent reference model.
- This is empirical cross-checking, not sequential equivalence.

## `riscv-formal` instruction checks

- Framework: YosysHQ `riscv-formal`, commit `c992aa61fdfe0846c5ed90324c596202a1c69b76`.
- SymbiYosys: commit `b1a1e98cba941ec8433f8dc27f416cd7bb7f14be`.
- Yosys: 0.52, git SHA `fee39a3284c90249e1d9684cf6944ffbbcbb8f90`.
- Engine: ABC `bmc3`, depth 12.
- Result: 37 supported-instruction checks PASS, 0 FAIL.
- Supported set: ADD/SUB/SLL/SLT/SLTU/XOR/SRL/SRA/OR/AND; ADDI/SLTI/SLTIU/XORI/ORI/ANDI/SLLI/SRLI/SRAI; LB/LH/LW/LBU/LHU; SB/SH/SW; BEQ/BNE/BLT/BGE/BLTU/BGEU; JAL/JALR/LUI/AUIPC.
- Not claimed: full RV32I, SYSTEM/CSR/FENCE, privilege behavior, illegal-instruction traps, misalignment traps, compressed instructions, or unbounded correctness.
- Per-check runtime sum: 540.73 s for the 37 instructions. Full ISA/RVFI consistency campaign total runtime is recorded in `formal/results/results.json`.

## PC / ordering checks

- `pc_fwd_ch0`: PASS, depth 12.
- `pc_bwd_ch0`: PASS, depth 12.
- `unique_ch0`: PASS, depth 12.
- These complement the internal order shadow counter; the order starts at zero and advances only on valid WB retirement.

## Internal architectural properties

- Command: `make formal-properties`.
- Engine: ABC `bmc3`; bounded BMC depth 16; runtime 14.16 s.
- Result: PASS.
- Properties cover x0 read-port values, order/valid correspondence, retirement validity, nonzero destination writes, load/store side-effect gating, mutually exclusive read/write retirement masks, and destination-write qualification.
- This is bounded proof evidence; no induction or unbounded proof is claimed.

## Pipeline / hazard properties

- PASS at depth 16: EX/MEM and MEM/WB selected values match the forwarding mux output.
- PASS at depth 16: EX/MEM has priority over MEM/WB for eligible matching nonzero producers.
- PASS at depth 16: load-use stall holds PC and IF/ID, captures an invalid ID/EX bubble, and later permits retirement.

## Control-flow properties

- PASS at depth 16: redirect updates PC to its target, invalidates younger IF/ID and ID/EX, and preserves the redirecting instruction in EX/MEM.
- The associated side-effect assertions require valid memory operations and valid retirement. Upstream PC checks cover supported aligned instruction semantics.

## Memory-side-effect properties

- PASS at depth 16: data reads require a valid EX/MEM load; nonzero write masks require a valid EX/MEM store; retiring load/store masks are mutually exclusive; stores do not claim a register write.
- Symbolic memory uses stable initial values and persistent masked byte writes.

## Cover properties

- Command: `PATH=/path/to/z3/bin:$PATH make formal-cover`.
- Engine: Z3 4.13.3 through `smtbmc`; bounded cover depth 24; runtime 17.70 s.
- Result: 13 cover statements reached in 12 preserved traces.
- Scenarios: EX/MEM forwarding, MEM/WB forwarding, priority conflict, load-use stall, taken/not-taken branch, JAL, JALR, load, store, retirement after stall, retirement after redirect, and attempted x0 write.
- Trace files: `artifacts/formal/m3-core-cover-depth24/`; details and shortest observed frames are in `docs/formal/COVERAGE.md`.

## Vacuity audit

- Completed in `docs/formal/VACUITY_AUDIT.md`.
- The internal cover run demonstrates all 13 listed trigger scenarios are reachable under its assumptions.
- The per-instruction framework assumes the selected instruction model is active; every supported operation has its own check result.

## Negative controls

- Command: `PATH=/path/to/z3/bin:$PATH make formal-negative`.
- RVFI corruption: flip bit 0 of `rvfi_rd_wdata`; upstream `insn_addi_ch0` fails at step 12 on `addi x16,x0,8` (expected `0x8`, observed `0x9`); runtime 96.49 s. Expected formal FAIL observed; self-test PASS.
- x0 corruption: force both x0 read ports to one; both `regfile.sv:26-27` assertions fail at step 3; runtime 2.18 s. Expected formal FAIL observed; self-test PASS.
- VCD, witnesses, property names, statuses, and logs are preserved under `artifacts/formal/m3-negative-*`.

## M1 regression

- `make verify-m1`: PASS.
- Verilator 5.032 `--lint-only --sv -Wall`: PASS, zero warnings.
- Cocotb/Verilator: one regression, nine directed scenarios, 498 ns; PASS.
- Yosys 0.52: synthesis and structural check PASS; final `check` has zero problems; generic netlist 8,556 cells.

## M2 differential regression

- C++20 reference unit tests: PASS, four suites, strict `-Werror` compile.
- Directed M1-derived differential: PASS, nine programs, 90 retirements, 148 cycles.
- Comparator self-test: PASS; injected retirement mismatch detected and artifact produced/replayed.
- Random differential: PASS, 500 programs, 11,188 retirements, 15,521 cycles.
- `make verify-m3-sim`: PASS, including RVFI-directed and RVFI-random comparisons above.

## Verilator lint

- Ordinary core: `make lint`, PASS, Verilator 5.032, `-Wall`, zero warnings.
- RVFI-enabled core: `make lint-rvfi`, PASS, Verilator 5.032, `-Wall`, zero warnings.

## Yosys synthesis

- Ordinary core: `make synth-check`, PASS, Yosys 0.52, 8,556 generic cells.
- RVFI-enabled core: `make synth-check-rvfi`, PASS, Yosys 0.52, 9,296 generic cells.
- Generic cell totals are not PPA, timing, area, or technology-mapped results.

## Assumptions and limitations

- Seven source-level `assume` sites in the RVFI wrapper bound instruction/data addresses and exclude misaligned control-flow/data accesses; the internal harness has two matching finite-address assumptions. Upstream per-check assumptions select the instruction being checked and selected register/PC/order context.
- Full details—including possible hidden bugs—are in `formal/ASSUMPTIONS.md` and `docs/formal/assumption-audit.md`.
- Formal memory is an eight-word instruction ROM and eight-word symbolic data memory. M1 trap behavior is not implemented. No caches, wait states, memory faults, external agents, or upper address-space behavior are covered.
- No unbounded proof was performed. Bounded PASS and cover reachability are reported separately.
- No RTL architectural bug was found. Early formal failures were a bad x0 property/clock harness and an upstream alignment-trap scope mismatch; the property issue and its preserved witness are recorded in `docs/formal/bugs/001-formal-harness-property-and-clock.md` and `artifacts/formal/m3-core-properties-first-attempt/`.
