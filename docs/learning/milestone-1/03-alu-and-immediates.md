# 03 — The ALU and immediate generator

## What are they?

The ALU is the calculator in EX. It adds, subtracts, shifts, compares, and performs bitwise logic. An immediate generator gathers number bits scattered through an instruction and extends them to 32 bits.

## Why does ProofCore need them?

Arithmetic instructions use the ALU directly. Loads and stores use it to add a base register and an offset. Branches compare register values and use an immediate to find their target.

## Mental model

The ALU is a calculator with a selector knob. The immediate generator is a jigsaw: instruction formats split an immediate across several fields, so the circuit puts those pieces back in the correct order.

## Actual implementation

The logic is in [rtl/alu.sv](../../../rtl/alu.sv) and [rtl/imm_gen.sv](../../../rtl/imm_gen.sv). The immediate contract is:

| Kind | Reassembled immediate |
| --- | --- |
| I | sign-extend `insn[31:20]` |
| S | sign-extend `{insn[31:25], insn[11:7]}` |
| B | sign-extend `{insn[31], insn[7], insn[30:25], insn[11:8], 0}` |
| U | `{insn[31:12], 12'b0}` |
| J | sign-extend `{insn[31], insn[19:12], insn[20], insn[30:21], 0}` |

## Important code

The ALU uses `$signed(a) < $signed(b)` for SLT and `a < b` for SLTU. SRA uses `$signed(a) >>> b[4:0]`; SRL uses `a >> b[4:0]`. Both shifts use only the low five bits because a 32-bit register has 32 possible bit positions.

The I/S/B/J immediate assignments copy the instruction's sign bit into the upper result bits. The U immediate instead appends twelve zeros and does not sign-extend the 20-bit field.

## Concrete example

For `addi x3, x0, -1`, the 12-bit immediate is `111111111111`. Sign extension repeats its leftmost one 20 more times, producing `0xFFFFFFFF`. Adding it to x0 yields the two's-complement value -1.

For `beq x1, x2, -4`, the B-immediate bits are reassembled into a signed byte offset `0xFFFFFFFC`. EX adds that offset to the branch's own PC, returning to the instruction four bytes before it.

## What could go wrong?

If the B immediate swaps `insn[7]` and `insn[11]`, forward branches may still pass while backward branches jump to the wrong place. If SRA is implemented as SRL, negative inputs shift in zeros and become positive.

## How our tests catch it

The regression checks all ten ALU operations, negative I/S/B offsets, SLT versus SLTU disagreement, SRA sign fill, and shifts by zero and 31.

## Hardware interpretation

The ALU's operation choice becomes a mux over adder, shifter, comparator, and bitwise results. The immediate generator is mostly wiring, concatenation, and sign-extension wires; it does not need a general-purpose arithmetic unit.

## Interview relevance

Be prepared to explain why `SLTIU` sign-extends its immediate before doing an unsigned compare, and why the B/J immediates have a zero low bit.

## Exercises

1. Sign-extend the 12-bit value `0x800` to 32 bits.
2. What is `0x80000000 >>> 1` as a signed arithmetic shift?

## Answers

<details><summary>Check your reasoning</summary>

`0x800` has sign bit 1, so the 32-bit result is `0xFFFFF800`. Arithmetic shift gives `0xC0000000` because the leftmost one is copied into the new high bit.

</details>
