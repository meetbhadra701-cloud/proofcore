# 16 — Bounded proofs versus induction

Bounded model checking explores all modeled traces up to a finite depth. M3's
instruction checks run to their configured depth (12), and the internal
property campaign checks depth 16. A BMC PASS means no violation was found in
those frames under the assumptions. A failure at depth n gives a concrete
counterexample of at most that length.

An inductive proof attempts a stronger argument. It proves a base case and then
shows that if a property holds in one arbitrary reachable state, it holds in
the next. Successful induction can establish a property for arbitrarily many
steps, subject to the model and assumptions. An induction attempt may fail
because the property needs strengthening even when the RTL is correct.

Milestone 3 does not claim an inductive processor result. All recorded core
assertions are bounded BMC. Covers are bounded reachability. The 64-bit order
counter's local property is still checked only to depth 16; do not upgrade its
claim merely because it is a simple invariant.

**Example:** If a write to x0 never changes its observable zero value, BMC can
check many write/read sequences but only to its depth. Induction could prove
the invariant forever if the base and transition relation establish it.

**Failure example:** A bug requiring 20 cycles may not appear in a depth-16
pass. Increasing depth can help, but complexity may grow. It does not itself
turn BMC into induction.

**Interview question:** What exactly does a depth-16 BMC PASS mean?

**Exercise:** Find one property in `core.sv` that might be easy to prove
inductively and explain what the induction step would need to show.
