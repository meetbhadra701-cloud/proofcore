# 01 — What formal verification is

## What

Formal verification expresses a rule as logic and asks a mathematical solver
whether the rule holds for every state allowed by the model, or whether one
counterexample exists. It is a search over a symbolic state space, rather than
a list of hand-picked examples.

## Why ProofCore needs it

Simulation checks only the instruction streams and data values in the test
corpus. A formal engine can choose unusual register values, instruction words,
and dependency sequences that the program generator did not happen to select.

## Tiny example: x0

Suppose a register `x0_q` is reset to zero and has a write enable. The desired
rule is `x0_q == 0` after reset. The solver starts from the reset state, tries
every legal write enable/address/data combination, and searches for a later
state where `x0_q != 0`. In ProofCore, x0 is not a physical element in the
register array: it is returned as a constant zero. The property therefore
checks the observable read ports, and the negative control deliberately breaks
that output to show the assertion can fail.

```text
reset state → possible inputs → next state → check assertion
```

## Actual implementation

The formal-only read-port assertions are in `rtl/regfile.sv` inside the
`FORMAL` section. `formal/harness/formal_top.sv` provides a clock and reset.
`scripts/run_core_formal.py --mode prove` runs bounded BMC to depth 16.

## Hardware and solver view

The solver does not build new hardware. Yosys translates the RTL and properties
into Boolean transition equations. The engine searches those equations for an
assignment that violates an assertion.

## What can go wrong

An assumption can remove the bad state. A clock that never advances can make
every sequential property pass vacuously. A property can also be stated against
an internal signal that does not exist in the real design. M3's clock-harness
repair and assumption audit are documented in `docs/formal/bugs/` and
`docs/formal/assumption-audit.md`.

## Interview question

What is a counterexample, and how does it differ from a failing simulation
test?

## Exercise

Write in plain English a property that says a bubble can never request a data
store. Identify the signal that makes a bubble explicit.
