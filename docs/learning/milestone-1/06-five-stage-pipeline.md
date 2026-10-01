# 06 — The five stages

## What are they?

The stages divide instruction work into steps that can overlap: IF fetches; ID decodes/reads; EX calculates/compares; MEM reads or writes data; WB updates registers and reports retirement.

## Why does ProofCore need them?

The boundaries make timing visible. A value's location tells us whether a dependent instruction can read it normally, forward it, or must wait. The same stages give branches a clear decision point and retirement a clear endpoint.

## Mental model

Each clock beat moves work one desk to the right. At the same time, the PC starts another instruction. An instruction's PC, bits, and controls travel with its data so a later stage knows which instruction it is serving.

## Actual implementation

The integration is in [rtl/core.sv](../../../rtl/core.sv). `ifid_t`, `idex_t`, `exmem_t`, and `memwb_t` are defined in [rtl/proofcore_pkg.sv](../../../rtl/proofcore_pkg.sv). The stages are not separate processors; they are combinational work separated by flip-flops.

## Important code

The EX logic chooses forwarded operands, computes an ALU result, determines branch conditions, forms the target, and builds the EX/MEM payload. MEM uses EX/MEM's address/control to drive the data port. WB selects the final value and creates the retirement record.

## Concrete cycle example

For three independent instructions:

```text
Cycle | IF | ID | EX | MEM | WB
------+----+----+----+-----+---
1     | I1 |    |    |     |
2     | I2 | I1 |    |     |
3     | I3 | I2 | I1 |     |
4     | I4 | I3 | I2 | I1  |
5     | I5 | I4 | I3 | I2  | I1
```

Once full, different instructions occupy all five stages. A taken branch or a load-use dependency inserts empty positions and interrupts that ideal flow.

## What could go wrong?

If a PC is not carried with the instruction, a branch target or retirement PC may accidentally use the current fetch PC, which belongs to a younger instruction. If memory controls are detached from the instruction, a store can use the wrong address or data.

## How our tests catch it

Retirement checks instruction PC and next PC across dependencies, stalls, redirects, and loops. Expected retirement order catches instructions that are lost or duplicated.

## Hardware interpretation

Each stage boundary is a bank of flip-flops. Within a stage, comparators, adders, and muxes settle during the cycle before the next edge.

## Interview relevance

Be able to explain which work occurs in EX versus MEM and why state advances at clock edges.

## Exercises

1. In which stage does ProofCore decide if a branch is taken?
2. In which stage does a load return its final register value to the pipeline?

## Answers

<details><summary>Check your reasoning</summary>

Branches resolve in EX. Data memory supplies a word in MEM; the selected/extended value is carried into MEM/WB and written in WB.

</details>
