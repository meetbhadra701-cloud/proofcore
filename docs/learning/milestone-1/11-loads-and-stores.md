# 11 — Loads and stores

## What are they?

Loads copy bytes from data memory into a register. Stores copy register bytes into memory. The instruction gives an address and a width: one byte, two bytes, or four bytes.

## Why does ProofCore need them?

Programs need memory for arrays, stack data, and values too large for registers. The core must turn a byte address into a word lane and must extend small loaded values to 32 bits correctly.

## Mental model

Picture a 32-bit box with four numbered byte spaces. Address bits `[1:0]` point to one space. A byte store opens one lane; a halfword store opens two adjacent lanes; a word store opens all four.

## Actual implementation

Address calculation happens in EX. MEM presents the aligned containing-word address on `dmem_addr`; the testbench returns that 32-bit word. `core.sv` selects a byte or halfword, extends it, and generates write masks/data. The interface contract is in [SPEC.md](../../../SPEC.md).

Byte data is masked to eight source bits before lane shifting. Halfword data is masked to sixteen bits. This keeps retirement data clean and makes the lane placement easy to inspect. Misaligned halfword/word operations are inert no-ops by project decision; no trap or split transaction exists.

## Concrete example

If memory word `0x80007F80` is at byte address 128, its bytes at increasing addresses are `80 7F 00 80`. `LB` from address 128 returns `0xFFFFFF80`, while `LBU` returns `0x00000080`. `LH` at address 130 returns `0xFFFF8000`; `LHU` returns `0x00008000`.

For `sb x7, 1(x1)` with x7=`0xFFFFFFAA`, the write mask is `0010` and lane-positioned write data is `0x0000AA00`. The upper source bits are not written.

## What could go wrong?

Using the effective address instead of the aligned word address to index a word memory selects the wrong word. Forgetting sign extension makes negative byte values positive. A mask shifted by one lane in the wrong direction corrupts an adjacent byte.

## How our tests catch it

The test initializes an asymmetric word, checks signed and unsigned byte/halfword loads, checks all masks and lane-positioned write data, and inspects the resulting bytes.

## Hardware interpretation

Address addition is a 32-bit adder. Lane selection is a mux. Sign extension is mostly repeated wires from the selected sign bit. Masks are small decode logic. Stores do not need to shift the entire memory; they enable selected byte lanes.

## Interview relevance

Explain why the memory address is byte-addressed, what `addr[1:0]` controls, and how LB differs from LBU.

## Exercises

1. What mask does `SH` at a naturally aligned address ending in binary `10` use?
2. What does `LB` return for byte `0x80`?

## Answers

<details><summary>Check your reasoning</summary>

The mask is `1100`, selecting lanes two and three. LB sign-extends to `0xFFFFFF80`.

</details>
