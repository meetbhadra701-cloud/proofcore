# 15 — Milestone 2 Interview Guide

These questions stay within the implementation in this repository. Answers are separated so you can try each question before reading them.

## Questions

1. Why is a software reference model architecturally sequential while the DUT is pipelined?
2. What is a common-mode error in differential verification?
3. Why compare at retirement instead of on every clock?
4. How does the harness avoid applying a store twice?
5. Which signed C++ operations require extra care for RV32I?
6. What is the difference between the JAL link value and its target PC?
7. How does the driver know how many instructions to expect from a bounded program?
8. What does the comparator self-test validate?
9. What does a 500-program passing campaign establish, and what does it not establish?
10. How would you extend the comparator if retirement gains an exception field later?

## Answer notes

<details><summary>Show answers</summary>

1. RTL cycles include stalls and bubbles; architectural instructions still have one ordered result each. The simpler sequential model reduces shared implementation assumptions.
2. It is a defect repeated in both DUT and oracle, causing false agreement. Independent decode/execute structure and focused boundary tests reduce this risk.
3. Internal stage contents are temporary and can be invalid or flushed. Retirement marks a committed architectural instruction and permits different cycle timing.
4. The RTL and reference use separate memory objects. RTL memory changes only from its own byte-mask port; reference memory changes only when the interpreter executes a store.
5. Signed comparisons, arithmetic right shifts, overflow, and shifting by large amounts. Use unsigned modular arithmetic, deliberate bit reinterpretation, and shift masks.
6. JAL writes `PC+4` to rd, but transfers to `PC+sign_extended_J_imm`.
7. The reference walks its architectural PC until the case's `stop_pc`, with an instruction cap; the RTL run has a separate cycle cap.
8. That a deliberate changed comparison event is noticed and produces a useful artifact. It does not validate every comparator field independently.
9. It gives empirical evidence for those exact generated cases and seeds. It is neither exhaustive nor a formal proof.
10. Define when the field is meaningful, add it to the C++ event and RTL mapping, compare it semantically, and add a directed case that exercises both values.

</details>

## Whiteboard exercises

- Sketch two memories and mark which component is allowed to write each one.
- Write a comparator table showing when `rd_value`, `mem_addr`, and `mem_wdata` matter.
- Explain what happens when a load-use stall inserts a bubble while the reference waits.
- Add an instruction and immediate decoder test for one reserved encoding without changing the RTL.

## How tests support these answers

`make reference-test`, `make diff-directed`, `make comparator-selftest`, and `make diff-random` each target a different layer. Run each separately before relying on the top-level `make verify` result.
