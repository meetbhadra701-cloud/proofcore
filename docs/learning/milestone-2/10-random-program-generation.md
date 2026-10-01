# 10 — Deterministic Random Program Generation

## What is it?

The generator creates small legal instruction sequences from templates, selecting registers, values, and operation choices from a fixed-seed pseudorandom stream.

## Why does ProofCore need it?

Directed tests target known corner cases. Randomized combinations can connect them in new orders, such as a load feeding arithmetic and then a branch.

## Actual implementation

[`generate.py`](../../../tools/programgen/generate.py) uses a specified xorshift64* generator. Six tiers cover arithmetic, dependencies, memory, load-use, control flow, and mixed instruction sequences. The manifest stores each seed, tier, and static instruction count.

## Reproducibility

For a fixed `--seed` and `--count`, the generator emits the same `.case` images and manifest. Default campaign uses seeds 12345 through 12844. Do not add calls to an unseeded random source.

## Program safety

Every case names a `stop_pc`. Forward branches and jumps skip known instructions; backward loops first load a small positive counter and decrement it. The harness has an architectural instruction bound and a separate cycle bound. Hitting either limit is `TIMEOUT`, not PASS.

## What could go wrong?

An uncontrolled backward branch can hang the run. Random immediate fields can accidentally create unsupported encodings. A generated misaligned data access can test a policy unintentionally instead of the intended operation. The templates constrain encodings and offsets to avoid these cases.

## How tests catch it

`make diff-random` actually executes the generated images through both machines. The checked-in generator itself can be rerun with the same seed; failures preserve the exact machine image in the artifact.

## Exercise

Add one bounded branch pattern that deliberately uses an unsigned comparison disagreement, and state the initial values that guarantee its outcome.
