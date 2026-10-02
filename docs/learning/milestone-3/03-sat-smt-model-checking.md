# 03 — SAT, SMT, and model checking

## What the solver receives

Yosys lowers registers and combinational logic into equations for the current
state, inputs, and next state. Assertions become “bad-state” conditions. SAT
search asks whether the equations plus a bad-state condition are satisfiable.
SMT adds richer theories such as bit-vectors and arithmetic; SMTBMC asks a
solver such as Z3 about one frame after another.

## ProofCore use

The `riscv-formal` instruction checks and the internal assertion run use ABC
`bmc3` bundled with Yosys for bounded search. Internal cover reachability and
expected-failure traces use Z3 with SymbiYosys. Exact tool revisions and
per-check engines are recorded in the result manifest.

The 32-bit values are finite bit-vectors. For example, `0xffffffff + 1` wraps
to zero. This is not unbounded integer arithmetic; it matches hardware width.
Signed comparison is an interpretation of the same 32 bits, not a different
storage format.

## What hardware this represents

An ALU addition becomes bit-vector addition, an equality test becomes a
comparator, and a mux becomes a conditional equation. These are models of the
RTL logic, not gates placed on a chip. Yosys `synth` separately checks a
synthesizable hardware netlist.

## What can go wrong

State-space size can grow sharply with memory, pipeline state, and proof depth.
A timeout is inconclusive. A bounded search can pass its requested frames and
still miss a longer failure. A SAT result on a negative control is useful only
if its waveform corresponds to an actual assertion and the assumptions allow
the scenario.

## Example

For a register update `q_next = we ? d : q`, the solver can choose `we=1` and
`d=1` to falsify an invariant that q must remain zero—unless the RTL gates the
write or the environment incorrectly assumes `we=0`.

**Interview question:** Why can bit-vector SMT model wraparound more accurately
than ordinary mathematical integer arithmetic?

**Exercise:** For 32-bit unsigned arithmetic, compute `0xffffffff + 1` and
explain which width causes the result.
