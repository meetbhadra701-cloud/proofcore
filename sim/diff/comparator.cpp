#include "comparator.hpp"

#include <iomanip>
#include <sstream>

namespace proofcore::diff {
namespace {

template <typename T>
void compare_field(std::vector<std::string>& differences, const char* name,
                   const T expected, const T observed) {
  if (expected != observed) differences.emplace_back(name);
}

std::string hex32(const std::uint32_t value) {
  std::ostringstream output;
  output << "0x" << std::hex << std::setw(8) << std::setfill('0') << value;
  return output.str();
}

}  // namespace

std::vector<std::string> compare_events(
    const proofcore::reference::RetireEvent& expected,
    const proofcore::reference::RetireEvent& observed) {
  std::vector<std::string> differences;
  compare_field(differences, "valid", expected.valid, observed.valid);
  compare_field(differences, "pc", expected.pc, observed.pc);
  compare_field(differences, "instruction", expected.instruction, observed.instruction);
  compare_field(differences, "rd_write", expected.rd_write, observed.rd_write);
  compare_field(differences, "rd", expected.rd, observed.rd);
  if (expected.rd_write || observed.rd_write)
    compare_field(differences, "rd_value", expected.rd_value, observed.rd_value);
  compare_field(differences, "next_pc", expected.next_pc, observed.next_pc);
  compare_field(differences, "mem_rmask", expected.mem_rmask, observed.mem_rmask);
  compare_field(differences, "mem_wmask", expected.mem_wmask, observed.mem_wmask);
  const bool expected_memory = expected.mem_rmask != 0U || expected.mem_wmask != 0U;
  const bool observed_memory = observed.mem_rmask != 0U || observed.mem_wmask != 0U;
  if (expected_memory || observed_memory)
    compare_field(differences, "mem_addr", expected.mem_addr, observed.mem_addr);
  if (expected.mem_wmask != 0U || observed.mem_wmask != 0U)
    compare_field(differences, "mem_wdata", expected.mem_wdata, observed.mem_wdata);
  return differences;
}

std::string event_json(const proofcore::reference::RetireEvent& event) {
  std::ostringstream output;
  output << "{\"valid\":" << (event.valid ? "true" : "false")
         << ",\"pc\":\"" << hex32(event.pc) << "\",\"instruction\":\""
         << hex32(event.instruction) << "\",\"rd\":" << static_cast<unsigned>(event.rd)
         << ",\"rd_write\":" << (event.rd_write ? "true" : "false")
         << ",\"rd_value\":\"" << hex32(event.rd_value) << "\",\"mem_addr\":\""
         << hex32(event.mem_addr) << "\",\"mem_rmask\":" << static_cast<unsigned>(event.mem_rmask)
         << ",\"mem_wmask\":" << static_cast<unsigned>(event.mem_wmask)
         << ",\"mem_wdata\":\"" << hex32(event.mem_wdata) << "\",\"next_pc\":\""
         << hex32(event.next_pc) << "\"}";
  return output.str();
}

}  // namespace proofcore::diff
