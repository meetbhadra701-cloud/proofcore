#pragma once

#include <cstdint>
#include <string>

namespace proofcore::reference {

enum class Operation : std::uint8_t {
  Illegal,
  Add, Sub, Sll, Slt, Sltu, Xor, Srl, Sra, Or, And,
  Addi, Slti, Sltiu, Xori, Ori, Andi, Slli, Srli, Srai,
  Lb, Lh, Lw, Lbu, Lhu, Sb, Sh, Sw,
  Beq, Bne, Blt, Bge, Bltu, Bgeu, Jal, Jalr, Lui, Auipc
};

struct DecodedInstruction {
  Operation operation{Operation::Illegal};
  std::uint8_t rd{};
  std::uint8_t rs1{};
  std::uint8_t rs2{};
  std::uint32_t immediate{};
};

[[nodiscard]] DecodedInstruction decode_instruction(std::uint32_t instruction) noexcept;
[[nodiscard]] std::uint32_t decode_immediate(std::uint32_t instruction, char format) noexcept;
[[nodiscard]] const char* operation_name(Operation operation) noexcept;
[[nodiscard]] std::string disassemble(std::uint32_t pc, std::uint32_t instruction);

}  // namespace proofcore::reference
