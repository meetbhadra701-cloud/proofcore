# 12 — The ProofCore Milestone-1 retirement interface

## What is it?

Retirement is the point where an instruction's architectural work becomes official. The interface reports the instruction and the effect it commits.

## Why does ProofCore need it?

Tests need a stable way to observe what software should see without depending on internal muxes or register encodings. Later verification can adapt this record to a formal interface, but this record is not RVFI.

## Mental model

Think of a receipt printed at checkout. It names the instruction, records any register or memory update, and gives the next architectural PC. A bubble does not get a receipt.

## Actual implementation

The `retire_t` packed type is in [rtl/proofcore_pkg.sv](../../../rtl/proofcore_pkg.sv). In [rtl/core.sv](../../../rtl/core.sv), MEM/WB supplies its fields and the top-level module flattens the record into individual outputs. A valid unsupported instruction still retires, but its effect fields are zero and `next_pc=pc+4`.

The fields are `valid`, `pc`, `insn`, `rd`, `rd_write`, `rd_value`, `mem_addr`, `mem_read_mask`, `mem_write_mask`, `mem_write_data`, and `next_pc`.

## Concrete example

For `addi x5,x0,7` at PC `0x20`, the record says valid=1, pc=`0x20`, instruction bits for that ADDI, rd=5, rd_write=1, rd_value=7, memory masks=0, and next_pc=`0x24`.

For a taken BEQ at PC `0x20` with target `0x30`, rd_write=0, memory fields are zero, and next_pc=`0x30`.

## What could go wrong?

If retirement is taken from EX, an instruction may be reported before older memory or writeback work commits. If a flushed instruction remains valid, it can appear in the retirement stream even if its register write is blocked elsewhere.

## How our tests catch it

The testbench observes retirement at the WB clock boundary. It checks ordered PCs, destination writes, next PC, memory lane data, and absence of wrong-path instructions.

## Hardware interpretation

The record is mostly wires from MEM/WB plus valid qualification. It adds observability, not another execution engine.

## Interview relevance

Explain why a retirement interface is useful for architectural checking and why a project-specific record must not be called RVFI.

## Exercises

1. Should an invalid bubble produce a valid retirement event?
2. What should `rd_value` be when `rd_write=0`?

## Answers

<details><summary>Check your reasoning</summary>

No. It should be zero under the ProofCore record contract.

</details>
