# 14 — Debugging a Mismatch

## Start at the first different event

Open `failure.json` and read `retirement_index`, `cycle`, `expected`, `observed`, and `mismatching_fields`. The retirement index is one-based. The `register_state_before` array belongs to the reference before it applied that event.

## Follow the evidence

1. Read `program.s` around the reported PC. Check the instruction's decoded source/destination registers and immediate.
2. Compare expected versus observed PC/instruction. If these differ, suspect redirect, flush, or instruction/memory setup before arithmetic.
3. If PC and instruction match but `rd_value` differs, inspect operation semantics, forwarding, stall timing, and load data.
4. For memory events, compare effective address, mask, and lane data. For a final memory mismatch, inspect `memory_differences` and compare the corresponding store retirements.
5. Replay using the command in `failure.md`. Keep the original artifact unchanged while debugging.

## Concrete example

For expected `x6=14` and observed `x6=7` at `add x6,x5,x5`, first confirm both streams agree on instruction PC and bits. Then inspect whether both operands were 7 at EX and whether EX/MEM or MEM/WB should have won. The report does not claim which internal condition caused the value.

## What not to conclude

One mismatch is evidence that these implementations disagree for this case. It does not automatically prove which side is wrong, minimize the program, find the first microarchitectural error, or prove a synthesis transformation caused it. The self-test artifact is explicitly marked as a deliberately corrupted comparison sample, not a processor bug.

## How this connects to later work

Later milestones may add witness minimization and causal localization. M2 only preserves the reproducible program, seed, initial memory, mismatch, and recent architectural context those tools could consume.

## Exercise

Pick a `next_pc` mismatch and write two hypotheses: one architectural decode/target hypothesis and one RTL redirect/flush hypothesis. Name one next observation that would distinguish them.
