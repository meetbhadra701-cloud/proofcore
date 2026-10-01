#include "rv32i_decode.hpp"

#include <bit>
#include <iomanip>
#include <sstream>

namespace proofcore::reference {
namespace {

constexpr std::uint32_t field(const std::uint32_t word, const unsigned high,
                              const unsigned low) noexcept {
  const unsigned width = high - low + 1U;
  return (word >> low) & ((1U << width) - 1U);
}

constexpr std::uint32_t sign_extend(const std::uint32_t value,
                                    const unsigned bits) noexcept {
  const std::uint32_t sign = 1U << (bits - 1U);
  const std::uint32_t mask = (1U << bits) - 1U;
  return (value & sign) != 0U ? (value | ~mask) : (value & mask);
}

}  // namespace

std::uint32_t decode_immediate(const std::uint32_t instruction,
                               const char format) noexcept {
  switch (format) {
    case 'I': return sign_extend(field(instruction, 31U, 20U), 12U);
    case 'S': {
      const std::uint32_t imm = (field(instruction, 31U, 25U) << 5U) |
                                field(instruction, 11U, 7U);
      return sign_extend(imm, 12U);
    }
    case 'B': {
      const std::uint32_t imm = (field(instruction, 31U, 31U) << 12U) |
                                (field(instruction, 7U, 7U) << 11U) |
                                (field(instruction, 30U, 25U) << 5U) |
                                (field(instruction, 11U, 8U) << 1U);
      return sign_extend(imm, 13U);
    }
    case 'U': return instruction & 0xfffff000U;
    case 'J': {
      const std::uint32_t imm = (field(instruction, 31U, 31U) << 20U) |
                                (field(instruction, 19U, 12U) << 12U) |
                                (field(instruction, 20U, 20U) << 11U) |
                                (field(instruction, 30U, 21U) << 1U);
      return sign_extend(imm, 21U);
    }
    default: return 0U;
  }
}

DecodedInstruction decode_instruction(const std::uint32_t instruction) noexcept {
  DecodedInstruction decoded{};
  decoded.rd = static_cast<std::uint8_t>(field(instruction, 11U, 7U));
  decoded.rs1 = static_cast<std::uint8_t>(field(instruction, 19U, 15U));
  decoded.rs2 = static_cast<std::uint8_t>(field(instruction, 24U, 20U));
  const std::uint32_t opcode = field(instruction, 6U, 0U);
  const std::uint32_t funct3 = field(instruction, 14U, 12U);
  const std::uint32_t funct7 = field(instruction, 31U, 25U);
  const auto set = [&decoded](const Operation operation) { decoded.operation = operation; };

  switch (opcode) {
    case 0x33U:
      switch (funct3) {
        case 0U: if (funct7 == 0U) set(Operation::Add); else if (funct7 == 0x20U) set(Operation::Sub); break;
        case 1U: if (funct7 == 0U) set(Operation::Sll); break;
        case 2U: if (funct7 == 0U) set(Operation::Slt); break;
        case 3U: if (funct7 == 0U) set(Operation::Sltu); break;
        case 4U: if (funct7 == 0U) set(Operation::Xor); break;
        case 5U: if (funct7 == 0U) set(Operation::Srl); else if (funct7 == 0x20U) set(Operation::Sra); break;
        case 6U: if (funct7 == 0U) set(Operation::Or); break;
        case 7U: if (funct7 == 0U) set(Operation::And); break;
        default: break;
      }
      break;
    case 0x13U:
      decoded.immediate = decode_immediate(instruction, 'I');
      switch (funct3) {
        case 0U: set(Operation::Addi); break;
        case 2U: set(Operation::Slti); break;
        case 3U: set(Operation::Sltiu); break;
        case 4U: set(Operation::Xori); break;
        case 6U: set(Operation::Ori); break;
        case 7U: set(Operation::Andi); break;
        case 1U: if (funct7 == 0U) { set(Operation::Slli); decoded.immediate = field(instruction, 24U, 20U); } break;
        case 5U:
          if (funct7 == 0U) { set(Operation::Srli); decoded.immediate = field(instruction, 24U, 20U); }
          else if (funct7 == 0x20U) { set(Operation::Srai); decoded.immediate = field(instruction, 24U, 20U); }
          break;
        default: break;
      }
      break;
    case 0x03U:
      decoded.immediate = decode_immediate(instruction, 'I');
      if (funct3 == 0U) set(Operation::Lb);
      else if (funct3 == 1U) set(Operation::Lh);
      else if (funct3 == 2U) set(Operation::Lw);
      else if (funct3 == 4U) set(Operation::Lbu);
      else if (funct3 == 5U) set(Operation::Lhu);
      break;
    case 0x23U:
      decoded.immediate = decode_immediate(instruction, 'S');
      if (funct3 == 0U) set(Operation::Sb);
      else if (funct3 == 1U) set(Operation::Sh);
      else if (funct3 == 2U) set(Operation::Sw);
      break;
    case 0x63U:
      decoded.immediate = decode_immediate(instruction, 'B');
      if (funct3 == 0U) set(Operation::Beq);
      else if (funct3 == 1U) set(Operation::Bne);
      else if (funct3 == 4U) set(Operation::Blt);
      else if (funct3 == 5U) set(Operation::Bge);
      else if (funct3 == 6U) set(Operation::Bltu);
      else if (funct3 == 7U) set(Operation::Bgeu);
      break;
    case 0x37U: decoded.immediate = decode_immediate(instruction, 'U'); set(Operation::Lui); break;
    case 0x17U: decoded.immediate = decode_immediate(instruction, 'U'); set(Operation::Auipc); break;
    case 0x6fU: decoded.immediate = decode_immediate(instruction, 'J'); set(Operation::Jal); break;
    case 0x67U:
      if (funct3 == 0U) { decoded.immediate = decode_immediate(instruction, 'I'); set(Operation::Jalr); }
      break;
    default: break;
  }
  return decoded;
}

const char* operation_name(const Operation operation) noexcept {
  switch (operation) {
#define PC_NAME(op) case Operation::op: return #op
    PC_NAME(Illegal); PC_NAME(Add); PC_NAME(Sub); PC_NAME(Sll); PC_NAME(Slt);
    PC_NAME(Sltu); PC_NAME(Xor); PC_NAME(Srl); PC_NAME(Sra); PC_NAME(Or);
    PC_NAME(And); PC_NAME(Addi); PC_NAME(Slti); PC_NAME(Sltiu); PC_NAME(Xori);
    PC_NAME(Ori); PC_NAME(Andi); PC_NAME(Slli); PC_NAME(Srli); PC_NAME(Srai);
    PC_NAME(Lb); PC_NAME(Lh); PC_NAME(Lw); PC_NAME(Lbu); PC_NAME(Lhu);
    PC_NAME(Sb); PC_NAME(Sh); PC_NAME(Sw); PC_NAME(Beq); PC_NAME(Bne);
    PC_NAME(Blt); PC_NAME(Bge); PC_NAME(Bltu); PC_NAME(Bgeu); PC_NAME(Jal);
    PC_NAME(Jalr); PC_NAME(Lui); PC_NAME(Auipc);
#undef PC_NAME
  }
  return "Illegal";
}

std::string disassemble(const std::uint32_t pc, const std::uint32_t instruction) {
  const DecodedInstruction decoded = decode_instruction(instruction);
  std::ostringstream output;
  output << std::hex << std::setfill('0') << std::setw(8) << pc << ": "
         << std::setw(8) << instruction << "  ";
  if (decoded.operation == Operation::Illegal) {
    output << ".word 0x" << std::setw(8) << instruction << " # unsupported -> nop";
    return output.str();
  }
  const char* name = operation_name(decoded.operation);
  std::string mnemonic(name);
  for (char& character : mnemonic) character = static_cast<char>(character | 0x20);
  output << mnemonic;
  switch (decoded.operation) {
    case Operation::Lui: case Operation::Auipc:
      output << " x" << std::dec << static_cast<unsigned>(decoded.rd)
             << ",0x" << std::hex << decoded.immediate; break;
    case Operation::Jal:
      output << " x" << std::dec << static_cast<unsigned>(decoded.rd) << ","
             << std::bit_cast<std::int32_t>(decoded.immediate); break;
    case Operation::Jalr: case Operation::Lb: case Operation::Lh:
    case Operation::Lw: case Operation::Lbu: case Operation::Lhu:
      output << " x" << std::dec << static_cast<unsigned>(decoded.rd) << ","
             << std::bit_cast<std::int32_t>(decoded.immediate) << "(x"
             << static_cast<unsigned>(decoded.rs1) << ")"; break;
    case Operation::Sb: case Operation::Sh: case Operation::Sw:
      output << " x" << static_cast<unsigned>(decoded.rs2) << ","
             << std::bit_cast<std::int32_t>(decoded.immediate) << "(x"
             << static_cast<unsigned>(decoded.rs1) << ")"; break;
    case Operation::Beq: case Operation::Bne: case Operation::Blt:
    case Operation::Bge: case Operation::Bltu: case Operation::Bgeu:
      output << " x" << static_cast<unsigned>(decoded.rs1) << ",x"
             << static_cast<unsigned>(decoded.rs2) << ","
             << std::bit_cast<std::int32_t>(decoded.immediate); break;
    case Operation::Addi: case Operation::Slti: case Operation::Sltiu:
    case Operation::Xori: case Operation::Ori: case Operation::Andi:
    case Operation::Slli: case Operation::Srli: case Operation::Srai:
      output << " x" << std::dec << static_cast<unsigned>(decoded.rd) << ",x"
             << static_cast<unsigned>(decoded.rs1) << ",";
      if (decoded.operation == Operation::Slli || decoded.operation == Operation::Srli ||
          decoded.operation == Operation::Srai) output << decoded.immediate;
      else output << std::bit_cast<std::int32_t>(decoded.immediate);
      break;
    default:
      output << " x" << std::dec << static_cast<unsigned>(decoded.rd) << ",x"
             << static_cast<unsigned>(decoded.rs1) << ",x"
             << static_cast<unsigned>(decoded.rs2); break;
  }
  return output.str();
}

}  // namespace proofcore::reference
