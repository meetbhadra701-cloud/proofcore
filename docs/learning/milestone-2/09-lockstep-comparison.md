# 09 — Retirement Lockstep Comparison

## What is it?

Lockstep here means the next reference instruction is executed when the RTL emits its next valid retirement. It does not mean both machines advance one instruction per clock.

## Why does ProofCore need it?

Stalls and flushes mean clock cycles are not architectural instruction steps. Advancing the reference on every cycle would compare unrelated moments.

## Actual implementation

`RtlDriver::run()` checks `retire_valid`. If it is low, the model does not step. If high, it snapshots reference state, calls exactly one `step()`, constructs the RTL event from pins, optionally applies explicit self-test corruption, and calls `compare_events()`.

## Comparator semantics

`compare_events()` reports a list of field names. PC, instruction, destination write, and next PC are always checked. Destination value matters only when a write occurred. Masks are always checked; addresses and store data matter only for memory operations. End-of-program memory contents are compared separately.

## Concrete example

On a load-use hazard, RTL may spend an extra cycle holding IF/ID and injecting a bubble. The reference remains unchanged during that clock. When the dependent instruction later retires, both sides consume one architectural step and compare its result.

## What could go wrong?

If a bubble advances the model, all later PCs shift and produce misleading failures. If the comparator checks irrelevant `rd_value` bits on stores, harmless interface conventions can fail the test. If it omits store masks, a wrong byte lane could pass when a later load does not inspect it.

## How tests catch it and exercise

The directed set includes stalled loads, redirects, and wrong-path suppression. The self-test changes the sampled x6 result from 14 to 7 and confirms the comparator reports `rd_value`. Exercise: explain why comparing final registers alone is less diagnostic than the first differing retirement.
