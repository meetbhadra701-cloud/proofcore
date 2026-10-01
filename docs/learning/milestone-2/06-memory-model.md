# 06 — Two Independent Memory Models

## What is it?

The C++ `Memory` is a sparse map from 32-bit byte addresses to eight-bit values. The Verilator harness owns a second, separate `Memory` object as the RTL's external memory device.

## Why keep two copies?

If both implementations shared one memory object, one side could silently repair the other's missing or duplicated store. Separate memories evolve independently from identical initial bytes; disagreement stays visible.

## Actual implementation

[`memory.cpp`](../../../reference/src/memory.cpp) assembles words little-endian: byte at address `A` is bits 7:0. `RtlDriver::run()` constructs a reference memory and RTL memory independently using the same deterministic initializer and applies each case's overrides to both. The RTL adapter feeds a containing aligned word to `dmem_rdata` and applies `dmem_write_mask/data` on the active edge. The reference changes memory only while interpreting its own store instruction.

## Concrete example

Suppose bytes at `0x1000..0x1003` are `11 22 33 44`. An LW reads `0x44332211`. An SB of `0xaa` to `0x1002` uses mask `0100` and lane data `0x00aa0000`; afterward the word is `0x44aa2211`.

## Store synchronization

The RTL store pin fires in MEM, before that store's WB retirement. The model applies the architectural store when it steps the corresponding retirement. These updates happen at different microarchitectural times, but the separate memories preserve program order; loads compare their returned architectural values, store events compare address/mask/data, and final memory bytes are compared at program completion.

## What could go wrong?

Applying an RTL store to both memories duplicates the operation. Feeding stale dmem data to RTL can break a load even if the reference reads correctly. Using host-endian word casts can reverse byte lanes.

## How tests catch it

M1-derived cases initialize bytes at `0x80`, exercise LB/LBU/LH/LHU/LW and stores, then compare all final byte values. Random memory cases use the nonzero built-in image and store/load combinations.

## Hardware interpretation and exercise

The RTL side is an external memory adapter; this does not add a cache or memory controller. Exercise: write the byte mask and 32-bit lane data for `sh x5,2(x10)` when `x5=0x1234`.
