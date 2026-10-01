#include "rtl_driver.hpp"

#include "case_file.hpp"
#include "comparator.hpp"
#include "../../reference/include/rv32i_model.hpp"

#include <algorithm>
#include <bit>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <unordered_set>

namespace proofcore::diff {
namespace {

proofcore::reference::RetireEvent observed_event(const Vcore& dut) {
  proofcore::reference::RetireEvent event{};
  event.valid = dut.retire_valid != 0U;
  event.pc = dut.retire_pc;
  event.instruction = dut.retire_insn;
  event.rd = static_cast<std::uint8_t>(dut.retire_rd);
  event.rd_write = dut.retire_rd_write != 0U;
  event.rd_value = dut.retire_rd_value;
  event.mem_addr = dut.retire_mem_addr;
  event.mem_rmask = static_cast<std::uint8_t>(dut.retire_mem_read_mask);
  event.mem_wmask = static_cast<std::uint8_t>(dut.retire_mem_write_mask);
  event.mem_wdata = dut.retire_mem_write_data;
  event.next_pc = dut.retire_next_pc;
  return event;
}

std::string hex32(const std::uint32_t value) {
  std::ostringstream output;
  output << "0x" << std::hex << std::setw(8) << std::setfill('0') << value;
  return output.str();
}

}  // namespace

RtlDriver::RtlDriver() : dut_{} {}
RtlDriver::~RtlDriver() { dut_.final(); }

void RtlDriver::reset() {
  dut_.clk = 0U;
  dut_.reset = 1U;
  dut_.imem_rdata = 0U;
  dut_.dmem_rdata = 0U;
  dut_.eval();
  for (unsigned edge = 0; edge < 2U; ++edge) {
    dut_.clk = 1U;
    dut_.eval();
    dut_.clk = 0U;
    dut_.eval();
  }
  dut_.reset = 0U;
  dut_.eval();
}

std::uint32_t RtlDriver::instruction_at(const std::uint32_t address) const {
  std::uint32_t result = 0U;
  for (unsigned lane = 0; lane < 4U; ++lane) {
    const std::uint64_t byte_address = static_cast<std::uint64_t>(address) + lane;
    if (byte_address < instruction_bytes_.size())
      result |= static_cast<std::uint32_t>(instruction_bytes_[static_cast<std::size_t>(byte_address)])
                << (8U * lane);
  }
  return result;
}

RunResult RtlDriver::run(const ProgramCase& program, const bool corrupt_observed,
                         const std::uint64_t corrupt_retirement) {
  instruction_words_ = program.words;
  instruction_bytes_ = image_bytes(program);
  rtl_memory_ = proofcore::reference::make_initial_memory();
  auto reference_memory = proofcore::reference::make_initial_memory();
  for (const auto& [address, value] : program.initial_bytes) {
    reference_memory.write8(address, value);
    rtl_memory_.write8(address, value);
  }
  proofcore::reference::Rv32iModel model(instruction_bytes_, reference_memory);
  if (model.memory().bytes() != rtl_memory_.bytes())
    throw std::runtime_error("reference and RTL initial memory images differ");

  // Determine the exact architectural retirement stream length before running
  // RTL. This is model-only test scheduling; comparison still happens online.
  proofcore::reference::Rv32iModel expected_length(instruction_bytes_, reference_memory);
  std::uint64_t target_retirements = 0U;
  while (expected_length.state().pc != program.stop_pc && target_retirements < 100000U) {
    (void)expected_length.step();
    ++target_retirements;
  }
  if (expected_length.state().pc != program.stop_pc)
    return {0U, 0U, false, "TIMEOUT", "architectural instruction limit reached before stop_pc", nullptr};
  if (target_retirements == 0U)
    return {0U, 0U, false, "SETUP_ERROR", "program stop_pc equals initial PC", nullptr};

  reset();
  RunResult result{};
  result.status = "PASS";
  std::vector<proofcore::reference::RetireEvent> recent;
  const std::uint64_t cycle_limit = target_retirements * 12U + 64U;

  for (std::uint64_t cycle = 1U; cycle <= cycle_limit; ++cycle) {
    dut_.clk = 0U;
    dut_.eval();
    dut_.imem_rdata = instruction_at(dut_.imem_addr);
    dut_.dmem_rdata = rtl_memory_.read32(dut_.dmem_addr);
    dut_.eval();
    const std::uint32_t store_address = dut_.dmem_addr;
    const std::uint8_t store_mask = static_cast<std::uint8_t>(dut_.dmem_write_mask);
    const std::uint32_t store_data = dut_.dmem_write_data;
    result.cycles = cycle;

    if (dut_.retire_valid != 0U) {
      const proofcore::reference::ArchitecturalState state_before = model.state();
      const proofcore::reference::RetireEvent expected = model.step();
      proofcore::reference::RetireEvent observed = observed_event(dut_);
      if (corrupt_observed && result.retirements + 1U == corrupt_retirement) {
        // Mutate only this sampled comparison copy, never the RTL or model.
        if (expected.rd_write && expected.rd_value == 14U) observed.rd_value = 7U;
        else if (observed.rd_write) observed.rd_value ^= 1U;
        else observed.next_pc ^= 4U;
      }
      auto differences = compare_events(expected, observed);
      ++result.retirements;
      recent.push_back(observed);
      if (recent.size() > 12U) recent.erase(recent.begin());

      if (!differences.empty()) {
        result.success = false;
        result.status = "FAIL";
        result.message = "retirement event differs at " + hex32(observed.pc);
        result.mismatch = std::make_unique<Mismatch>();
        result.mismatch->seed = program.seed;
        result.mismatch->cycle = cycle;
        result.mismatch->retirement_index = result.retirements;
        result.mismatch->pc_before = state_before.pc;
        result.mismatch->expected = expected;
        result.mismatch->observed = observed;
        result.mismatch->fields = std::move(differences);
        result.mismatch->test_corruption_injected = corrupt_observed &&
                                                     result.retirements == corrupt_retirement;
        result.mismatch->state_before = state_before;
        result.mismatch->recent = std::move(recent);
        return result;
      }
      if (result.retirements == target_retirements) {
        if (corrupt_observed && corrupt_retirement > target_retirements) {
          result.success = false;
          result.status = "SETUP_ERROR";
          result.message = "self-test corruption index exceeds executed retirement count";
          return result;
        }
        std::unordered_set<std::uint32_t> addresses;
        for (const auto& [address, value] : model.memory().bytes()) {
          (void)value;
          addresses.insert(address);
        }
        for (const auto& [address, value] : rtl_memory_.bytes()) {
          (void)value;
          addresses.insert(address);
        }
        std::vector<std::uint32_t> ordered_addresses(addresses.begin(), addresses.end());
        std::sort(ordered_addresses.begin(), ordered_addresses.end());
        auto mismatch = std::make_unique<Mismatch>();
        for (const std::uint32_t address : ordered_addresses) {
          const std::uint8_t expected_byte = model.memory().read8(address);
          const std::uint8_t observed_byte = rtl_memory_.read8(address);
          if (expected_byte != observed_byte)
            mismatch->memory_differences.push_back({address, expected_byte, observed_byte});
        }
        if (!mismatch->memory_differences.empty()) {
          result.success = false;
          result.status = "FAIL";
          result.message = "final RTL and reference memory bytes differ";
          mismatch->seed = program.seed;
          mismatch->cycle = cycle;
          mismatch->retirement_index = result.retirements;
          mismatch->pc_before = model.state().pc;
          mismatch->fields = {"memory_state"};
          mismatch->state_before = model.state();
          mismatch->recent = recent;
          if (!recent.empty()) mismatch->expected = mismatch->observed = recent.back();
          result.mismatch = std::move(mismatch);
          return result;
        }
        // The observed retirement is sampled immediately before its committing
        // edge. Apply that edge before declaring architectural execution done.
        dut_.clk = 1U;
        dut_.eval();
        if (store_mask != 0U)
          rtl_memory_.write_masked_word(store_address, store_mask, store_data);
        result.success = true;
        return result;
      }
    }

    // The external memory model samples store pins at the same active edge as
    // the RTL pipeline. Its state is distinct from the reference memory.
    dut_.clk = 1U;
    dut_.eval();
    if (store_mask != 0U) rtl_memory_.write_masked_word(store_address, store_mask, store_data);
  }

  result.success = false;
  result.status = "TIMEOUT";
  result.message = "cycle limit reached before expected architectural stop_pc";
  result.mismatch = std::make_unique<Mismatch>();
  result.mismatch->seed = program.seed;
  result.mismatch->cycle = result.cycles;
  result.mismatch->retirement_index = result.retirements + 1U;
  result.mismatch->pc_before = model.state().pc;
  result.mismatch->state_before = model.state();
  result.mismatch->expected = model.step();
  result.mismatch->observed = {};
  result.mismatch->fields = {"retirement_timeout"};
  result.mismatch->recent = std::move(recent);
  return result;
}


}  // namespace proofcore::diff
