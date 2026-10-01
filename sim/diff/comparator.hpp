#pragma once

#include "../../reference/include/architectural_state.hpp"
#include "../../reference/include/retire_event.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace proofcore::diff {

struct Mismatch {
  std::uint64_t seed{};
  std::uint64_t cycle{};
  std::uint64_t retirement_index{};
  std::uint32_t pc_before{};
  proofcore::reference::RetireEvent expected{};
  proofcore::reference::RetireEvent observed{};
  std::vector<std::string> fields;
  bool test_corruption_injected{};
  proofcore::reference::ArchitecturalState state_before{};
  std::vector<proofcore::reference::RetireEvent> recent;
  struct MemoryByte {
    std::uint32_t address{};
    std::uint8_t expected{};
    std::uint8_t observed{};
  };
  std::vector<MemoryByte> memory_differences;
};

[[nodiscard]] std::vector<std::string> compare_events(
    const proofcore::reference::RetireEvent& expected,
    const proofcore::reference::RetireEvent& observed);
[[nodiscard]] std::string event_json(const proofcore::reference::RetireEvent& event);

}  // namespace proofcore::diff
