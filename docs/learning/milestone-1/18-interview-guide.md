# 18 — Interview practice guide

These are practice prompts modeled on common ASIC design, verification, and formal concepts. They are not claims about private interview question banks or guaranteed questions from any named employer. Every answer is grounded in this ProofCore implementation.

## Verification and processor design prompts

### Apple silicon, formal, and CAD-style practice

**Question:** How would you determine whether a wrong retirement value came from a bad ALU result or a bad forwarding choice?

<details><summary>Answer</summary>

Use a directed dependent sequence and observe the consumer's operands, forwarding selector, EX result, and WB retirement. If EX operands are already wrong, investigate forwarding; if operands are right but the result is wrong, investigate the ALU operation and signedness. This is simulation localization, not a formal proof.

</details>

**Question:** What property would you eventually assert about x0?

<details><summary>Answer</summary>

Every architectural read of x0 is zero, and no retirement event claims a write to x0. M1 tests this behavior but does not implement a formal assertion framework.

</details>

### NVIDIA hardware verification-style practice

**Question:** How would you test that a taken branch cannot commit a wrong-path store?

<details><summary>Answer</summary>

Place a store in the younger path, take the branch, initialize memory to a known value, then check the byte array, retirement PCs, and store write mask. All must show no wrong-path effect.

</details>

**Question:** Why use directed edge values for signed/unsigned comparisons?

<details><summary>Answer</summary>

Values with bit 31 set have opposite ordering depending on whether interpreted as signed or unsigned. Ordinary positive values may not expose a mistaken cast.

</details>

### Amazon Annapurna ASIC-style practice

**Question:** What does a single-cycle data-memory assumption simplify, and what real system concerns does it omit?

<details><summary>Answer</summary>

It removes ready/wait-state control and makes one load-use stall sufficient. It omits bus arbitration, variable latency, caches, and memory ordering interactions.

</details>

**Question:** What structures does the register file likely become in this synthesis run?

<details><summary>Answer</summary>

The observed Yosys generic netlist contains 992 resettable storage bits for x1-x31 and read-selection logic. A real target may use different mapped cells or a memory macro; this run does not select a physical library.

</details>

### Qualcomm DV/CAD-style practice

**Question:** How do you avoid false load-use stalls on immediate instructions?

<details><summary>Answer</summary>

The decoder emits `uses_rs1` and `uses_rs2`. The hazard unit compares a source only when the instruction actually reads it; immediate bits are not treated as an rs2 dependency.

</details>

**Question:** What does a passing Yosys run prove?

<details><summary>Answer</summary>

It shows Yosys can elaborate and synthesize this design and found no structural check problems in that run. It does not prove ISA correctness or absence of bugs.

</details>

### AMD design-verification-style practice

**Question:** How does the testbench verify the memory interface?

<details><summary>Answer</summary>

It supplies the aligned containing word, applies only lanes enabled by the 4-bit mask at a rising edge, and checks both the resulting bytes and retirement masks/data.

</details>

**Question:** Why check retirement order in addition to final register contents?

<details><summary>Answer</summary>

A wrong-path instruction might be overwritten later or leave final registers apparently correct. The retirement stream reveals lost, duplicated, reordered, or flushed instructions.

</details>

## Whiteboard exercises

### Design a forwarding unit

For each source, compare the needed register number with valid nonzero destinations in EX/MEM and MEM/WB. Exclude an EX/MEM load because its data is not ready. Give EX/MEM first priority, then MEM/WB, else the register-file value.

### Detect a load-use hazard

When IF/ID and ID/EX are valid, ID/EX is a load to nonzero `rd`, and an actually used IF/ID source equals that `rd`, hold PC and IF/ID and inject an invalid ID/EX entry.

### Draw a pipeline execution

Draw cycles as rows and stages as columns. Mark when each producer value is available and note each redirect or bubble. Do not assume all five stages contain valid instructions.

### Implement an ALU operation

Show the operation encoding, operand widths, result width, signed interpretation if any, and two boundary tests. For SRA, show how the sign bit fills vacated upper positions.

### Explain setup and hold conceptually

Data must be stable for a small interval before and after a capturing clock edge. A timing path that misses those windows can be sampled incorrectly. M1 does not run static timing analysis, so this is conceptual background only.

### Identify a signedness bug

Compare `0x80000000` and `1`. Signed comparison says the first is smaller; unsigned comparison says it is larger. Inspect declarations and casts at the comparison operator.

### Debug a wrong-path commit

Find the branch's EX cycle, then inspect redirect, IF/ID flush, ID/EX flush, valid bits, memory mask, register-write enable, and retirement valid. The first invalidated stage that stays valid is a likely source.

## Further practice

Answer the 48 questions in `FINAL_TEACHBACK.md` without looking at the answer sections in this guide. Mark your own progress separately; the project does not infer understanding from reading.
