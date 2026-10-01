# 05 — Register and PC Semantics in the Model

## What is it?

The register array and PC are the model's core architectural state. Each call to `step()` reads the old source values, calculates one instruction's result, then records the resulting next PC and destination write.

## Why does ProofCore need it?

Forwarding, register-file timing, and PC redirect bugs all eventually change a retirement result. The reference can ignore how RTL arrives there while still checking the exact architectural value and next address.

## Actual implementation

`ArchitecturalState` contains `std::array<uint32_t,32> registers`, `pc`, and `retirement_count`. x0 is reset to zero after each step and writes to destination zero are not exposed as register writes. Sequential PC uses unsigned `pc + 4`; branch/jump targets use the instruction's own PC and decoded displacement.

## Concrete example

At PC `0x20`, `jal x3,8` retires with `rd=x3`, `rd_value=0x24`, and `next_pc=0x28`. The result written to x3 is PC+4, while the new control-flow PC is PC+8. These are different values with distinct meanings.

## Signedness and wraparound

RV32I registers contain bit patterns. Addition wraps in 32 bits. Signed `SLT` compares their two's-complement interpretations, while `SLTU` compares the unsigned values. `std::bit_cast<int32_t>` makes the intended signed interpretation explicit.

## What could go wrong?

Using `pc+4` as a JAL target would confuse the link value with the transfer target. Letting x0 accept a stored result would make later source reads incorrect. Using C++ signed overflow would make behavior undefined instead of wrapping.

## How tests catch it

The unit tests check x0 writes, signed/unsigned disagreement, AUIPC's own PC, JAL/JALR link values, and next PC. Differential runs compare `next_pc` on every retirement.

## Hardware interpretation and exercise

The model's state array corresponds loosely to architectural flops, but the RTL register file is a separate hardware structure with synchronous writes and bypasses. Exercise: for an instruction at PC 100, write the link value and target of `jal x1,20` in hexadecimal and explain the difference.
