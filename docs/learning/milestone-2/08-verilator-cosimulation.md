# 08 — Verilator Co-Simulation

## What is it?

Verilator translates synthesizable SystemVerilog into a C++ model that a host program can clock and observe. Co-simulation means the C++ harness drives inputs to that generated model while also running the independent reference model.

## Why does ProofCore use it?

The top-level core has simple instruction and data ports. A small C++ driver can supply memory contents, generate clock edges, and observe retirement without introducing a large external verification framework.

## Actual implementation

[`rtl_driver.cpp`](../../../sim/diff/rtl_driver.cpp) owns a generated `Vcore`. `reset()` applies synchronous reset. Each loop drives instruction word and aligned data word, samples retirement before the active edge (the M1 bench's documented convention), and toggles the clock. Stores from RTL pins update only the harness's RTL-side memory at that edge.

## Cycle mental model

At the beginning of a modeled cycle, the driver evaluates combinational logic using current pipeline state. It supplies memory data, evaluates again, observes the current WB retirement, then applies the external memory store and raises clock. After the edge, stage registers change. The next iteration presents memory for the new state.

## What could go wrong?

Sampling only after the rising edge can associate the next WB record with the wrong architectural step. Updating memory before deriving its address/data can use stale pins. Failing to reset between cases leaks register state across programs.

## Toolchain note

The installed Verilator 5.032 runtime has two `%p` format warnings with GCC 15.2. The harness build downgrades only that format warning from an error for the bundled Verilator support code. The reference-model test build remains full `-Werror`.

## How tests catch it and exercise

The harness reruns many independent programs through one DUT, resetting it each time. Exercise: sketch the order of combinational memory drive, retirement sample, store commit, and rising edge for one cycle.
