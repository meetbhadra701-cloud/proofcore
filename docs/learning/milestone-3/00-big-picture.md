# 00 — Milestone 3: the big picture

## What M3 adds

Milestone 1 built the processor. Milestone 2 compared its retirements with a
separately written software model. M3 adds a third kind of evidence: a solver
explores many possible processor states and checks whether a precise property
can ever become false within a configured bound.

There are two independent formal views. The upstream `riscv-formal` checks see
ProofCore through RVFI and compare each supported instruction with a compact
ISA model. ProofCore's own assertions look inside the pipeline to check x0,
valid retirement, forwarding selection, load-use stalls, redirects, and memory
side effects.

## Why ProofCore needs both

An ISA check can report that an ADD wrote the wrong value, but it does not
explain which internal bypass mux supplied that value. An internal property can
prove a particular forwarding selection condition, but it may not know whether
the final result matches the ISA. Using both gives an architectural check and a
microarchitectural check with different failure views.

## Mental model

Imagine asking: “Can you find any allowed starting state and input sequence
that makes this rule false?” The solver tries to construct one. A counterexample
is such a sequence. A bounded pass means it found no counterexample through the
stated number of steps. A cover asks the opposite existence question: “Can this
interesting situation happen?”

## Actual implementation

`rtl/core.sv` and `rtl/regfile.sv` contain properties under `FORMAL`.
`rtl/core.sv` adds optional RVFI ports under `PROOFCORE_RVFI`.
`formal/harness/` supplies clocks, reset, and finite symbolic memories.
`formal/rvfi/` configures the pinned checker; `scripts/` runs it and records
per-check results in `formal/results/results.json`.

## What the results do not prove

M3 checks the 37 implemented integer instructions in a 32-byte aligned memory
window with the assumptions in `formal/ASSUMPTIONS.md`. BMC is bounded, and
covers demonstrate reachability only. There is no full privilege/trap model,
inductive processor proof, physical implementation proof, or claim that the
human has learned the design.

## Failure example

If an ADDI retires `x5=8` when the ISA says `x5=7`, the instruction check can
produce a trace showing the instruction, operands, and bad destination value.
If the error came from stale forwarding, the internal property focuses on the
producer match and selected bypass source.

## Interview question

Why combine an ISA-level check with internal pipeline properties?

## Exercise

Pick one M2 differential test and name one invariant inside the pipeline that
could catch the same bug earlier than retirement comparison.
