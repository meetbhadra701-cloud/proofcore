# 19 — Walkthrough of a preserved formal counterexample

This is the counterexample from the deliberate RVFI mutation, not a defect in
the normal RTL. The explicit `PROOFCORE_RVFI_NEGATIVE` define flips bit 0 of
every valid nonzero destination value. The upstream `insn_addi_ch0` check is
expected to catch the changed value. The VCD, witness, and solver log are in
`artifacts/formal/m3-negative-rvfi-addi/`.

## Initial state and symbolic program

Reset initializes the integer registers to zero. Instruction memory is
symbolic, but the solver chooses a concrete witness. The relevant retired
instructions are:

| Formal frame | PC | Instruction | Retired effect |
| ---: | ---: | --- | --- |
| 5 | `0x0` | `beq x0,x16,+12` (`0x01000663`) | x0 equals x16=0, so next PC is `0xC`; order 0 |
| 8 | `0xC` | `beq x4,x8,-8` (`0xFE820CE3`) | both sources are zero, so next PC is `0x4`; order 1 |
| 11 | `0x4` | unsupported encoding `0xFFC02004` | M1 inert sequential retirement; next PC `0x8`; order 2 |
| 12 | `0x8` | `addi x16,x0,8` (`0x00800813`) | expected destination value 8; order 3 |

This symbolic loop keeps execution inside the 32-byte memory window. The
upstream check is aimed specifically at the final ADDI event; it does not assume
the earlier unrelated instructions are correct.

## Property trigger and failure

At frame 12, `rvfi_valid=1`, `rvfi_pc_rdata=0x8`, and `rvfi_rd_addr=16`. The
instruction model computes `0 + 8 = 8`, so `spec_rd_wdata=0x00000008`. The
test-only mutation changes the public RVFI result to
`rvfi_rd_wdata=0x00000009`. The upstream destination-result assertion at
`rvfi_insn_check.sv:177` fails. This shows the checker is connected to the RVFI
result and can detect a known corruption.

The normal build does not define the mutation. The negative mode is recorded as
self-test PASS because the formal result was the expected FAIL. It must not be
reported as an RTL bug.

## Separate x0-property trace

The second preserved trace, `artifacts/formal/m3-negative-x0/trace.vcd`, enables
`PROOFCORE_NEGATIVE_X0`. At formal step 3 the register-file read addresses are
both zero, while the mutated outputs are both one. Assertions at
`rtl/regfile.sv:26-27` fail. This isolates the internal-property layer from the
upstream ISA checker.

## What this teaches

Start with the named assertion in the solver log, then find the event that
activated its checker, compare the expected expression to the observed RVFI
field, and follow the source path back to the RTL. A useful counterexample has
enough context to explain the failure, but it does not automatically identify
the original cause. Here the cause is known because the negative-control define
deliberately changes the signal.
