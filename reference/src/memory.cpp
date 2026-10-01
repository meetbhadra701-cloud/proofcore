#include "memory.hpp"

namespace proofcore::reference {

std::uint8_t Memory::read8(const std::uint32_t address) const {
  const auto found = bytes_.find(address);
  return found == bytes_.end() ? 0U : found->second;
}

std::uint16_t Memory::read16(const std::uint32_t address) const {
  const std::uint32_t low = read8(address);
  const std::uint32_t high = read8(address + 1U);
  return static_cast<std::uint16_t>(low | (high << 8U));
}

std::uint32_t Memory::read32(const std::uint32_t address) const {
  std::uint32_t value = 0U;
  for (std::uint32_t lane = 0U; lane < 4U; ++lane) {
    value |= static_cast<std::uint32_t>(read8(address + lane)) << (8U * lane);
  }
  return value;
}

void Memory::write8(const std::uint32_t address, const std::uint8_t value) {
  bytes_[address] = value;
}

void Memory::write16(const std::uint32_t address, const std::uint16_t value) {
  write8(address, static_cast<std::uint8_t>(value & 0xffU));
  write8(address + 1U, static_cast<std::uint8_t>((value >> 8U) & 0xffU));
}

void Memory::write32(const std::uint32_t address, const std::uint32_t value) {
  for (std::uint32_t lane = 0U; lane < 4U; ++lane) {
    write8(address + lane, static_cast<std::uint8_t>((value >> (8U * lane)) & 0xffU));
  }
}

void Memory::write_masked_word(const std::uint32_t aligned_address,
                               const std::uint8_t mask,
                               const std::uint32_t lane_data) {
  for (std::uint32_t lane = 0U; lane < 4U; ++lane) {
    if (((mask >> lane) & 1U) != 0U) {
      write8(aligned_address + lane,
             static_cast<std::uint8_t>((lane_data >> (8U * lane)) & 0xffU));
    }
  }
}

Memory make_initial_memory() {
  Memory memory;
  // A deterministic nonzero region catches accidental assumptions that all
  // bytes begin as zero while staying identical for model and RTL copies.
  for (std::uint32_t offset = 0U; offset < 256U; ++offset) {
    const auto value = static_cast<std::uint8_t>((offset * 37U + 11U) & 0xffU);
    memory.write8(0x1000U + offset, value);
  }
  return memory;
}

}  // namespace proofcore::reference
