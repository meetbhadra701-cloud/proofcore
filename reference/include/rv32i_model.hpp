#pragma once

#include "architectural_state.hpp"
#include "memory.hpp"
#include "retire_event.hpp"

#include <cstdint>
#include <vector>

namespace proofcore::reference {

class Rv32iModel {
 public:
  Rv32iModel(std::vector<std::uint8_t> instruction_bytes, Memory initial_memory);

  [[nodiscard]] RetireEvent step();
  [[nodiscard]] const ArchitecturalState& state() const noexcept { return state_; }
  [[nodiscard]] const Memory& memory() const noexcept { return memory_; }
  [[nodiscard]] Memory& memory() noexcept { return memory_; }

 private:
  [[nodiscard]] std::uint32_t fetch(std::uint32_t address) const noexcept;
  std::vector<std::uint8_t> instruction_bytes_;
  Memory memory_;
  ArchitecturalState state_{};
};

}  // namespace proofcore::reference
