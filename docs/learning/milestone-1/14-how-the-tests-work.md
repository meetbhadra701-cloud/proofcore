# 14 — How the regression works

## What is it?

A testbench is a small environment around the processor. It supplies clock/reset, instructions, and memory data, then checks what the processor commits.

## Why does ProofCore need it?

RTL that compiles can still calculate wrong values, lose instructions, or corrupt memory. Simulation runs actual instruction sequences and checks visible architectural behavior.

## Mental model

The testbench acts as the outside memory system and keeps a notebook of every retirement receipt. It knows which program is loaded and which bytes should change.

## Actual implementation

See [tb/cocotb/test_core.py](../../../tb/cocotb/test_core.py) and [scripts/run_cocotb.py](../../../scripts/run_cocotb.py). Python helpers encode I/R/S/B/U/J instruction formats. A `Bench` object serves instructions by PC, returns aligned data words, applies masked stores at rising edges, and records retirement fields.

The regression has one Cocotb test that runs nine directed program scenarios. Checks cover ALU operations and boundaries, forwarding priority, load-use stalls, all branches in both outcomes, backward branches, JAL/JALR, wrong-path effects, all load/store widths, lane masks, and unsupported/misaligned no-effect behavior.

## Concrete example

For a store, the testbench reads `dmem_write_mask` and `dmem_write_data` before the edge, updates only selected bytes, and then compares the resulting memory byte array. It separately checks the store's retirement record. A correct final byte with an incorrect retirement mask would therefore still fail.

## What could go wrong?

If the testbench fetches the instruction using the previous PC, it tests a different program. If it records retirement after the wrong edge, it can report a cycle off. If it only checks the final register and not the retirement stream, a flushed instruction may escape detection.

## How our tests catch it

The tests assert exact retirement PC sequences, destination values, memory contents, lane masks, and cycle counts for no-stall and load-use examples. They have directed cases; they are not random-only tests.

## Hardware interpretation

The cocotb code is not synthesized. It drives simulation inputs and observes RTL outputs through VPI. The RTL alone is the hardware being synthesized.

## Reproduction

```sh
make test
```

## Interview relevance

Explain why checking architectural retirement is stronger than peeking at one internal wire, and why directed edge cases are valuable.

## Exercises

1. Why is the initial memory word asymmetric (`0x80007F80`)?
2. What bug would exact retirement-PC checking catch?

<details><summary>Answers</summary>

The asymmetric bytes expose wrong lane selection and sign extension. PC checking catches lost, duplicated, or wrong-path instructions.

</details>
