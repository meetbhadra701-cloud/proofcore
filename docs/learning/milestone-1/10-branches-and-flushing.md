# 10 — Branches, jumps, and flushing

## What are they?

A branch or jump changes which instruction address is fetched next. A flush invalidates younger instructions that were fetched along the path that is no longer selected.

## Why does ProofCore need flushing?

ProofCore fetches sequentially before a branch decision is known. When EX finds a taken branch, the current IF and ID instructions may be wrong-path instructions. They must not reach a register or memory write.

## Mental model

The pipeline is walking down a hallway before learning which door is correct. At the branch desk, it learns the chosen door and discards the two younger hallway tickets.

## Actual implementation

Branches, JAL, and JALR resolve in EX in [rtl/core.sv](../../../rtl/core.sv). Branch comparisons are signed or unsigned as directed by the decoder. JAL target is PC plus J-immediate; JALR target is rs1 plus I-immediate with bit zero cleared. Redirect invalidates IF/ID and ID/EX. Older EX/MEM and MEM/WB instructions continue.

## Concrete example

```asm
beq x1, x2, target
sw  x3, 0(x0)      # wrong path when branch is taken
addi x4, x0, 99    # wrong path when branch is taken
target: addi x5, x0, 7
```

When BEQ reaches EX and is true, the store and addi are discarded. Only the target instruction later retires. If the BEQ is false, the store and addi remain valid and execute in order.

## What could go wrong?

Flushing only instruction bits but leaving valid high can still permit a store. Flushing an older EX/MEM instruction can lose a correct update. A signed compare used for BLTU can reverse outcomes for values with the high bit set.

## How our tests catch it

Tests check all six branch predicates with taken and not-taken outcomes, JAL/JALR links, load-to-branch forwarding, wrong-path store memory contents, retirement PCs, and architectural next PC.

## Hardware interpretation

Branch comparisons are equality or magnitude comparators. Target generation is an adder and alignment check. Redirect is control logic into the PC; flush clears valid bits in younger stage registers.

## Interview relevance

Explain why branch resolution stage sets the flush penalty and why wrong-path side effects are a correctness failure even if the final PC is right.

## Exercises

1. Which stages contain younger work when a branch resolves in EX?
2. Should a store already older than the branch and in MEM be flushed?

## Answers

<details><summary>Check your reasoning</summary>

The younger stages are IF/ID and ID. An older store in MEM must complete; it belongs before the branch in program order.

</details>
