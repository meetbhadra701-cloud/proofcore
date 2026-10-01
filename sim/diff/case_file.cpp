#include "case_file.hpp"

#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <string>

namespace proofcore::diff {

ProgramCase load_case(const std::filesystem::path& path) {
  std::ifstream input(path);
  if (!input) throw std::runtime_error("cannot open case file: " + path.string());
  ProgramCase result;
  result.source_path = path;
  std::string label;
  std::string value;
  if (!(input >> label >> result.seed) || label != "seed")
    throw std::runtime_error("case must start with: seed <decimal>");
  if (!(input >> label >> result.tier) || label != "tier")
    throw std::runtime_error("case must contain: tier <label>");
  input >> std::hex;
  if (!(input >> label >> result.stop_pc) || label != "stop_pc")
    throw std::runtime_error("case must contain: stop_pc <hex>");
  if (!(input >> label) || label != "instructions")
    throw std::runtime_error("case must contain an instructions section");
  std::string token;
  bool reading_memory = false;
  while (input >> token) {
    if (token == "memory") {
      reading_memory = true;
      continue;
    }
    if (!reading_memory) {
      result.words.push_back(static_cast<std::uint32_t>(std::stoul(token, nullptr, 16)));
    } else {
      const std::uint32_t address = static_cast<std::uint32_t>(std::stoul(token, nullptr, 16));
      std::string byte_token;
      if (!(input >> byte_token)) throw std::runtime_error("memory entry requires address and byte");
      const auto byte = static_cast<std::uint8_t>(std::stoul(byte_token, nullptr, 16));
      result.initial_bytes.emplace_back(address, byte);
    }
  }
  if (result.words.empty()) throw std::runtime_error("case has no instructions");
  if ((result.stop_pc & 3U) != 0U) throw std::runtime_error("stop_pc must be word aligned");
  return result;
}

std::vector<std::uint8_t> image_bytes(const ProgramCase& program) {
  std::vector<std::uint8_t> bytes(program.words.size() * 4U, 0U);
  for (std::size_t index = 0; index < program.words.size(); ++index) {
    for (unsigned lane = 0U; lane < 4U; ++lane) {
      bytes[index * 4U + lane] = static_cast<std::uint8_t>(
          (program.words[index] >> (8U * lane)) & 0xffU);
    }
  }
  return bytes;
}

}  // namespace proofcore::diff
