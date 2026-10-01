# 13 — Eight complete instruction walk-throughs

These examples use the stage notation IF, ID, EX, MEM, WB. A stage entry names the instruction occupying it during that cycle. A dash is an empty stage. A displayed retirement value commits at WB.

## Example A — Independent arithmetic

```asm
addi x1, x0, 5
addi x2, x0, 8
add  x3, x1, x2
```

| Cycle | IF | ID | EX | MEM | WB |
| ---: | --- | --- | --- | --- | --- |
| 1 | I1 | — | — | — | — |
| 2 | I2 | I1 | — | — | — |
| 3 | I3 | I2 | I1 | — | — |
| 4 | next | I3 | I2 | I1 | — |
| 5 | next | next | I3 | I2 | I1: x1=5 |
| 6 | — | — | — | I3 | I2: x2=8 |
| 7 | — | — | — | — | I3: x3=13 |

The third instruction reads x1/x2 before they are committed. In EX it receives x1 from MEM/WB and x2 from EX/MEM. It adds 5+8.

## Example B — EX/MEM forwarding

```asm
add x5, x1, x2
sub x6, x5, x3
```

| Cycle | IF | ID | EX | MEM | WB |
| ---: | --- | --- | --- | --- | --- |
| 1 | ADD | — | — | — | — |
| 2 | SUB | ADD | — | — | — |
| 3 | next | SUB | ADD | — | — |
| 4 | next | next | SUB | ADD | — |
| 5 | — | — | — | SUB | ADD: x5 result |

In cycle 4, SUB's EX operand mux selects the ADD result from EX/MEM. It does not use the old x5 value read in ID.

## Example C — Forwarding priority

```asm
addi x1, x0, 1
addi x1, x1, 1
add  x2, x1, x1
```

| Cycle | IF | ID | EX | MEM | WB |
| ---: | --- | --- | --- | --- | --- |
| 1 | I1 | — | — | — | — |
| 2 | I2 | I1 | — | — | — |
| 3 | I3 | I2 | I1 | — | — |
| 4 | next | I3 | I2 | I1 | — |
| 5 | — | — | I3 | I2: x1=2 | I1: x1=1 |
| 6 | — | — | — | I3: x2=4 | I2: x1=2 |

Both forwarding comparisons match x1 in cycle 5. EX/MEM is newer, so 2 wins over 1 for both operands. The architectural result is x2=4.

## Example D — Load-use stall

```asm
lw  x5, 0(x1)
add x6, x5, x2
```

| Cycle | IF | ID | EX | MEM | WB |
| ---: | --- | --- | --- | --- | --- |
| 1 | LW | — | — | — | — |
| 2 | ADD | LW | — | — | — |
| 3 | next | ADD | LW | — | — |
| 4 | held | ADD held | bubble | LW | — |
| 5 | target stream | next | ADD | — | LW data |
| 6 | — | — | next | ADD | — |
| 7 | — | — | — | next | ADD result |

The load data is not ready when ADD first reaches EX. The interlock holds PC and IF/ID for one cycle and injects an invalid ID/EX entry. In cycle 5, the load data can be forwarded from MEM/WB.

## Example E — Taken branch

```asm
beq x1, x2, target
addi x3, x0, 99
sw   x3, 0(x0)
target: addi x4, x0, 7
```

| Cycle | IF | ID | EX | MEM | WB |
| ---: | --- | --- | --- | --- | --- |
| 1 | BEQ | — | — | — | — |
| 2 | ADDI | BEQ | — | — | — |
| 3 | SW | ADDI | BEQ (taken) | — | — |
| 4 | target fetch | bubble | bubble | BEQ | — |
| 5 | next | target ADDI | — | — | BEQ |
| 6 | — | — | target ADDI | — | — |
| 7 | — | — | — | target ADDI | — |
| 8 | — | — | — | — | target ADDI |

The ADDI in ID and SW in IF are younger than BEQ, so both valid bits are cleared. The target begins again at the selected address.

## Example F — Wrong-path store prevention

Use Example E with memory byte zero initialized to `0x55`. If the flush is correct, the `sw` never becomes a valid MEM operation, memory remains `0x55`, and no retirement entry has the store's PC. The testbench checks all three observations: memory, write mask, and retirement order.

## Example G — JAL and JALR

```asm
jal  x5, target       # at PC 0x00; target=0x08, x5=0x04
addi x6, x0, 99       # flushed
target: addi x7, x0, 65 # PC 0x08; odd base tests JALR bit-zero clearing
jalr x9, 0(x7)        # target becomes 64; x9=PC+4=16
...                  # PC 64 contains the next instruction
```

JAL computes target from its own PC and J-immediate. JALR adds a forwarded rs1 value and I-immediate, clears bit zero, and writes its own `PC+4` to rd. ProofCore's regression sets x7 to 65 and confirms JALR reaches aligned PC 64 after clearing bit zero.

## Example H — Signed versus unsigned branch

Let x1=`0x80000000` and x2=`0x00000001`. As signed values, x1 is -2,147,483,648, so BLT x1,x2 is true. As unsigned values, x1 is 2,147,483,648, so BLTU x1,x2 is false. Same bits; different interpretation. The regression exercises this disagreement.

## What could go wrong?

Wrong-stage forwarding, a missing bubble, or an uncleared valid bit produces different values or extra retirement PCs. A cycle table makes the first bad stage visible.

## How our tests catch it

The corresponding nine directed program scenarios are listed in [tb/programs/README.md](../../../tb/programs/README.md). Each checks committed results rather than assuming internal mux encodings.

## Interview relevance and exercises

1. Draw Example C and identify which producer wins.
2. Add a `lw` to Example E's branch operand and mark the extra bubble.

<details><summary>Answers</summary>

Example C uses the second ADDI's result from EX/MEM. A branch using a load destination incurs the same one-cycle hold before branch comparison in EX.

</details>
