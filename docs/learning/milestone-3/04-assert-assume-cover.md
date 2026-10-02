# 04 — Assert, assume, and cover

`assert(P)` says the design must make proposition P true. A failed assertion
means the solver found an allowed trace with P false. `assume(A)` narrows the
allowed environment to traces where A is true. `cover(C)` asks the solver to
find at least one allowed trace where C is true.

For ProofCore, an assertion checks that an EX/MEM bypass selector produces the
producer value. An assumption limits the wrapper to a 32-byte memory window.
A cover asks for a load-use stall or taken redirect so we know the corresponding
property has a reachable trigger.

These constructs are not interchangeable. Assuming a forwarding result equals
the producer would hide a forwarding bug. Asserting a memory address remains
inside the test window would be wrong if the environment, rather than the core,
owns that restriction. Covering an event shows reachability but does not prove
that every event is handled correctly.

The M3 source-level RVFI wrapper has seven custom assumption sites: two address
bounds, three control-flow alignment conditions, and two data alignment
conditions. The internal-property harness has its own two address bounds. The
upstream framework also assumes the selected instruction model is active while
checking that instruction. All are itemized in `formal/ASSUMPTIONS.md`.

## Failure example

If the harness assumes `load_use_stall == 0`, an assertion about correct stall
recovery may pass because the critical event was removed. The matching cover
would then remain unreachable and expose the problem.

## Hardware interpretation

Assertions, assumptions, and covers are verification logic. They do not create
processor gates in ordinary builds because they are included only for formal
analysis; optional RVFI outputs are also guarded by a compile-time define.

**Interview question:** Why must every important assumption be reviewed for
what behavior it excludes?

**Exercise:** Classify “the instruction memory remains stable” as an assertion,
assumption, or cover for this harness and justify it.
