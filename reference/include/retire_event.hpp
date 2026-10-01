#pragma once

#include <cstdint>

namespace proofcore::reference {

struct RetireEvent {
  bool valid{};
  std::uint32_t pc{};
  std::uint32_t instruction{};
  std::uint8_t rd{};
  bool rd_write{};
  std::uint32_t rd_value{};
  std::uint32_t mem_addr{};
  std::uint8_t mem_rmask{};
  std::uint8_t mem_wmask{};
  std::uint32_t mem_wdata{};
  std::uint32_t next_pc{};
};

}  // namespace proofcore::reference
