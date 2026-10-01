#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

namespace proofcore::diff {

struct ProgramCase {
  std::uint64_t seed{};
  std::string tier;
  std::uint32_t stop_pc{};
  std::vector<std::uint32_t> words;
  std::vector<std::pair<std::uint32_t, std::uint8_t>> initial_bytes;
  std::filesystem::path source_path;
};

[[nodiscard]] ProgramCase load_case(const std::filesystem::path& path);
[[nodiscard]] std::vector<std::uint8_t> image_bytes(const ProgramCase& program);

}  // namespace proofcore::diff
