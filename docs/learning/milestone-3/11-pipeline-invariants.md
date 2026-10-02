# 11 — Pipeline invariants

An invariant is a rule that must hold whenever the modeled pipeline reaches a
state. The M3 properties are intentionally about relationships among validity,
control, and visible effects, rather than one test program's final answer.

Examples in `rtl/core.sv` assert that a retirement implies a valid MEM/WB
entry, a destination write implies a valid retirement and nonzero `rd`, a data
read has a valid load in EX/MEM, and a nonzero store mask comes from a valid
store. The register file asserts that reads addressed to x0 produce zero.

Forwarding assertions connect selector values to the source actually selected.
Additional priority assertions check the case where EX/MEM and MEM/WB both
match: the newer EX/MEM producer must win when its value is ready. Stall
assertions compare the current PC and IF/ID payload with the values saved before
the stall and require ID/EX to be invalid. Redirect assertions require the
target PC and cleared younger valid bits.

The local shadow counter checks RVFI order: it advances from its own state only
when a valid WB instruction retires, then compares the reported order against
that expected count. The upstream `unique` check provides a second order view.

These are bounded checks through 16 formal steps. They have not been proved by
induction, so a pass says no counterexample exists within that bound under the
documented assumptions.

**Failure example:** If `dmem_write_mask` escaped from a bubble, the assertion
requiring a valid EX/MEM store would fail without waiting for a full program to
produce a final-memory mismatch.

**Hardware view:** `FORMAL` assertions are verification-only statements. The
valid bits and control logic they inspect are real pipeline flops and gates.

**Interview question:** What evidence is needed before calling an assertion
non-vacuous?

**Exercise:** Pick an invariant above and identify the state transition that
could violate it.
