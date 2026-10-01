# 09 — Hazards and load-use stalls

## What is a hazard?

A hazard is a situation where the next instruction cannot safely do its work on the ordinary schedule. A data hazard happens when it needs a result that is not available yet.

## Why does ProofCore need an interlock?

Arithmetic results can be forwarded from EX/MEM. A load's address is known in EX, but its data arrives from memory in MEM. Its immediate consumer reaches EX before that data can be forwarded, so ProofCore waits one cycle.

## Mental model

The load is fetching a package from a warehouse. Knowing the shelf location is not the same as having the package. A following instruction that needs the package must wait until the memory response arrives.

## Actual implementation

`hazard_unit` in [rtl/hazard_unit.sv](../../../rtl/hazard_unit.sv) compares the EX load destination with source registers used by the valid IF/ID instruction. It ignores x0 and source fields that the decoder says are not used. `core.sv` holds PC and IF/ID and sends invalid input into ID/EX for one cycle.

## Concrete cycle example

```asm
lw   x5, 0(x1)
add  x6, x5, x2
```

| Cycle | IF | ID | EX | MEM | WB |
| ---: | --- | --- | --- | --- | --- |
| 1 | LW | — | — | — | — |
| 2 | ADD | LW | — | — | — |
| 3 | next | ADD | LW | — | — |
| 4 | held | ADD held | bubble | LW | — |
| 5 | target stream | next | ADD | — | LW |
| 6 | — | — | next | ADD | — |

During cycle 4, the load advances. The consumer is held in ID, and ID/EX becomes invalid. In cycle 5, the load value can be forwarded from MEM/WB as the consumer enters EX.

## What could go wrong?

If the consumer is not held, it uses old x5. If the load does not advance, the interlock can repeat forever. If an instruction's immediate bits are mistaken for rs2, false stalls appear.

## How our tests catch it

The cocotb scenario makes a loaded value feed both ALU operands and then rs1 of an immediate instruction. It also loads x0 and asserts no extra cycle for the false dependency.

## Hardware interpretation

Hazard detection is equality comparators plus source-use gates. A stall is control wiring into the PC and IF/ID hold inputs and the ID/EX valid input.

## Interview relevance

Explain why bypassing is sufficient for a nearby ADD but a load-use pair usually needs a bubble in this fixed-latency five-stage design.

## Exercises

1. Why should a load to x0 not stall a consumer that reads x0?
2. Which instruction advances during the bubble cycle?

## Answers

<details><summary>Check your reasoning</summary>

x0 remains zero, so the load cannot produce a new architectural value. The older load advances from EX to MEM while the consumer waits in ID.

</details>
