# 05 — Decoding instructions

## What is it?

Decoding means reading an instruction's bit fields and deciding which operations and data sources it needs. The result is a bundle of control signals.

## Why does ProofCore need it?

The core must distinguish, for example, an ADD from a load even though both have source and destination register fields. The decoder is also the single place that says which exact opcode/funct combinations ProofCore supports.

## Mental model

The instruction is a form. Opcode is the form category; `funct3` and sometimes `funct7` are the detailed choice. Decode reads the form, then sends a small checklist down the pipeline: “use rs1,” “read memory,” “write rd,” and so on.

## Actual implementation

The decode logic is in [rtl/decoder.sv](../../../rtl/decoder.sv); the readable mapping is [docs/architecture/decode-table.md](../../architecture/decode-table.md). The packed `decode_t` structure in [rtl/proofcore_pkg.sv](../../../rtl/proofcore_pkg.sv) includes source-use bits, ALU/immediate selections, branch type, memory direction/size, and writeback selection.

## Important code

The decoder begins with safe controls: no legality, no writes, no memory. It then enables controls only for a recognized instruction. For example, a load marks `uses_rs1`, `use_imm`, `mem_read`, and `reg_write`; the `funct3` selects byte/half/word size and signed/unsigned extension.

`uses_rs1` and `uses_rs2` are important for hazards. A store's immediate bits occupy places that look like register fields, but decode says whether those fields are real operands. That prevents false stalls.

## Concrete example

For `add x6, x5, x5`, decode says `uses_rs1=1`, `uses_rs2=1`, `reg_write=1`, `alu_op=ADD`. For `addi x6, x5, 7`, it says `uses_rs1=1`, `uses_rs2=0`, `use_imm=1`. The second instruction must not claim that bits containing its immediate name a second source register.

## What could go wrong?

If an invalid funct7 pattern is accepted as ADD, an unsupported encoding can change state. If an immediate instruction incorrectly sets `uses_rs2`, a load-use interlock may stall because an immediate bit field happens to equal a load destination.

## How our tests catch it

Directed programs execute every supported instruction family and include `0xFFFFFFFF` as an unsupported word. Hazard tests check real dependencies and the x0 false-dependency case. The Markdown decode table is reviewed against the decoder case arms.

## Hardware interpretation

The opcode/funct comparisons become equality gates. The control struct is a set of wires, and selection among its values becomes mux-control wiring. No table memory is required.

## Interview relevance

Be ready to trace opcode, `funct3`, and `funct7` from an instruction word into ALU, memory, and register-write controls.

## Exercises

1. Why does `lw` use rs1 but not rs2?
2. Why does the hazard unit need `uses_rs2` rather than just compare instruction bits `[24:20]`?

## Answers

<details><summary>Check your reasoning</summary>

A load computes its address from one base register and an immediate. The `[24:20]` bits can be part of an immediate, so comparing them without checking source use creates false dependencies.

</details>
