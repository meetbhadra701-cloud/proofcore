# ProofCore Status

## Current position

- Milestone: 1 — Processor Foundation
- State: **ENGINEERING COMPLETE; HUMAN LEARNING UNASSESSED.** The complete implementation, directed simulation, Verilator lint, and Yosys synthesis gates pass. The requested learning pack and report are present.
- Current task: none; next work begins with human teach-back review or Milestone 2 planning.
- RTL top: `core` in `rtl/core.sv`.

## Implemented behavior

- Five-stage in-order RV32I subset with explicit valid state.
- All listed register/immediate ALU operations, aligned LB/LH/LW/LBU/LHU and SB/SH/SW, all six branches, JAL/JALR, LUI, and AUIPC.
- EX/MEM and MEM/WB forwarding with youngest-producer priority, x0 exclusion, and a one-cycle load-use interlock.
- EX-stage redirect, younger IF/ID and ID/EX flush, fixed-width PC update, and a WB Milestone-1 retirement record.
- Deterministic inert no-op policy for unsupported encodings and misaligned memory/control transfers.

## Validation evidence

- `make test`: PASS — 1 Cocotb regression test runs 9 directed program scenarios, checking 90 expected valid retirement events across the traces, exact retirement PCs/values, masks/data, memory bytes, stalls, and flushes.
- `make lint`: PASS — Verilator 5.032, `--lint-only --sv -Wall`, no warnings.
- `make synth-check`: PASS — Yosys 0.52, hierarchy check, process lowering, structural checks, generic synthesis, and final check; zero problems reported.
- Tool versions: Python 3.14.0, Cocotb 2.1.0, Verilator 5.032, Yosys 0.52. Tools were locally extracted from Ubuntu packages because administrator authentication blocked system package installation.
- Icarus 12.0: NOT COUNTED. A direct compile attempt emitted packed-struct width/frontend warnings; the Cocotb Icarus run was not completed. The required simulation gate passed with Verilator.
- Formal verification: NOT RUN; no formal correctness claim is made.

## Known limitations

No traps, interrupts, privilege modes, compressed instructions, misaligned split accesses, shared/variable-latency memory, caches, atomics, RV32M, floating point, branch prediction, out-of-order execution, formal proof infrastructure, RVFI, or ProofTrace. Unsupported instructions and out-of-contract misalignment are deterministically suppressed as specified; this is not full RISC-V exception compliance. Yosys generic cell statistics are not a physical area/timing result.

## Learning state

All teach-back questions remain `UNASSESSED`. Reading the learning pack is not evidence that the human has learned or can independently reproduce the design.
