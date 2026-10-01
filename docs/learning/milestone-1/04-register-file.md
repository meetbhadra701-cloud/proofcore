# 04 — The register file

## What is it?

The register file is the processor's small bank of fast working values. ProofCore has 32 registers, each 32 bits wide. An instruction can read two values and write one value.

## Why does ProofCore need it?

Most arithmetic instructions name their inputs and output by register number. The register file supplies operands in ID and receives committed results in WB.

## Mental model

Think of 32 labeled cubbies. Two readers can look at two cubbies at once. At the clock edge, one writer can update one cubby. Cubbie x0 is a painted zero: it cannot be changed.

## Actual implementation

See [rtl/regfile.sv](../../../rtl/regfile.sv). The array stores entries 1 through 31, so there is no writable storage location for x0. The two read paths return zero for address zero. If a WB write address matches a read address, the read path selects the new write data immediately. This is write-through behavior and ensures an instruction entering ID/EX on that edge does not capture an older value.

## Important code

The `always_ff` block clears x1-x31 on synchronous reset and otherwise writes only when `we` is true and `waddr != 0`. The combinational read block defaults to zero, looks up nonzero addresses, then applies the WB-match bypass.

## Concrete example

```asm
addi x5, x0, 7
add  x6, x5, x5
```

When the add is in ID, x5 may not yet have been written. The explicit forwarding network normally supplies the value in EX. If the add is far enough behind that `addi` is in WB in the same cycle, the register-file write-through also makes 7 visible to ID.

## What could go wrong?

Removing the x0 check lets software change a register that should always be zero. Removing write-through can make a same-cycle WB/ID match capture stale data after the value is no longer in the later forwarding stages.

## How our tests catch it

Tests read x0, attempt an x0 write, and use a back-to-back dependency chain. The full-core regression checks values at retirement, so a stale operand becomes a visible wrong result.

## Hardware interpretation

In this RTL the 31 writable words become 992 resettable flip-flops plus read-selection logic. An implementation technology may infer a register-file macro differently; no macro mapping is claimed here.

## Interview relevance

Explain why x0 is implemented as a constant, why reads are combinational, and what “write-first” or “write-through” means.

## Exercises

1. What should `x0` read after a write of `0xDEADBEEF` to x0?
2. How many bits of writable storage are there?

## Answers

<details><summary>Check your reasoning</summary>

It still reads zero. There are 31 writable registers times 32 bits, or 992 bits.

</details>
