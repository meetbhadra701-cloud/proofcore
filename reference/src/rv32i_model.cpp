#include "rv32i_model.hpp"

#include "rv32i_decode.hpp"

#include <bit>
#include <cstdint>
#include <utility>

namespace proofcore::reference {
namespace {

constexpr bool is_load(const Operation op) noexcept {
  return op == Operation::Lb || op == Operation::Lh || op == Operation::Lw ||
         op == Operation::Lbu || op == Operation::Lhu;
}

constexpr bool is_store(const Operation op) noexcept {
  return op == Operation::Sb || op == Operation::Sh || op == Operation::Sw;
}

constexpr bool is_branch(const Operation op) noexcept {
  return op == Operation::Beq || op == Operation::Bne || op == Operation::Blt ||
         op == Operation::Bge || op == Operation::Bltu || op == Operation::Bgeu;
}

constexpr bool reads_rs1(const Operation op) noexcept {
  return op == Operation::Add || op == Operation::Sub || op == Operation::Sll ||
         op == Operation::Slt || op == Operation::Sltu || op == Operation::Xor ||
         op == Operation::Srl || op == Operation::Sra || op == Operation::Or ||
         op == Operation::And || op == Operation::Addi || op == Operation::Slti ||
         op == Operation::Sltiu || op == Operation::Xori || op == Operation::Ori ||
         op == Operation::Andi || op == Operation::Slli || op == Operation::Srli ||
         op == Operation::Srai || is_load(op) || is_store(op) || is_branch(op) ||
         op == Operation::Jalr;
}

constexpr bool reads_rs2(const Operation op) noexcept {
  return op == Operation::Add || op == Operation::Sub || op == Operation::Sll ||
         op == Operation::Slt || op == Operation::Sltu || op == Operation::Xor ||
         op == Operation::Srl || op == Operation::Sra || op == Operation::Or ||
         op == Operation::And || is_store(op) || is_branch(op);
}

constexpr std::uint8_t lane_mask(const std::uint32_t address,
                                 const unsigned byte_count) noexcept {
  const std::uint32_t low_mask = (1U << byte_count) - 1U;
  return static_cast<std::uint8_t>(low_mask << (address & 3U));
}

constexpr bool misaligned(const Operation op, const std::uint32_t address) noexcept {
  if (op == Operation::Lh || op == Operation::Lhu || op == Operation::Sh)
    return (address & 1U) != 0U;
  if (op == Operation::Lw || op == Operation::Sw)
    return (address & 3U) != 0U;
  return false;
}

constexpr std::uint32_t signed_less(const std::uint32_t lhs,
                                    const std::uint32_t rhs) noexcept {
  return std::bit_cast<std::int32_t>(lhs) < std::bit_cast<std::int32_t>(rhs) ? 1U : 0U;
}

}  // namespace

Rv32iModel::Rv32iModel(std::vector<std::uint8_t> instruction_bytes,
                       Memory initial_memory)
    : instruction_bytes_(std::move(instruction_bytes)), memory_(std::move(initial_memory)) {}

std::uint32_t Rv32iModel::fetch(const std::uint32_t address) const noexcept {
  std::uint32_t instruction = 0U;
  for (std::uint32_t lane = 0U; lane < 4U; ++lane) {
    const std::uint64_t byte_address = static_cast<std::uint64_t>(address) + lane;
    if (byte_address < instruction_bytes_.size()) {
      instruction |= static_cast<std::uint32_t>(instruction_bytes_[static_cast<std::size_t>(byte_address)])
                     << (8U * lane);
    }
  }
  return instruction;
}

RetireEvent Rv32iModel::step() {
  const std::uint32_t pc = state_.pc;
  const std::uint32_t instruction = fetch(pc);
  const DecodedInstruction decoded = decode_instruction(instruction);
  const Operation op = decoded.operation;
  const std::uint32_t lhs = state_.registers[decoded.rs1];
  const std::uint32_t rhs = state_.registers[decoded.rs2];
  const std::uint32_t imm = decoded.immediate;
  RetireEvent event{};
  event.valid = true;
  event.pc = pc;
  event.instruction = instruction;
  event.next_pc = pc + 4U;
  if (reads_rs1(op)) {
    event.rs1_addr = decoded.rs1;
    event.rs1_rdata = lhs;
  }
  if (reads_rs2(op)) {
    event.rs2_addr = decoded.rs2;
    event.rs2_rdata = rhs;
  }

  bool request_register_write = false;
  std::uint32_t result = 0U;
  bool take_transfer = false;
  std::uint32_t target = event.next_pc;

  switch (op) {
    case Operation::Add: result = lhs + rhs; request_register_write = true; break;
    case Operation::Sub: result = lhs - rhs; request_register_write = true; break;
    case Operation::Sll: result = lhs << (rhs & 31U); request_register_write = true; break;
    case Operation::Slt: result = signed_less(lhs, rhs); request_register_write = true; break;
    case Operation::Sltu: result = lhs < rhs ? 1U : 0U; request_register_write = true; break;
    case Operation::Xor: result = lhs ^ rhs; request_register_write = true; break;
    case Operation::Srl: result = lhs >> (rhs & 31U); request_register_write = true; break;
    case Operation::Sra:
      result = static_cast<std::uint32_t>(std::bit_cast<std::int32_t>(lhs) >> (rhs & 31U));
      request_register_write = true;
      break;
    case Operation::Or: result = lhs | rhs; request_register_write = true; break;
    case Operation::And: result = lhs & rhs; request_register_write = true; break;
    case Operation::Addi: result = lhs + imm; request_register_write = true; break;
    case Operation::Slti: result = signed_less(lhs, imm); request_register_write = true; break;
    case Operation::Sltiu: result = lhs < imm ? 1U : 0U; request_register_write = true; break;
    case Operation::Xori: result = lhs ^ imm; request_register_write = true; break;
    case Operation::Ori: result = lhs | imm; request_register_write = true; break;
    case Operation::Andi: result = lhs & imm; request_register_write = true; break;
    case Operation::Slli: result = lhs << (imm & 31U); request_register_write = true; break;
    case Operation::Srli: result = lhs >> (imm & 31U); request_register_write = true; break;
    case Operation::Srai:
      result = static_cast<std::uint32_t>(std::bit_cast<std::int32_t>(lhs) >> (imm & 31U));
      request_register_write = true;
      break;
    case Operation::Lui: result = imm; request_register_write = true; break;
    case Operation::Auipc: result = pc + imm; request_register_write = true; break;
    case Operation::Jal:
      target = pc + imm;
      take_transfer = true;
      result = pc + 4U;
      request_register_write = true;
      break;
    case Operation::Jalr:
      target = (lhs + imm) & 0xffff'fffeU;
      take_transfer = true;
      result = pc + 4U;
      request_register_write = true;
      break;
    case Operation::Beq: take_transfer = lhs == rhs; break;
    case Operation::Bne: take_transfer = lhs != rhs; break;
    case Operation::Blt: take_transfer = std::bit_cast<std::int32_t>(lhs) < std::bit_cast<std::int32_t>(rhs); break;
    case Operation::Bge: take_transfer = std::bit_cast<std::int32_t>(lhs) >= std::bit_cast<std::int32_t>(rhs); break;
    case Operation::Bltu: take_transfer = lhs < rhs; break;
    case Operation::Bgeu: take_transfer = lhs >= rhs; break;
    case Operation::Lb: case Operation::Lh: case Operation::Lw:
    case Operation::Lbu: case Operation::Lhu:
    case Operation::Sb: case Operation::Sh: case Operation::Sw:
    case Operation::Illegal: break;
  }

  if (is_branch(op) && take_transfer) target = pc + imm;
  if (take_transfer && (target & 3U) != 0U) {
    // M1's frozen subset suppresses a misaligned transfer as an inert no-op.
    take_transfer = false;
    request_register_write = false;
  }
  if (take_transfer) event.next_pc = target;

  if (is_load(op) || is_store(op)) {
    const std::uint32_t address = lhs + imm;
    if (!misaligned(op, address)) {
      event.mem_addr = address;
      const unsigned bytes = (op == Operation::Lb || op == Operation::Lbu || op == Operation::Sb) ? 1U :
                             (op == Operation::Lh || op == Operation::Lhu || op == Operation::Sh) ? 2U : 4U;
      const std::uint8_t mask = lane_mask(address, bytes);
      const unsigned shift = static_cast<unsigned>(address & 3U) * 8U;
      if (is_load(op)) {
        event.mem_rmask = mask;
        const std::uint32_t word = memory_.read32(address & ~3U);
        event.mem_rdata = word;
        const std::uint32_t raw = (word >> shift) & ((bytes == 4U) ? 0xffff'ffffU : ((1U << (8U * bytes)) - 1U));
        if (op == Operation::Lb) {
          const std::uint32_t byte = raw & 0xffU;
          result = (byte & 0x80U) != 0U ? (byte | 0xffff'ff00U) : byte;
        } else if (op == Operation::Lh) {
          result = (raw & 0x8000U) != 0U ? (raw | 0xffff'0000U) : raw;
        } else result = raw;
        request_register_write = true;
      } else {
        event.mem_wmask = mask;
        const std::uint32_t payload_mask = bytes == 4U ? 0xffff'ffffU : ((1U << (8U * bytes)) - 1U);
        event.mem_wdata = (rhs & payload_mask) << shift;
        memory_.write_masked_word(address & ~3U, mask, event.mem_wdata);
      }
    }
  }

  if (request_register_write && decoded.rd != 0U) {
    event.rd = decoded.rd;
    event.rd_write = true;
    event.rd_value = result;
    state_.registers[decoded.rd] = result;
  }
  state_.pc = event.next_pc;
  state_.retirement_count += 1U;
  state_.enforce_x0();
  return event;
}

}  // namespace proofcore::reference
