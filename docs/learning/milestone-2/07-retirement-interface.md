# 07 — The Retirement Interface

## What is it?

Retirement is the point where an instruction's architectural effects are committed in program order. ProofCore exposes a record for that instruction: PC, bits, register effect, memory metadata, and next PC.

## Why does ProofCore need it?

The checker needs a stable architectural boundary. It should not inspect internal forwarding selectors or pipeline payloads to decide what the program did.

## Actual implementation

The RTL record type is `retire_t` in [`proofcore_pkg.sv`](../../../rtl/proofcore_pkg.sv); the top module exposes its fields as individual pins in [`core.sv`](../../../rtl/core.sv). The C++ equivalent is [`RetireEvent`](../../../reference/include/retire_event.hpp). The harness maps pins into that host-side record.

## Field meaning

`valid` says an instruction retires. `pc` and `instruction` identify it. `rd_write`, `rd`, and `rd_value` describe an architectural register update. `mem_addr`, read/write masks, and `mem_wdata` describe memory activity. `next_pc` is the architectural next instruction address.

## Comparison rules

The comparator checks PC, instruction, write enable, destination, next PC, and masks. It compares a written value only when there is a write. It compares memory address only when a read/write mask is nonzero, and write data only for stores. This avoids treating output bits declared zero/don't-care for non-effects as meaningful state.

## Concrete example

`sw x6,0(x10)` with `x10=0x1000` and `x6=14` retires with no register write, address `0x1000`, write mask `1111`, lane data `0x0000000e`, and sequential next PC. The checker compares each of these relevant fields.

## What could go wrong?

If the RTL asserts `retire_valid` twice, the second event is compared with the next reference PC and fails. If a bubble is incorrectly marked valid, its PC/instruction will disturb the ordered stream. If the comparator ignores `next_pc`, a bad branch target may go unnoticed until later.

## How tests catch it and exercise

Every directed/random program uses this record. Exercise: for `addi x0,x0,5`, identify which fields are meaningful and explain why the destination write must be false.
