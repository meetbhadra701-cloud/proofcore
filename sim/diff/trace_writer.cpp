#include "trace_writer.hpp"

#include "comparator.hpp"
#include "../../reference/include/rv32i_decode.hpp"
#include "../../reference/include/memory.hpp"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>

namespace proofcore::diff {
namespace {
std::string hex32(const std::uint32_t value) {
  std::ostringstream output;
  output << "0x" << std::hex << std::setw(8) << std::setfill('0') << value;
  return output.str();
}
}  // namespace

void write_failure_artifact(const std::filesystem::path& artifact_root,
                            const ProgramCase& program, const Mismatch& mismatch) {
  std::ostringstream run_id;
  run_id << "seed-" << program.seed << "-cycle-" << mismatch.cycle << "-ret-"
         << mismatch.retirement_index;
  std::filesystem::path directory = artifact_root / run_id.str();
  for (unsigned suffix = 1U; std::filesystem::exists(directory); ++suffix)
    directory = artifact_root / (run_id.str() + "-" + std::to_string(suffix));
  std::filesystem::create_directories(directory);

  {
    std::ofstream output(directory / "failure.json");
    output << "{\n  \"seed\":" << mismatch.seed << ",\n  \"cycle\":" << mismatch.cycle
           << ",\n  \"retirement_index\":" << mismatch.retirement_index
           << ",\n  \"test_only_comparator_corruption\":"
           << (mismatch.test_corruption_injected ? "true" : "false")
           << ",\n  \"pc_before\":\"" << hex32(mismatch.pc_before) << "\",\n  \"expected\":"
           << event_json(mismatch.expected) << ",\n  \"observed\":" << event_json(mismatch.observed)
           << ",\n  \"mismatching_fields\":[";
    for (std::size_t index = 0; index < mismatch.fields.size(); ++index) {
      if (index != 0U) output << ',';
      output << '"' << mismatch.fields[index] << '"';
    }
    output << "],\n  \"register_state_before\":[";
    for (std::size_t index = 0; index < mismatch.state_before.registers.size(); ++index) {
      if (index != 0U) output << ',';
      output << '"' << hex32(mismatch.state_before.registers[index]) << '"';
    }
    auto initial_memory = proofcore::reference::make_initial_memory();
    for (const auto& [address, value] : program.initial_bytes)
      initial_memory.write8(address, value);
    std::vector<std::pair<std::uint32_t, std::uint8_t>> initial_bytes(
        initial_memory.bytes().begin(), initial_memory.bytes().end());
    std::sort(initial_bytes.begin(), initial_bytes.end());
    output << "],\n  \"initial_memory\":[";
    for (std::size_t index = 0; index < initial_bytes.size(); ++index) {
      if (index != 0U) output << ',';
      output << "{\"address\":\"" << hex32(initial_bytes[index].first)
             << "\",\"byte\":" << static_cast<unsigned>(initial_bytes[index].second) << '}';
    }
    output << "],\n  \"recent_retirements\":[";
    for (std::size_t index = 0; index < mismatch.recent.size(); ++index) {
      if (index != 0U) output << ',';
      output << event_json(mismatch.recent[index]);
    }
    output << "],\n  \"memory_differences\":[";
    for (std::size_t index = 0; index < mismatch.memory_differences.size(); ++index) {
      if (index != 0U) output << ',';
      const auto& difference = mismatch.memory_differences[index];
      output << "{\"address\":\"" << hex32(difference.address)
             << "\",\"expected_byte\":" << static_cast<unsigned>(difference.expected)
             << ",\"observed_byte\":" << static_cast<unsigned>(difference.observed) << '}';
    }
    output << "]\n}\n";
  }
  {
    std::ofstream output(directory / "failure.md");
    output << "# Differential failure\n\nSeed: `" << mismatch.seed << "`\n\n"
           << "RTL cycle: " << mismatch.cycle << "; retirement index: "
           << mismatch.retirement_index << ".\n\nMismatching fields: ";
    for (std::size_t index = 0; index < mismatch.fields.size(); ++index) {
      if (index != 0U) output << ", ";
      output << '`' << mismatch.fields[index] << '`';
    }
    output << "\n\nExpected: `" << event_json(mismatch.expected) << "`\n\nObserved: `"
           << event_json(mismatch.observed) << "`\n\nReplay with `build/diff_obj/Vcore --case-dir "
           << directory.string();
    if (mismatch.test_corruption_injected)
      output << " --selftest-corrupt-retire " << mismatch.retirement_index;
    output << "`. The artifact reproduces the observed mismatch; it does not identify root cause.\n";
  }
  {
    std::ofstream output(directory / "seed.txt");
    output << program.seed << '\n';
  }
  {
    std::ofstream output(directory / "program.case");
    output << "seed " << program.seed << "\ntier " << program.tier << "\nstop_pc "
           << std::hex << program.stop_pc << "\ninstructions\n";
    for (const std::uint32_t word : program.words)
      output << std::setw(8) << std::setfill('0') << word << '\n';
    const auto complete_initial = proofcore::reference::make_initial_memory();
    std::vector<std::pair<std::uint32_t, std::uint8_t>> case_memory(
        complete_initial.bytes().begin(), complete_initial.bytes().end());
    for (const auto& [address, value] : program.initial_bytes) {
      const auto found = std::find_if(case_memory.begin(), case_memory.end(),
          [address](const auto& entry) { return entry.first == address; });
      if (found == case_memory.end()) case_memory.emplace_back(address, value);
      else found->second = value;
    }
    std::sort(case_memory.begin(), case_memory.end());
    output << "memory\n";
    for (const auto& [address, value] : case_memory)
      output << std::hex << address << ' ' << static_cast<unsigned>(value) << '\n';
  }
  {
    std::ofstream output(directory / "memory.hex");
    const auto complete_initial = proofcore::reference::make_initial_memory();
    std::vector<std::pair<std::uint32_t, std::uint8_t>> memory_bytes(
        complete_initial.bytes().begin(), complete_initial.bytes().end());
    for (const auto& [address, value] : program.initial_bytes) {
      const auto found = std::find_if(memory_bytes.begin(), memory_bytes.end(),
          [address](const auto& entry) { return entry.first == address; });
      if (found == memory_bytes.end()) memory_bytes.emplace_back(address, value);
      else found->second = value;
    }
    std::sort(memory_bytes.begin(), memory_bytes.end());
    for (const auto& [address, value] : memory_bytes)
      output << std::hex << std::setw(8) << std::setfill('0') << address << ' '
             << std::setw(2) << static_cast<unsigned>(value) << '\n';
  }
  {
    std::ofstream binary(directory / "program.bin", std::ios::binary);
    for (const std::uint32_t word : program.words) {
      for (unsigned lane = 0U; lane < 4U; ++lane)
        binary.put(static_cast<char>((word >> (8U * lane)) & 0xffU));
    }
  }
  {
    std::ofstream hex(directory / "program.hex");
    std::ofstream assembly(directory / "program.s");
    for (std::size_t index = 0; index < program.words.size(); ++index) {
      hex << std::hex << std::setw(8) << std::setfill('0') << program.words[index] << '\n';
      assembly << proofcore::reference::disassemble(static_cast<std::uint32_t>(index * 4U),
                                                     program.words[index]) << '\n';
    }
  }
  {
    std::ofstream output(directory / "retire-trace.jsonl");
    for (const auto& event : mismatch.recent) output << event_json(event) << '\n';
  }
}


}  // namespace proofcore::diff
