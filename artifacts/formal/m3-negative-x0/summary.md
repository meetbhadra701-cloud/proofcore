# x0 invariant negative-control trace

With `PROOFCORE_NEGATIVE_X0` enabled, reads whose address is x0 are deliberately
changed to 1. The solver reaches the bad output immediately after reset and
reports both x0 read-port assertions in `rtl/regfile.sv` (lines 26 and 27) as
failed at formal step 3. The VCD, SMT constraints, Yosys witness, and solver log
are preserved here. This is an expected self-test failure, not a production RTL
bug. The mutation is absent unless its explicit test-only define is passed.
