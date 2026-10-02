# RVFI negative-control trace

The test-only `PROOFCORE_RVFI_NEGATIVE` define flips bit 0 of the reported
destination value for a valid nonzero register write. The trace reaches
`addi x16,x0,8` at PC `0x8`; the ISA model expects `rd_wdata=8`, while corrupted
RVFI reports 9. Upstream `rvfi_insn_check.sv:177` fails at step 12. This is the
expected outcome, so the failure-plumbing self-test is PASS. The VCD, Yosys
witness, and SBY log are preserved beside this summary. Normal builds do not
define the mutation macro.
