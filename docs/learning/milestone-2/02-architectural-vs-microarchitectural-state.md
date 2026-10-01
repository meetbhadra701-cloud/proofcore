# 02 — Architectural State vs. Microarchitectural State

## What is it?

Architectural state is what a program can observe as the machine's defined state: the PC, integer registers, and memory. Microarchitectural state is the internal machinery used to produce those results: pipeline registers, valid bits, forwarding choices, and stall control.

## Why does ProofCore need the distinction?

The reference model should be simpler than the RTL. If it copied IF/ID, ID/EX, forwarding, and flush behavior, it could repeat the same implementation defect instead of checking it.

## Mental model

The architectural state is the public scoreboard. Microarchitectural state is the worker's private notes and work queue. A wrong note matters only when it causes a wrong public result.

## Concrete snapshot

The reference might contain `PC=0x100`, `x5=7`, and `memory[0x200]=12`. At the same moment, RTL might also hold four partially processed instructions, one invalid bubble, a forwarding selector, and a load-use stall signal. The C++ model deliberately has none of those fields.

## Actual implementation

[`architectural_state.hpp`](../../../reference/include/architectural_state.hpp) stores 32 registers, PC, and retirement count. [`proofcore_pkg.sv`](../../../rtl/proofcore_pkg.sv) and the pipeline register modules hold RTL-only state. The two sides meet at `RetireEvent`, not at their internal registers.

## Concrete dependency example

```asm
addi x5, x0, 7
add  x6, x5, x5
```

The reference immediately updates `x5` during its first `step()`; the next step reads 7 twice. The RTL may have the first instruction in EX/MEM while the second is in EX, so it needs forwarding. A stale RTL read can produce the wrong `x6` even though the reference has no notion of forwarding.

## What could go wrong?

Comparing internal cycle state would be too strict: valid pipeline implementations can have different timing. Comparing only the final registers can be too weak: two wrong operations might later overwrite each other. Retirement is the useful middle boundary.

## How tests catch it

The driver advances the model exactly once for each valid RTL retirement. It does not advance on idle cycles or stalls. A bad flush often appears as an unexpected PC/instruction; a bad forwarding value appears as a wrong retirement destination value.

## Hardware and interview relevance

This distinction separates ISA-visible correctness from implementation choice. Exercise: classify `rd_value` at WB, an ID/EX valid bit, and a data-memory byte as architectural or microarchitectural, and explain why memory is architectural.
