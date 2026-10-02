# Formal harness review: x0 observation and modeled clock

## Symptom

The first internal-property attempt reported an assertion failure and Yosys
reported undriven bits while lowering the register-file storage. Its raw ABC
witness was retained at
`artifacts/formal/m3-core-properties-first-attempt/`, but the pinned
`yosys-witness` converter crashed before it could emit a readable VCD.

## Root cause

The initial property treated x0 as a physical array element. ProofCore stores
only `regs[1:31]`; x0 is implemented by returning a constant zero at both read
ports. A reference to nonexistent `regs[0]` was therefore a bad property, not a
processor-state violation. The first formal-clock setup also did not clearly
drive a processor clock from a SBY timestep, leaving open the risk of a
vacuous sequential pass.

## Correction

The x0 property now checks the observable contract: whenever `raddr1` or
`raddr2` is zero, the corresponding read data is zero. The formal top uses
SBY's `(* gclk *)` formal timestep and toggles the actual DUT clock on each
formal step, while sequencing a synchronous reset edge.

## Evidence after correction

With the corrected clock, internal assertions pass under both ABC `bmc3` and
Z3 at depth 16. The final `make formal-cover` reaches all 13 cover goals; the
x0 negative control deliberately returns both read ports as one and produces a
real Z3 counterexample at step 3 for both x0 assertions. See
`artifacts/formal/m3-negative-x0/`.

## Classification

Harness/property defect. No RTL architectural defect was found. The early
assertion failure was not counted as processor evidence. This record remains
because it demonstrates why formal clocking and property-to-implementation
mapping must be reviewed before trusting a PASS or FAIL.
