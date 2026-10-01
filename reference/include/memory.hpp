#pragma once

#include <array>
#include <cstdint>
#include <unordered_map>

namespace proofcore::reference {

// Sparse little-endian byte memory. Missing bytes read as zero.
class Memory {
 public:
  [[nodiscard]] std::uint8_t read8(std::uint32_t address) const;
  [[nodiscard]] std::uint16_t read16(std::uint32_t address) const;
  [[nodiscard]] std::uint32_t read32(std::uint32_t address) const;
  void write8(std::uint32_t address, std::uint8_t value);
  void write16(std::uint32_t address, std::uint16_t value);
  void write32(std::uint32_t address, std::uint32_t value);
  void write_masked_word(std::uint32_t aligned_address, std::uint8_t mask,
                         std::uint32_t lane_data);
  [[nodiscard]] const std::unordered_map<std::uint32_t, std::uint8_t>& bytes() const noexcept {
    return bytes_;
  }

 private:
  std::unordered_map<std::uint32_t, std::uint8_t> bytes_;
};

[[nodiscard]] Memory make_initial_memory();

}  // namespace proofcore::reference
