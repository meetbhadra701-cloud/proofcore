# 15 — How synthesis sees ProofCore

## What is synthesis?

Synthesis translates a hardware description into a network of logic cells and storage cells. It does not execute the design as a software program.

## Why does ProofCore need it?

Simulation checks behavior. Synthesis checks that a synthesis frontend can elaborate the design and map the logic into a circuit representation. It can catch unsupported constructs and structural problems, but it does not prove the processor correct.

## Mental model

The RTL is a recipe for circuit structure. Synthesis recognizes patterns: edge-triggered assignment becomes flip-flops, a selection becomes a mux, and arithmetic becomes an adder or shifter network.

## Actual implementation

Run `make synth-check`. The target invokes Yosys to read SystemVerilog, check hierarchy, lower processes, run structural checks, synthesize `core`, check again, and print statistics. The last observed run used Yosys 0.52 and reported zero structural problems.

That run reported approximately 8,556 generic cells before any target technology mapping: 992 resettable bits for x1-x31, plus pipeline/control state and combinational logic. This generic count is not an area or timing estimate for a real chip.

## Important RTL-to-hardware examples

- `always_ff @(posedge clk)` becomes storage elements.
- `if (reset)` creates reset selection on the relevant state.
- Register address comparisons and operand selections become decode logic and muxes.
- `case (alu_op)` becomes a result mux over arithmetic/shift/logic choices.
- `a + b` becomes an adder network.
- `valid && reg_write && rd != 0` becomes write-enable control.

The Yosys frontend cannot accept package imports in module headers or package-typed struct ports in this installed version. The internal design still uses enums/structs, while module boundaries use fixed-width packed vectors. Verilator checks the same source before Yosys synthesis. This frontend accommodation is documented in `DECISIONS.md`.

## What could go wrong?

An incomplete combinational assignment can infer a latch. A simulation-only construct can stop synthesis. Passing Yosys does not show that signed comparisons, hazards, or flushes are behaviorally correct.

## How our tests catch it

Verilator lint checks the source for warnings; Cocotb checks behavior; Yosys checks synthesis structure. These are separate forms of evidence. None replaces the others.

## Interview relevance

Explain why a `case` statement is not a software branch in the final circuit and why synthesis success is not a proof of functional correctness.

## Exercises

1. What circuit does a 32-bit register declaration imply when updated in `always_ff`?
2. Does 8,556 generic cells tell us the final Sky130 area?

<details><summary>Answers</summary>

It implies 32 bits of storage, generally flip-flops unless a memory macro is inferred. No: technology mapping, cell library, and physical design are not included.

</details>
