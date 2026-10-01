# 17 — Debugging ProofCore systematically

## What is debugging here?

Debugging is finding the earliest point where actual behavior stops matching the contract. For a pipeline, do not begin by staring at the final wrong register value. Find which instruction first got the wrong operand, decision, address, or validity.

## Why does ProofCore need a method?

A final wrong result may be caused many cycles earlier. For example, the final ADD may be correct but receive an old forwarded value. A memory byte may be wrong because the address lane was wrong in EX or the mask was wrong in MEM.

## Mental model

Follow an instruction card through the stages. At each boundary ask: Is it valid? What is its PC? What are its operands? Which controls travel with it? What can it legally change next?

## Practical debugging order

1. Reproduce with the smallest directed program that still fails.
2. Write the expected retirement PC/value/memory sequence.
3. Find the first retirement mismatch or unexpected memory update.
4. Work backward one stage at a time: WB value, MEM result, EX result/decision, ID controls/operands, fetched instruction/PC.
5. Check validity and producer age before debating arithmetic.
6. Change the RTL to fix the cause. Never relax the assertion to turn a failure green.
7. Re-run the failing test, then the complete `make verify` gate.

## Common symptom map

| Symptom | First places to inspect |
| --- | --- |
| `add` after `add` is wrong | EX forwarding selectors, producer valid/write/rd, EX/MEM priority |
| Wrong only after `lw` | load-use detector, PC/IFID hold, ID/EX bubble, MEM/WB load selection |
| Wrong branch direction | forwarded operands, signed versus unsigned compare, funct3 decode |
| Branch target is off | branch PC carried in ID/EX, B-immediate reassembly, byte offset add |
| Wrong-path store | redirect timing, IF/ID and ID/EX valid flush, `dmem_write_mask` valid guard |
| Only byte/half loads fail | aligned data word, offset lane, extension bit, read mask |
| Neighboring byte changes | store width mask, offset shift, payload mask |
| One instruction appears twice | stall hold/advance balance, IF/ID valid capture, redirect PC priority |
| Writes to x0 seem to matter | register-file write guard and forwarding `rd != 0` exclusion |

## Actual implementation signals

The retirement interface in [rtl/core.sv](../../../rtl/core.sv) is the first observation point. Verilator builds the design with tracing disabled by default; for deeper work add a waveform option to the runner and inspect PC/valid/control at stage registers. Do not add a waveform-only behavior change to RTL.

## Example failure walk

If x6 is wrong in `lw x5,0(x1); add x6,x5,x2`, first inspect the retirement of the ADD. If the PC and destination are correct but value is wrong, inspect the selected EX rs1 in the same cycle. If it equals the load address, the EX/MEM load was forwarded incorrectly. If it equals the prior x5 value, the consumer was not held until MEM/WB could provide the data.

## How our tests help

Each scenario has a named program shape and checks semantics: exact retirement order, register effects, memory lanes, or cycle count. See [tb/programs/README.md](../../../tb/programs/README.md). The command `make verify` re-runs all gates.

## Interview relevance

Interviewers value a disciplined “first divergence” explanation: state the stimulus, expected/actual value, the earliest wrong stage, and why the proposed fix addresses the cause.

## Exercises

1. A branch retires at the correct PC but reports the wrong `next_pc`. Which stage should you inspect first?
2. A byte store changes the correct byte, but the retirement write data has unrelated high bytes set. Is architectural memory wrong? Is the interface record still clean?

<details><summary>Answers</summary>

1. EX branch condition/target calculation and the PC/immediate carried into EX.
2. The write mask may protect memory, but the specified retirement record requires lane-positioned data with nonselected bits zero; fix the RTL and retain the assertion.

</details>
