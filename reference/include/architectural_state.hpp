#pragma once

#include <array>
#include <cstdint>

namespace proofcore::reference {

struct ArchitecturalState {
  std::array<std::uint32_t, 32> registers{};
  std::uint32_t pc{};
  std::uint64_t retirement_count{};

  void enforce_x0() noexcept { registers[0] = 0U; }
};

}  // namespace proofcore::reference
