# 03 — The C++ RV32I Model

## What is it?

The C++ model is an interpreter: it reads one 32-bit instruction, applies its architectural meaning, and produces a retirement event. It is software, not a second processor pipeline.

## Why does ProofCore need it?

It supplies expected values for differential tests without relying on the Cocotb test author to precompute every result in every generated program.

## Actual implementation

Read [`rv32i_model.hpp`](../../../reference/include/rv32i_model.hpp), [`rv32i_model.cpp`](../../../reference/src/rv32i_model.cpp), and [`memory.hpp`](../../../reference/include/memory.hpp). `Rv32iModel` owns the instruction bytes, a `Memory`, and an `ArchitecturalState`.

## Important operation blocks

`step()` snapshots `pc`, fetches instruction bytes in little-endian order, and calls the independent decoder. It reads the source registers, begins with sequential `next_pc=pc+4`, then switches over the decoded operation. Addition and subtraction use `uint32_t`; overflow therefore wraps modulo `2^32`, as RV32I requires. Signed comparisons reinterpret the same 32 bits as `int32_t`; unsigned comparisons use `uint32_t` ordering.

Loads compute an effective byte address, enforce the M1 alignment rule, read the containing little-endian word, select the requested bytes, and extend signed or unsigned data. Stores make a lane mask and lane-positioned payload, update only the selected bytes, and record the same architectural effect. At the end, the model writes nonzero `rd`, updates PC, increments retirement count, and restores x0 to zero.

## Concrete example

If `x1=0x80000000`, then `sra x2,x1,x3` with `x3=1` gives `0xc0000000`, while `srl` gives `0x40000000`. The model uses a signed view only for SRA and signed comparisons.

## What could go wrong?

Using signed C++ addition can invoke undefined behavior on overflow. Keeping arithmetic unsigned avoids that. Forgetting to mask shifts to five bits can make C++ shifts undefined for amounts >=32; the model always masks with 31.

## How tests catch it

[`reference_tests.cpp`](../../../reference/tests/reference_tests.cpp) compiles with `-Wall -Wextra -Wpedantic -Werror` and checks arithmetic boundaries, immediate shifts, memory widths, control flow, x0, and subset behavior. `make reference-test` runs it separately from RTL.

## Hardware interpretation

The C++ `switch` is not hardware decode. It is an executable specification. The RTL counterpart uses comparators, ALU logic, muxes, pipeline flops, and memory pins.

## Exercise

Add a model test for `0xffffffff + 1`. Predict the result before running it, then explain why unsigned C++ arithmetic is appropriate here.
