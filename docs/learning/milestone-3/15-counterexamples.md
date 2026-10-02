# 15 — Reading counterexamples

A counterexample is a concrete sequence of symbolic choices that violates an
assertion. It is not merely a statement that a test failed. In SBY, a trace may
be stored as VCD and Yosys witness files alongside the solver log.

The M3 x0 negative control deliberately changes the read-port result for address
zero to one under `PROOFCORE_NEGATIVE_X0`. The assertion in `rtl/regfile.sv`
expects zero when a read address is zero. The formal run should produce a short
trace: reset, a post-reset read of x0, and a failed assertion. The trace is
preserved in the negative-control artifact directory and is used in
`19-real-formal-trace.md`.

When reading a VCD, first find the failing property name in the SBY log. Then
inspect the clock/reset, the assertion's trigger, and the relevant source
signals. Do not infer causality only from a waveform label; map values back to
the RTL condition. A trace may include arbitrary irrelevant input values.

An assertion failure may be an RTL bug, a property bug, a harness bug, or a
contract mismatch. M3's early invalid x0-array assertion and bad clock setup
were harness/property issues. The early upstream alignment failures came from
asking an upstream trap model to check behavior M1 does not implement. None was
an RTL defect.

**Interview question:** What should you verify before calling a counterexample
an RTL bug?

**Exercise:** Open the preserved x0 negative trace and identify the cycle where
the reset has ended and the assertion first sees the corrupted value.
