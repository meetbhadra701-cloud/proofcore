# 01 — RISC-V basics used by ProofCore

## What is it?

RISC-V is a set of rules for how software instructions behave. ProofCore implements a 32-bit integer subset called RV32I. “32-bit” means the main registers and arithmetic values contain 32 bits. Each supported instruction is 32 bits long.

## Why does ProofCore need it?

The instruction-set rules tell us what `add`, `lw`, or `beq` must do even when the internal circuit changes. Tests can compare retired results with those architectural rules rather than with one particular wiring choice.

## Mental model

An instruction is a small packed form. Some bits say which job to do; other bits name registers or hold a number. The decoder is the reader that turns those bits into control signals.

The register names are `x0` through `x31`. Each holds 32 bits. `x0` is special: reads always return zero and writes disappear. The program counter (PC) is a byte address. Since instructions are four bytes, ordinary execution adds four to the PC.

## Actual implementation

The supported instructions and immediate bit placement are frozen in [SPEC.md](../../../SPEC.md). Opcode and function-field decisions are tabulated in [docs/architecture/decode-table.md](../../architecture/decode-table.md). The testbench's format encoders are in [tb/cocotb/test_core.py](../../../tb/cocotb/test_core.py).

Examples of formats:

```text
R type: funct7 | rs2 | rs1 | funct3 | rd | opcode
I type: immediate[11:0] | rs1 | funct3 | rd | opcode
S type: immediate[11:5] | rs2 | rs1 | funct3 | immediate[4:0] | opcode
```

## Concrete example

`addi x5, x0, 7` means: take the value from x0 (zero), add the signed number 7, and write x5. The instruction names x5 in its destination field, x0 in its source field, and contains 7 in its immediate field.

For `lw x5, 4(x1)`, the processor adds 4 to the value in x1. That sum is a **byte address**. The memory system then selects a 32-bit word containing those bytes.

## What could go wrong?

Treating a byte address as a word number moves memory accesses four times too far. Treating x0 as an ordinary storage cell allows a write to break every instruction that relies on zero. Reading an immediate from the wrong instruction bits turns a valid opcode into a wrong calculation.

## How our tests catch it

The regression checks x0 reads and writes, negative I and S immediates, each ALU family, all branch conditions, and byte addresses on different lanes.

## Hardware interpretation

Register names are 5-bit wires. Instruction fields are slices of a 32-bit word. The PC is a 32-bit register and normally feeds a 32-bit adder that adds four.

## Interview relevance

Explain the difference between an instruction-set architecture (what software sees) and a microarchitecture (how the circuit carries it out). Know why x0 is constant and why PC-relative instructions carry their own PC down the pipeline.

## Exercises

1. For `add x6, x5, x5`, identify `rs1`, `rs2`, and `rd`.
2. If the PC is `0x100`, what is the next sequential PC?

## Answers

<details><summary>Check your reasoning</summary>

Both source registers are x5; the destination is x6. The next sequential PC is `0x104`.

</details>
