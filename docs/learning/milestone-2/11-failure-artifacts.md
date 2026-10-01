# 11 — Reading a Differential Failure Artifact

## What is it?

A failure artifact is a self-contained record of a disagreement: which seed and retirement failed, what each side reported, what reference registers existed beforehand, and the program/memory needed for replay.

## Why does ProofCore need it?

“The test failed” is not enough for debugging. A reproducible artifact lets an engineer inspect the first wrong architectural event without hoping a later random run recreates it.

## Actual implementation

[`trace_writer.cpp`](../../../sim/diff/trace_writer.cpp) writes JSON, Markdown, seed, a replayable `.case`, little-endian `program.bin`, word-oriented `program.hex`, subset disassembly `program.s`, initial-memory description/overrides, and recent retirement JSONL.

## Real self-test example

The explicit comparator test runs:

```asm
addi x5, x0, 7
add  x6, x5, x5
```

At retirement 2 the expected value is `x6=14`. Test-only corruption changes the sampled comparison copy to `x6=7`. The artifact records expected `0x0000000e`, observed `0x00000007`, and mismatching field `rd_value`.

## Replay and limits

`failure.md` prints a replay command using the preserved `.case`. The self-test replay includes its explicit corruption flag. The artifact proves the checker plumbing detected a difference. It does not show a real RTL bug, minimize the program, or identify a cause.

## What could go wrong?

An artifact missing initial memory would not reproduce a load discrepancy. A missing seed would make the generator run opaque. A vague mismatch without expected/observed records would slow triage. Those facts are kept together here.

## Exercise

Open a generated self-test `failure.json` after `make comparator-selftest`. Find the PC, retirement index, expected/observed values, register state before the step, and the flag that marks test-only corruption.
