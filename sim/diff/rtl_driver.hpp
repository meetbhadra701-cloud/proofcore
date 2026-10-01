#pragma once

#include "case_file.hpp"
#include "comparator.hpp"
#include "../../reference/include/memory.hpp"
#include "../../reference/include/retire_event.hpp"
#include "Vcore.h"

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace proofcore::diff {

struct RunResult {
  std::uint64_t cycles{};
  std::uint64_t retirements{};
  bool success{};
  std::string status;
  std::string message;
  std::unique_ptr<Mismatch> mismatch;
};

class RtlDriver {
 public:
  RtlDriver();
  ~RtlDriver();
  RtlDriver(const RtlDriver&) = delete;
  RtlDriver& operator=(const RtlDriver&) = delete;
  [[nodiscard]] RunResult run(const ProgramCase& program, bool corrupt_observed,
                              std::uint64_t corrupt_retirement);
  [[nodiscard]] const std::vector<std::uint32_t>& instruction_words() const noexcept {
    return instruction_words_;
  }

 private:
  void reset();
  [[nodiscard]] std::uint32_t instruction_at(std::uint32_t address) const;
  Vcore dut_;
  std::vector<std::uint8_t> instruction_bytes_;
  std::vector<std::uint32_t> instruction_words_;
  proofcore::reference::Memory rtl_memory_;
};

}  // namespace proofcore::diff
