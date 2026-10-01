# 04 — Independent Instruction Decoding

## What is it?

Decoding turns instruction bit fields into an operation such as ADD, LW, or BLTU. The C++ model performs that decoding itself.

## Why does independence matter?

If the test oracle imports the RTL decoder's control table, one wrong funct interpretation could be copied into both sides. The checker would then agree on the same mistake.

## Actual implementation

[`rv32i_decode.cpp`](../../../reference/src/rv32i_decode.cpp) extracts opcode, `funct3`, and `funct7`, then uses its own C++ switches. It does not include the SystemVerilog package or use RTL-generated control values. The decode table in [`docs/architecture/decode-table.md`](../../architecture/decode-table.md) is a human reference, not an imported implementation artifact.

## Shared assumptions

Both sides necessarily rely on the public RV32I instruction bit positions and opcode encodings. Those are ISA facts, not ProofCore-specific control enums. This common assumption is documented in `DECISIONS.md`; it remains a possible common-mode source that later cross-checks can reduce.

## Concrete example

For `add x6,x5,x5`, opcode `0110011`, `funct3=000`, and `funct7=0000000` identify register ADD. Changing `funct7` to `0100000` identifies SUB. An unsupported combination decodes to `Illegal`, which the frozen M1 subset retires as an inert sequential no-op.

## What could go wrong?

An off-by-one field slice could mistake an immediate bit for a register index. A permissive decoder could accept a reserved shift encoding. The model explicitly checks shift upper bits for SLLI/SRLI/SRAI.

## How tests catch it

The independent unit suite exercises every implemented family; the M1-derived differential corpus includes unsupported words and the randomized generator emits only supported encodings.

## Interview relevance and exercise

Be prepared to explain why sharing architectural constants is unavoidable while sharing a generated implementation table is risky. Exercise: decode the opcode and `funct3` of `lbu x4,1(x10)` by hand and name its source and destination registers.
