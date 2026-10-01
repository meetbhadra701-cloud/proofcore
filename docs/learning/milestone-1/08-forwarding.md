# 08 — Forwarding values around the register file

## What is it?

Forwarding is a bypass path that sends a result directly from a later pipeline stage to an EX operand mux. It avoids waiting for a result to travel to the register file and then back to the consumer.

## Why does ProofCore need it?

The register file may still hold an older value when a dependent instruction reaches EX. Without forwarding, `add x6,x5,x5` could use stale x5 immediately after an instruction produces x5.

## Mental model

Instead of putting a finished number on a shelf and asking someone to fetch it, hand the number directly from the producer to the waiting user.

## Actual implementation

The compare logic is in [rtl/forwarding.sv](../../../rtl/forwarding.sv). The chosen `FWD_EXMEM` or `FWD_MEMWB` selector controls the source muxes in [rtl/core.sv](../../../rtl/core.sv). EX/MEM is the newer producer, so it wins over MEM/WB. Each source operand is checked independently.

The producer must be valid, have register-write enabled, and target a nonzero register. An EX/MEM load is excluded because its data is not ready there.

## Concrete example

```asm
addi x1, x0, 1
addi x1, x1, 1
add  x2, x1, x1
```

When the `add` is in EX, the newest x1 value is 2 from the second `addi` in EX/MEM. The older x1 value 1 may also be in MEM/WB. Selecting MEM/WB would compute 2 instead of 4; EX/MEM priority makes x2 equal 4.

## What could go wrong?

If the priority is reversed, consecutive writes to the same destination return an old value. If x0 is not excluded, an instruction that names x0 can appear to depend on a producer that must never architecturally write.

## How our tests catch it

The forwarding program above is in the regression. It checks same-destination priority, both source operands, the result chain, and attempted writes to x0.

## Hardware interpretation

Register comparisons become equality comparators and enable gates. Each operand mux selects among register-file, EX/MEM, and MEM/WB values. The EX/MEM-first condition is the priority rule.

## Interview relevance

Draw the forwarding mux and write its conditions. Explain why a valid producer's destination register must match the consumer's source register.

## Exercises

1. If both EX/MEM and MEM/WB write x5, which value should feed a consumer of x5?
2. Should an EX/MEM load be forwarded as its ALU address?

## Answers

<details><summary>Check your reasoning</summary>

Use EX/MEM because it is newer. No: that stage contains the load's address, not its loaded data.

</details>
