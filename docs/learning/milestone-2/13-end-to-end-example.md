# 13 — End-to-End Example: From Program to Compared Retirements

The example initializes x10 as a data pointer, then follows the requested dependent arithmetic/store/load sequence.

```asm
00000000: 00001537    lui  x10,0x1000
00000004: 00700293    addi x5,x0,7
00000008: 00528333    add  x6,x5,x5
0000000c: 00652023    sw   x6,0(x10)
00000010: 00052383    lw   x7,0(x10)
00000014: 40538433    sub  x8,x7,x5
```

All values are actual RV32I encodings. Initially, `x0=0` and memory at `0x1000` has the deterministic initial byte pattern. After execution: `x5=7`, `x6=14`, `memory[0x1000]=14`, `x7=14`, and `x8=7`.

## RTL pipeline movement

`I0` is LUI, `I1` ADDI, `I2` ADD, `I3` SW, `I4` LW, and `I5` SUB. Stage columns describe the instruction occupying that stage during each cycle. `—` is empty; `hold` means the front end retains the load consumer while an invalid slot passes through EX.

| Cycle | IF | ID | EX | MEM | WB |
| ---: | --- | --- | --- | --- | --- |
| 1 | I0 | — | — | — | — |
| 2 | I1 | I0 | — | — | — |
| 3 | I2 | I1 | I0 | — | — |
| 4 | I3 | I2 | I1 | I0 | — |
| 5 | I4 | I3 | I2 | I1 | I0 |
| 6 | I5 | I4 | I3 | I2 | I1 |
| 7 | hold | I5 | I4 (load) | I3 (store) | I2 |
| 8 | hold | I5 | bubble | I4 (load) | I3 |
| 9 | next | next | I5 | bubble | I4 |
| 10 | — | — | — | I5 | bubble |
| 11 | — | — | — | — | I5 |

At cycle 5, I2's EX operands need the just-computed x5; EX/MEM forwarding supplies 7. At cycle 7, I5 wants x7 while I4 is a load in EX, so the hazard unit holds the front end for one cycle. At cycle 9, the loaded value is available from MEM/WB forwarding to I5's EX operand.

## RTL retirement versus reference steps

The RTL emits one valid WB record at a time. For each record, the reference fetches at its own PC and executes one complete instruction. The event comparison checks the instruction identity, architectural write, memory metadata when present, and next PC. The reference does not recreate the table above.

| Retirement PC | Instruction | Architectural effect | Next PC |
| ---: | --- | --- | ---: |
| `0x00` | LUI x10,0x1000 | x10 = `0x1000` | `0x04` |
| `0x04` | ADDI x5,x0,7 | x5 = 7 | `0x08` |
| `0x08` | ADD x6,x5,x5 | x6 = 14 | `0x0c` |
| `0x0c` | SW x6,0(x10) | word at `0x1000` = 14 | `0x10` |
| `0x10` | LW x7,0(x10) | x7 = 14 | `0x14` |
| `0x14` | SUB x8,x7,x5 | x8 = 7 | `0x18` |

The store reaches RTL memory in MEM before its retirement event; its reference copy is updated when the model steps that store event. The memories are separate. The later load must return 14 on both sides, and the final byte memories must match.

## What would a bug look like?

If the ADD forwarding mux supplies stale x5=0, the ADD retirement differs at PC `0x08`. If the load-use stall is missing, the SUB can retire x8 with stale x7. If the store mask/data lane is wrong, the store event or later load value disagrees. The first mismatch is the debugging point; this milestone does not automatically identify the responsible RTL decision.

## Exercise

Change the initial x5 value to 9 and predict x6, the memory word, x7, and x8. Then state which instruction pair requires EX forwarding and which pair requires a stall.
