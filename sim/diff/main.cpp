#include "case_file.hpp"
#include "rtl_driver.hpp"
#include "trace_writer.hpp"

#include "../../reference/include/rv32i_decode.hpp"

#include <verilated.h>

#include <algorithm>
#include <charconv>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

struct Options {
  std::filesystem::path case_directory;
  std::filesystem::path artifacts{"artifacts/failures"};
  bool corrupt{};
  std::uint64_t corrupt_retirement{};
};

std::uint64_t parse_u64(const std::string& text, const char* option) {
  std::uint64_t value = 0U;
  const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
  if (error != std::errc{} || end != text.data() + text.size())
    throw std::runtime_error(std::string("invalid integer for ") + option + ": " + text);
  return value;
}

Options parse_options(const int argc, char** argv) {
  Options options;
  for (int index = 1; index < argc; ++index) {
    const std::string argument(argv[index]);
    if (argument == "--case-dir" && index + 1 < argc) options.case_directory = argv[++index];
    else if (argument == "--artifacts" && index + 1 < argc) options.artifacts = argv[++index];
    else if (argument == "--selftest-corrupt-retire" && index + 1 < argc) {
      options.corrupt = true;
      options.corrupt_retirement = parse_u64(argv[++index], "--selftest-corrupt-retire");
      if (options.corrupt_retirement == 0U)
        throw std::runtime_error("self-test retirement index is one-based and must be positive");
    } else throw std::runtime_error("usage: proofcore_diff --case-dir DIR [--artifacts DIR] [--selftest-corrupt-retire N]");
  }
  if (options.case_directory.empty()) throw std::runtime_error("--case-dir is required");
  return options;
}

}  // namespace

int main(int argc, char** argv) {
  Verilated::commandArgs(argc, argv);
  try {
    const Options options = parse_options(argc, argv);
    std::vector<std::filesystem::path> cases;
    for (const auto& entry : std::filesystem::directory_iterator(options.case_directory))
      if (entry.is_regular_file() && entry.path().extension() == ".case") cases.push_back(entry.path());
    std::sort(cases.begin(), cases.end());
    if (cases.empty()) throw std::runtime_error("case directory contains no .case files");
    if (options.corrupt && cases.size() != 1U)
      throw std::runtime_error("comparator self-test requires exactly one case file");

    proofcore::diff::RtlDriver driver;
    std::uint64_t total_retirements = 0U;
    std::uint64_t total_cycles = 0U;
    for (const auto& path : cases) {
      const auto program = proofcore::diff::load_case(path);
      const auto result = driver.run(program, options.corrupt, options.corrupt_retirement);
      total_retirements += result.retirements;
      total_cycles += result.cycles;
      if (!result.success) {
        std::cerr << result.status << ": seed=" << program.seed << " case=" << path.filename().string()
                  << " cycles=" << result.cycles << " retirements=" << result.retirements
                  << " — " << result.message << '\n';
        if (result.mismatch) {
          proofcore::diff::write_failure_artifact(options.artifacts, program, *result.mismatch);
          std::cerr << "failure artifact: " << options.artifacts.lexically_normal().string() << '\n';
        }
        return 1;
      }
      if (!options.corrupt && (cases.size() <= 10U || total_retirements % 10000U == 0U))
        std::cout << "PASS seed=" << program.seed << " tier=" << program.tier
                  << " retirements=" << result.retirements << " cycles=" << result.cycles << '\n';
    }
    if (options.corrupt) {
      std::cerr << "SELFTEST ERROR: injected corruption was not detected\n";
      return 2;
    }
    std::cout << "PASS programs=" << cases.size() << " retirements=" << total_retirements
              << " cycles=" << total_cycles << '\n';
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "SETUP_ERROR: " << error.what() << '\n';
    return 2;
  }
}
