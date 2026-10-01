# 00 — The whole processor, from the beginning

## What is it?

A processor is a circuit that repeatedly reads an instruction, figures out what that instruction means, works on numbers, possibly reads or writes memory, and records the result. ProofCore does those jobs in five steps called stages: **IF** fetches an instruction, **ID** decodes it and reads registers, **EX** calculates, **MEM** accesses data memory, and **WB** writes a result back.

The stages are like five people on an assembly line. While one instruction is being written back, a newer one can be doing arithmetic. Each instruction still moves in order.

## Why does ProofCore need it?

A five-stage pipeline is large enough to show real hardware problems. One instruction may need a number that a nearby older instruction has not written yet. A branch may make the next fetched instruction wrong. Loads take longer to produce their number than ordinary arithmetic. These are the reasons forwarding, stalls, bubbles, and flushing exist.

## Mental model

Imagine five desks in a row. Each desk works on one paper, and at each clock beat every paper moves to the next desk. A paper can be blank. ProofCore labels a blank desk with `valid=0`; it never guesses that a particular instruction bit pattern means “blank.”

```text
instruction memory -> IF -> IF/ID -> ID -> ID/EX -> EX -> EX/MEM -> MEM -> MEM/WB -> WB
                                            |                              |
                                            +-- branch redirect ----------> PC
                                                                           +-> registers
                                                                           +-> retirement record
```

## Actual implementation

The top module is [rtl/core.sv](../../../rtl/core.sv). Reusable operation names and packed data types are in [rtl/proofcore_pkg.sv](../../../rtl/proofcore_pkg.sv). The human-readable contract is [SPEC.md](../../../SPEC.md) at the repository root; the five-stage picture is [docs/architecture/milestone-1.md](../../architecture/milestone-1.md).

The core talks to two simple memories. Instruction memory supplies one 32-bit instruction for the current byte address. Data memory supplies a containing 32-bit word and accepts byte-write masks. Keeping those ports separate means a fetch and a load can happen during the same cycle.

## Important code idea

Every pipeline register has a separate valid bit. For example, `proofcore_if_id` captures a new PC and instruction only when it is not held or flushed. Its `flush` condition clears validity. The instruction bits are just information; validity decides whether they may cause effects.

## Concrete example

```asm
addi x5, x0, 7
add  x6, x5, x5
sub  x7, x6, x5
```

The first instruction creates the number 7. The second needs that new 7 twice, and the third needs both the second result and the first result. The results move down the pipeline; bypass paths send a newer result directly to EX so the dependent instruction does not wait for it to be written and read again.

## What could go wrong?

If a taken branch changes the PC but leaves a younger instruction valid, that wrong-path instruction may write a register or memory. If a bubble is represented only by zero instruction bits, a future decode change can accidentally make the “empty” slot perform work. Validity makes both rules explicit.

## How our tests catch it

The cocotb regression runs nine directed program scenarios. It checks register results, retirement PCs, memory bytes, branch next PCs, and that wrong-path instructions never retire or modify memory. Run `make test`.

## Hardware interpretation

The five stage boundaries become groups of flip-flops. Arithmetic becomes adders, shifters, and comparators. Operand selection and writeback become multiplexers. Decode becomes control logic. The valid bits are ordinary flip-flops that prevent an empty stage from enabling writes.

## Interview relevance

Be ready to draw the five stages, explain why a pipeline is faster for a stream of independent instructions, and describe what happens when a branch is taken.

## Exercises

1. Draw the five instruction positions for three independent instructions over seven cycles.
2. Circle the point where ProofCore changes architectural register state.

## Answers

<details><summary>Check your reasoning</summary>

The first instruction fetches in cycle 1 and reaches WB in cycle 5. Each later instruction starts one cycle later. WB is the architectural update point.

</details>
