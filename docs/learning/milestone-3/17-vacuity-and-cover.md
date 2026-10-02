# 17 — Vacuity and cover reachability

A property can pass because its condition never happens. For example,
“whenever a load-use hazard occurs, the consumer is preserved” is vacuously
true if the environment never permits a load. A cover asks the solver to find a
trace in which that hazard occurs.

ProofCore covers EX/MEM forwarding, MEM/WB forwarding, a priority conflict,
load-use stall, taken and not-taken branch, JAL, JALR, loads, stores, retirement
after a stall, retirement after a redirect, and an x0 write attempt. The cover
campaign uses stable symbolic instruction words and reaches its goals within
the recorded depth 24. SBY writes a waveform for each found cover trace.

Cover success does not prove the assertion. It proves that one allowed path
reaches the event. Conversely, an uncovered goal can mean the path is impossible
or merely that it needs a deeper bound. Read `docs/formal/COVERAGE.md` alongside
`docs/formal/VACUITY_AUDIT.md` and the assumptions.

The upstream instruction checker selects one instruction model at a time. Its
`assume(spec_valid)` chooses the query; it does not prove that all instructions
occur in a single run. The generated campaign contains separate check records
for every supported operation.

**Failure example:** If an assumption disabled branches, a branch assertion
could pass while the taken-branch cover remained unreachable. That mismatch
would signal a vacuity problem.

**Interview question:** Does a successful cover prove correctness? What does it
prove instead?

**Exercise:** Pick one cover in `core.sv` and name the assumptions that could
prevent it from being reached.
