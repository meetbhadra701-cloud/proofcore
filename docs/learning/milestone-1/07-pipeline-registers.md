# 07 — Pipeline registers and validity

## What are they?

Pipeline registers are flip-flops between stages. They remember an instruction's values until the next clock edge. ProofCore carries a valid bit beside each payload.

## Why does ProofCore need them?

Combinational logic needs time to settle. A clock edge stores its result and starts the next stage's work. Valid bits distinguish a real instruction from a bubble caused by reset, a stall, or a flush.

## Mental model

Each boundary is a tray holding one instruction card and its data. `valid=1` means there is a card to process. `valid=0` means the tray is empty, regardless of leftover bits.

## Actual implementation

The stage registers are [rtl/pipeline/if_id.sv](../../../rtl/pipeline/if_id.sv), `id_ex.sv`, `ex_mem.sv`, and `mem_wb.sv`. They accept packed vectors at their module boundary because Yosys 0.52 cannot elaborate typed struct ports; the core connects those vectors to named packed structs in [rtl/core.sv](../../../rtl/core.sv).

IF/ID has an explicit hold input for a load-use stall and a flush input for redirect. ID/EX is loaded invalid during a stall and invalidated on redirect. EX/MEM and MEM/WB advance every cycle so older work is not stopped by a front-end stall.

## Concrete example

When a load is in EX and its consumer is in ID, the rising edge moves the load into EX/MEM, keeps IF/ID unchanged, and puts `valid=0` in ID/EX. The consumer stays on the tray for one more beat. The invalid ID/EX entry is a bubble.

## What could go wrong?

Holding every stage during a load-use stall also holds the load in EX forever. Flushing the EX/MEM stage on a branch can cancel an older instruction that should still complete. Each boundary needs the correct control priority.

## How our tests catch it

Cycle-count assertions check that ordinary sequences do not stall, true load-use sequences add one cycle, and x0 dependencies do not. Retirement PC sequences verify that a stall neither loses nor duplicates the held instruction.

## Hardware interpretation

Payloads and valid bits are flip-flops. Hold uses clock-enable behavior or feedback muxes. Flush/reset logic selects zero validity. The payload can be zeroed for clarity, but the valid bit is the correctness condition.

## Interview relevance

Explain the difference between a bubble and a valid NOP instruction. Discuss which stages must hold and which must advance for a load-use stall.

## Exercises

1. Why can invalid payload bits be harmless?
2. During a front-end stall, should EX/MEM stop advancing?

## Answers

<details><summary>Check your reasoning</summary>

All side effects are qualified by valid, so invalid bits cannot write state. EX/MEM must advance the older EX instruction, such as the load, toward memory.

</details>
