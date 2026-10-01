#include "rv32i_decode.hpp"
#include "rv32i_model.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

using proofcore::reference::Memory;
using proofcore::reference::Operation;
using proofcore::reference::Rv32iModel;

namespace {

void check(const bool condition, const std::string& message) {
  if (!condition) {
    std::cerr << "FAIL: " << message << '\n';
    std::exit(1);
  }
}

std::uint32_t enc_i(const std::uint32_t opcode, const unsigned rd, const unsigned funct3,
                    const unsigned rs1, const int immediate) {
  return ((static_cast<std::uint32_t>(immediate) & 0xfffU) << 20U) |
         (rs1 << 15U) | (funct3 << 12U) | (rd << 7U) | opcode;
}

std::uint32_t enc_r(const unsigned rd, const unsigned funct3, const unsigned rs1,
                    const unsigned rs2, const unsigned funct7 = 0U) {
  return (funct7 << 25U) | (rs2 << 20U) | (rs1 << 15U) | (funct3 << 12U) |
         (rd << 7U) | 0x33U;
}

std::uint32_t enc_s(const unsigned funct3, const unsigned rs1, const unsigned rs2,
                    const int immediate) {
  const auto imm = static_cast<std::uint32_t>(immediate) & 0xfffU;
  return ((imm >> 5U) << 25U) | (rs2 << 20U) | (rs1 << 15U) | (funct3 << 12U) |
         ((imm & 0x1fU) << 7U) | 0x23U;
}

std::uint32_t enc_b(const unsigned funct3, const unsigned rs1, const unsigned rs2,
                    const int immediate) {
  const auto imm = static_cast<std::uint32_t>(immediate) & 0x1fffU;
  return (((imm >> 12U) & 1U) << 31U) | (((imm >> 5U) & 0x3fU) << 25U) |
         (rs2 << 20U) | (rs1 << 15U) | (funct3 << 12U) |
         (((imm >> 1U) & 0xfU) << 8U) | (((imm >> 11U) & 1U) << 7U) | 0x63U;
}

std::uint32_t enc_j(const unsigned rd, const int immediate) {
  const auto imm = static_cast<std::uint32_t>(immediate) & 0x1fffffU;
  return (((imm >> 20U) & 1U) << 31U) | (((imm >> 1U) & 0x3ffU) << 21U) |
         (((imm >> 11U) & 1U) << 20U) | (((imm >> 12U) & 0xffU) << 12U) |
         (rd << 7U) | 0x6fU;
}

std::vector<std::uint8_t> image(const std::vector<std::uint32_t>& words) {
  std::vector<std::uint8_t> bytes(words.size() * 4U);
  for (std::size_t index = 0; index < words.size(); ++index) {
    for (unsigned lane = 0; lane < 4U; ++lane)
      bytes[index * 4U + lane] = static_cast<std::uint8_t>((words[index] >> (8U * lane)) & 0xffU);
  }
  return bytes;
}

void test_decode_and_arithmetic() {
  const std::vector<std::uint32_t> program{
      0x800000b7U,                  // lui x1,0x80000000
      enc_i(0x13U, 2U, 0U, 0U, -1),  // addi x2,x0,-1
      enc_i(0x13U, 3U, 0U, 0U, 1),
      enc_r(4U, 2U, 2U, 3U),         // slt x4,x2,x3 => 1
      enc_r(5U, 3U, 2U, 3U),         // sltu x5,x2,x3 => 0
      enc_r(6U, 5U, 1U, 3U, 0x20U),  // sra x6,x1,x3
      enc_r(7U, 5U, 1U, 3U),         // srl x7,x1,x3
      enc_i(0x13U, 8U, 3U, 2U, 1),   // sltiu x8,x2,1 => 0 (imm is 1)
      enc_i(0x13U, 9U, 3U, 3U, -1),  // sltiu x9,x3,-1 => 1
      enc_i(0x13U, 10U, 5U, 1U, 1),  // srli x10,x1,1
      enc_i(0x13U, 11U, 5U, 1U, 0x401), // srai x11,x1,1
      enc_i(0x13U, 12U, 1U, 3U, 31), // slli x12,x3,31
      enc_i(0x13U, 13U, 4U, 2U, 0x55),
      enc_i(0x13U, 14U, 6U, 2U, 0x55),
      enc_i(0x13U, 15U, 7U, 2U, 0x55),
      enc_r(16U, 0U, 1U, 3U), enc_r(17U, 0U, 1U, 3U, 0x20U),
      enc_r(18U, 1U, 3U, 3U), enc_r(19U, 4U, 2U, 3U),
      enc_r(20U, 6U, 2U, 3U), enc_r(21U, 7U, 2U, 3U),
  };
  Rv32iModel model(image(program), Memory{});
  for (std::size_t index = 0; index < program.size(); ++index) {
    const auto event = model.step();
    check(event.valid && event.pc == index * 4U, "sequential retirement PC");
  }
  const auto& registers = model.state().registers;
  check(registers[0] == 0U, "x0 stays zero");
  check(registers[1] == 0x80000000U && registers[2] == 0xffffffffU, "boundary constants");
  check(registers[4] == 1U && registers[5] == 0U, "signed and unsigned comparisons disagree");
  check(registers[6] == 0xc0000000U && registers[7] == 0x40000000U, "SRA versus SRL");
  check(registers[8] == 0U && registers[9] == 1U, "SLTIU sign-extends its immediate");
  check(registers[10] == 0x40000000U && registers[11] == 0xc0000000U, "immediate shifts");
  check(registers[12] == 0x80000000U, "SLLI shift 31");
  check(registers[13] == (0xffffffffU ^ 0x55U), "XORI");
  check(registers[14] == (0xffffffffU | 0x55U), "ORI");
  check(registers[15] == (0xffffffffU & 0x55U), "ANDI");
  check(registers[16] == 0x80000001U && registers[17] == 0x7fffffffU, "ADD and SUB wrap");
  check(registers[18] == 2U, "SLL");
  check(registers[19] == 0xfffffffeU && registers[20] == 0xffffffffU && registers[21] == 1U,
        "XOR, OR, AND");
}

void test_memory_and_x0() {
  const std::vector<std::uint32_t> program{
      0x000010b7U,                  // lui x1,0x1000
      enc_i(0x13U, 2U, 0U, 0U, -128),
      enc_s(0U, 1U, 2U, 1),         // sb x2,1(x1)
      enc_i(0x03U, 3U, 0U, 1U, 1),  // lb x3,1(x1)
      enc_i(0x03U, 4U, 4U, 1U, 1),  // lbu x4,1(x1)
      enc_i(0x13U, 5U, 0U, 0U, 0x123),
      enc_s(1U, 1U, 5U, 2),         // sh x5,2(x1)
      enc_i(0x03U, 6U, 1U, 1U, 2),  // lh
      enc_i(0x03U, 7U, 5U, 1U, 2),  // lhu
      enc_s(2U, 1U, 5U, 4),         // sw
      enc_i(0x03U, 8U, 2U, 1U, 4),  // lw
      enc_i(0x13U, 0U, 0U, 0U, 55), // attempted x0 write
  };
  Rv32iModel model(image(program), proofcore::reference::make_initial_memory());
  std::vector<proofcore::reference::RetireEvent> events;
  for (std::size_t index = 0; index < program.size(); ++index) events.push_back(model.step());
  check(events[2].mem_addr == 0x1001U && events[2].mem_wmask == 0x2U && events[2].mem_wdata == 0x8000U,
        "SB lane event is architectural effective address and lane-positioned data");
  check(model.state().registers[3] == 0xffffff80U && model.state().registers[4] == 0x80U,
        "LB sign extension and LBU zero extension");
  check(model.state().registers[6] == 0x123U && model.state().registers[7] == 0x123U,
        "LH and LHU");
  check(model.state().registers[8] == 0x123U, "LW");
  check(!events.back().rd_write && events.back().rd == 0U && model.state().registers[0] == 0U,
        "writes to x0 disappear");
  check(model.memory().read8(0x1001U) == 0x80U && model.memory().read32(0x1004U) == 0x123U,
        "store byte/word update little-endian memory");
}

void test_control_and_subset_policy() {
  // Taken branch skips the wrong-path write. JALR clears bit zero. At the
  // destination a misaligned JAL is suppressed per the frozen M1 subset.
  const std::vector<std::uint32_t> program{
      enc_i(0x13U, 1U, 0U, 0U, 1), enc_b(0U, 1U, 1U, 8),
      enc_i(0x13U, 2U, 0U, 0U, 99), enc_i(0x13U, 2U, 0U, 0U, 7),
      enc_i(0x13U, 3U, 0U, 0U, 25), enc_i(0x67U, 4U, 0U, 3U, 0),
      0x0000006fU,                  // jal x0,0 at address 24
  };
  Rv32iModel model(image(program), Memory{});
  for (unsigned index = 0; index < 6U; ++index) (void)model.step();
  check(model.state().registers[2] == 7U, "taken branch skips wrong-path write");
  check(model.state().pc == 24U, "JALR bit zero cleared, destination aligned");
  const auto loop = model.step();
  check(loop.next_pc == 24U && model.state().pc == 24U, "JAL PC-relative target");
  check(model.state().registers[4] == 24U, "JALR link is the sequential PC");

  const std::vector<std::uint32_t> misaligned{
      enc_i(0x13U, 1U, 0U, 0U, 0x102), enc_i(0x67U, 2U, 0U, 1U, 0),
  };
  Rv32iModel bad_target(image(misaligned), Memory{});
  (void)bad_target.step();
  const auto suppressed = bad_target.step();
  check(suppressed.next_pc == 8U && !suppressed.rd_write,
        "misaligned JALR target is inert sequential no-op");

  const unsigned branch_funct3[6] = {0U, 1U, 4U, 5U, 6U, 7U};
  const bool branch_taken[6] = {false, true, true, false, false, true};
  for (std::size_t index = 0; index < 6U; ++index) {
    const std::vector<std::uint32_t> branch_program{
        enc_i(0x13U, 1U, 0U, 0U, -1), enc_i(0x13U, 2U, 0U, 0U, 1),
        enc_b(branch_funct3[index], 1U, 2U, 8), 0x00000013U, 0x00000013U};
    Rv32iModel branch_model(image(branch_program), Memory{});
    (void)branch_model.step();
    (void)branch_model.step();
    const auto branch_event = branch_model.step();
    check(branch_event.next_pc == (branch_taken[index] ? 16U : 12U),
          "each signed/unsigned branch predicate");
  }

  const std::vector<std::uint32_t> links{
      enc_j(4U, 8), 0x00000013U, (1U << 12U) | (5U << 7U) | 0x17U,
      enc_i(0x13U, 6U, 0U, 0U, 9)};
  Rv32iModel link_model(image(links), Memory{});
  const auto jal = link_model.step();
  check(jal.next_pc == 8U && jal.rd_write && jal.rd == 4U && jal.rd_value == 4U,
        "JAL target, destination, and PC+4 link");
  const auto auipc = link_model.step();
  check(auipc.pc == 8U && link_model.state().registers[5] == 0x1008U,
        "AUIPC uses the address of its own instruction");
}

void test_immediate_bit_layouts() {
  const std::uint32_t b_negative = enc_b(0U, 0U, 0U, -16);
  const std::uint32_t j_negative = enc_j(1U, -2048);
  check(proofcore::reference::decode_immediate(b_negative, 'B') == 0xfffffff0U,
        "negative B immediate split fields");
  check(proofcore::reference::decode_immediate(j_negative, 'J') == 0xfffff800U,
        "negative J immediate split fields");
  check(proofcore::reference::decode_immediate(enc_i(0x13U, 1U, 0U, 0U, -2048), 'I') == 0xfffff800U,
        "negative I immediate");
  check(proofcore::reference::decode_instruction(0xffffffffU).operation == Operation::Illegal,
        "unsupported opcode remains inert");
}

}  // namespace

int main() {
  test_decode_and_arithmetic();
  test_memory_and_x0();
  test_control_and_subset_policy();
  test_immediate_bit_layouts();
  std::cout << "reference tests: PASS (4 suites)\n";
  return 0;
}
