# 00 — Milestone 2 at a Glance

## What is it?

Milestone 2 gives ProofCore a second implementation of the processor's architectural rules. One implementation is the five-stage RTL. The other is a small C++ program that executes one whole instruction at a time. After each RTL instruction retires, a comparator asks whether both descriptions agree on what that instruction did.

## Why does ProofCore need it?

M1's directed tests know the answers for hand-selected examples. M2 adds a reusable oracle for larger programs. A wrong forwarding decision, a missed flush, or a bad load value can cause the RTL retirement stream to diverge from the reference.

## Mental model

Imagine two people solving the same arithmetic worksheet. One follows a five-step assembly line; the other solves one problem at a time. They do not need to take the same amount of time. They must agree on each answer in order.

## Actual implementation

The reference lives in [`reference/`](../../../reference/README.md). The Verilator bridge and comparator live in [`sim/diff/`](../../../sim/diff/README.md). The architectural boundary remains the M1 retirement record; no RTL pipeline design was replaced.

## Important behavior

`Rv32iModel::step()` fetches at its own PC, decodes, performs one architectural operation, updates its own registers/memory, and returns a `RetireEvent`. `RtlDriver::run()` advances that model only when `retire_valid` is high. A cycle with a bubble or stall does not advance the model.

## Concrete example

For `addi x5,x0,7; add x6,x5,x5`, the reference produces two events: `(PC=0, x5=7, nextPC=4)` and `(PC=4, x6=14, nextPC=8)`. The RTL may take several clock edges to produce those events. Only their architectural order and contents need to match.

## What could go wrong?

If the software model copies the RTL's bad forwarding rule, both can agree while both are wrong. Independence comes from describing architectural instruction semantics directly, not from merely writing the same pipeline logic in another language.

## How tests catch it

`make diff-directed` compares nine M1-derived programs. `make diff-random` runs seeded generated programs. `make comparator-selftest` proves a deliberate change to a sampled retirement event is detected.

## Hardware and interview relevance

The model is not hardware. It is a verification oracle. A natural interview question is: “Why does a reference model not need to model pipeline stalls?” Because stalls change time, not the architectural result sequence.

## Mini exercise

For `addi x1,x0,3; addi x2,x1,4`, write the two expected retirement records without thinking about cycles.
