# RV32I subset decode table

The decoder matches opcode, `funct3`, and (where required) `funct7`. Any row not explicitly listed is unsupported and follows the inert sequential no-op policy in `SPEC.md`.

| Instruction(s) | opcode | funct3 | funct7 / immediate constraint | Main controls |
| --- | --- | --- | --- | --- |
| ADD / SUB | `0110011` | `000` | `0000000` / `0100000` | rs1, rs2, rd; ALU add/sub; write rd |
| SLL | `0110011` | `001` | `0000000` | rs1, rs2, rd; ALU shift-left |
| SLT / SLTU | `0110011` | `010` / `011` | `0000000` | rs1, rs2, rd; signed / unsigned compare |
| XOR / OR / AND | `0110011` | `100` / `110` / `111` | `0000000` | rs1, rs2, rd; bitwise ALU |
| SRL / SRA | `0110011` | `101` | `0000000` / `0100000` | rs1, rs2, rd; logical / arithmetic shift |
| ADDI / SLTI / SLTIU / XORI / ORI / ANDI | `0010011` | `000` / `010` / `011` / `100` / `110` / `111` | I-immediate | rs1, rd; immediate ALU; write rd |
| SLLI | `0010011` | `001` | upper immediate bits `0000000` | rs1, rd; shamt=`insn[24:20]` |
| SRLI / SRAI | `0010011` | `101` | upper immediate bits `0000000` / `0100000` | rs1, rd; shamt=`insn[24:20]` |
| LB / LH / LW | `0000011` | `000` / `001` / `010` | I-immediate | rs1, rd; memory read; signed extension |
| LBU / LHU | `0000011` | `100` / `101` | I-immediate | rs1, rd; memory read; zero extension |
| SB / SH / SW | `0100011` | `000` / `001` / `010` | S-immediate | rs1, rs2; memory write; lane mask |
| BEQ / BNE | `1100011` | `000` / `001` | B-immediate | rs1, rs2; equality branch |
| BLT / BGE | `1100011` | `100` / `101` | B-immediate | rs1, rs2; signed branch |
| BLTU / BGEU | `1100011` | `110` / `111` | B-immediate | rs1, rs2; unsigned branch |
| JAL | `1101111` | — | J-immediate | rd=`PC+4`; redirect from PC |
| JALR | `1100111` | `000` | I-immediate | rs1, rd=`PC+4`; target from rs1+imm |
| LUI | `0110111` | — | U-immediate | rd gets U-immediate |
| AUIPC | `0010111` | — | U-immediate | rd gets PC+U-immediate |

The bit fields are intentionally decoded in one module rather than repeated in `core.sv`. The `uses_rs1` and `uses_rs2` outputs matter to load-use detection: immediate bit positions are not mistakenly treated as register numbers.
