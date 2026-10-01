# 01 — Why Differential Testing?

## What is it?

Differential testing runs two implementations on the same input and compares their observable results. Here, “differential” means comparing ProofCore RTL with the independent C++ RV32I model.

## Why does ProofCore need it?

A testbench that checks only final register values can miss where and how a bug first appeared. Retirement comparison checks each architectural instruction in order, which narrows a failure to its first visible divergence.

## Mental model

Two route planners receive the same map and instructions. One takes highways, one takes local roads. Their paths differ, but their arrival points and delivered packages should agree at every named checkpoint.

## Actual implementation

[`comparator.cpp`](../../../sim/diff/comparator.cpp) compares a reference `RetireEvent` with a sampled RTL event. It checks the fields that represent architectural effects and skips values that are defined as irrelevant when no write or memory operation occurs.

## Why retirement is the checkpoint

Internal RTL values can be temporary or stale. A pipeline register may contain an instruction that will be flushed. Forwarding muxes and bubbles have no direct architectural equivalent. At retirement, the machine says an instruction is committing; its PC, bits, destination write, memory operation, and next PC describe the architectural action.

## Concrete example

If the RTL returns `x6=7` for `add x6,x5,x5` while the reference returns `14`, the first differing retirement exposes the error before later instructions make the final state harder to interpret.

## Failure example

If the reference model accidentally uses signed comparison for `SLTU`, then a program comparing `0xffffffff` with `1` can reveal a disagreement with correct RTL. If both models copied the same mistaken rule, the checker could miss it; this is why decode and semantics are implemented independently.

## How tests catch it

The M1-derived corpus checks edge signedness, all branch predicates, load-use cases, wrong-path suppression, and subword memory. The 500-case campaign repeats varied legal combinations with deterministic seeds.

## Hardware interpretation

The comparator is ordinary host software. It does not synthesize into ProofCore and does not slow the processor. It reads the architectural observation pins in the simulator.

## Interview relevance and exercise

Be ready to explain what differential testing proves empirically and what it cannot prove universally. Exercise: name one RTL bug a final-state-only test might miss but a retirement trace would expose earlier.
