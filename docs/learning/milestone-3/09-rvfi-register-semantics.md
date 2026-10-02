# 09 — RVFI register semantics

RVFI describes the architectural register state before and after one
instruction. `rvfi_rs1_addr`/`rs2_addr` identify sources the instruction uses;
`rvfi_rs1_rdata`/`rs2_rdata` contain the values those sources supplied;
`rvfi_rd_addr`/`rvfi_rd_wdata` describe the destination result. x0 always reads
zero and never records a write.

The tricky point is timing. `addi x5,x0,7` may still be in EX while the next
`add x6,x5,x5` is in ID. The register file can still hold an old value for x5.
When the ADD reaches EX, the forwarding mux selects the producer's 7 for both
operands. ProofCore stores those effective EX operands in its optional EX/MEM
and MEM/WB payloads, then reports them at retirement. It does not report a stale
ID read.

For an unused source, the decoder's `uses_rs1`/`uses_rs2` bit suppresses the
address. For x0, the address may be zero, and its reported data is forced to
zero. Writes to x0 are normalized to `rd_addr=0`, `rd_wdata=0`.

`make diff-rvfi-directed` and the 500-program RVFI differential run compare
source IDs and values as well as destination, PC, and memory fields. Upstream
`reg_ch0` then checks architectural register consistency across retired events.

**Failure example:** If RVFI captured only the register-file read at ID, the
consumer above could claim `rs1_rdata=0` while its ADD result is 14. The result
may be right, but the observation record would be internally inconsistent and
could not safely serve as an ISA proof boundary.

**Hardware view:** Source-use bits and forwarded data are ordinary metadata
flops enabled only in RVFI builds. The bypass mux itself already exists for the
processor.

**Interview question:** Why is RVFI source data not always equal to the raw
register-file output from the decode cycle?

**Exercise:** For `addi x5,x0,7; add x6,x5,x5`, write the expected RVFI source
addresses/data for the ADD retirement.
