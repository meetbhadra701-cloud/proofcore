# 12 — Important Code Walkthrough

This guide follows one comparison from initial state to failure output. Paths below point to the actual implementation. The C++ model and RTL remain separate; the harness is the bridge.

## 1. Architectural state

**Code —** `reference/include/architectural_state.hpp`, `ArchitecturalState`:

```cpp
std::array<std::uint32_t, 32> registers{};
std::uint32_t pc{};
std::uint64_t retirement_count{};
void enforce_x0() noexcept { registers[0] = 0U; }
```

**Meaning —** Value initialization gives zero initial state. `uint32_t` gives fixed 32-bit bit patterns; the array models architectural registers, not the RTL's pipeline flops. Reasserting x0 protects the ISA invariant.

**Example —** After `addi x0,x0,5`, the architectural array still has `registers[0]==0`.

**Failure mode —** If source register zero were allowed to retain 5, every later instruction using x0 could be wrong.

## 2. Decoder and immediate reconstruction

**Code —** `reference/src/rv32i_decode.cpp`, `decode_instruction()` and `decode_immediate()`:

```cpp
case 0x33U:
  switch (funct3) {
    case 0U:
      if (funct7 == 0U) set(Operation::Add);
      else if (funct7 == 0x20U) set(Operation::Sub);
      break;
```

```cpp
case 'B': {
  const std::uint32_t imm = (field(instruction,31U,31U) << 12U) |
      (field(instruction,7U,7U) << 11U) |
      (field(instruction,30U,25U) << 5U) |
      (field(instruction,11U,8U) << 1U);
  return sign_extend(imm,13U);
}
```

**Meaning —** The public opcode/funct fields map to a software operation. B-immediate bits are scattered in the encoding, then restored to a signed byte displacement with bit zero equal to zero.

**Example —** A branch displacement of -4 becomes `0xfffffffc`; adding it to PC 12 returns target PC 8.

**Failure mode —** Swapping `insn[7]` and `insn[11]` changes branch destinations, especially for negative offsets.

## 3. Instruction execution

**Code —** `reference/src/rv32i_model.cpp`, `Rv32iModel::step()`:

```cpp
const std::uint32_t pc = state_.pc;
const std::uint32_t instruction = fetch(pc);
const DecodedInstruction decoded = decode_instruction(instruction);
event.next_pc = pc + 4U;
```

**Meaning —** A step begins at the current architectural PC and assumes sequential flow. Branches and jumps replace that default only when their ISA condition says to transfer.

```cpp
case Operation::Add: result = lhs + rhs; request_register_write = true; break;
case Operation::Slt: result = signed_less(lhs, rhs); request_register_write = true; break;
case Operation::Sltu: result = lhs < rhs ? 1U : 0U; request_register_write = true; break;
```

**Example —** With `lhs=0xffffffff` and `rhs=1`, SLT is 1 but SLTU is 0.

**Failure mode —** Reusing signed comparison for SLTU changes a visible destination value.

## 4. Memory access

**Code —** `Rv32iModel::step()` and `reference/src/memory.cpp`:

```cpp
const std::uint8_t mask = lane_mask(address, bytes);
const std::uint32_t word = memory_.read32(address & ~3U);
const unsigned shift = static_cast<unsigned>(address & 3U) * 8U;
```

**Meaning —** The architectural byte address identifies width and lane. A load reads the containing aligned word and shifts the selected lane into the low bits. A store forms a mask and updates exactly those bytes.

**Example —** SB at address offset 2 has mask `0100`; byte `0xaa` is recorded as `0x00aa0000`.

**Failure mode —** Reversing byte order turns `0x44332211` into `0x11223344`; an incorrect mask overwrites neighbors.

## 5. Retirement event

**Code —** `reference/include/retire_event.hpp` and the end of `step()`:

```cpp
if (request_register_write && decoded.rd != 0U) {
  event.rd = decoded.rd;
  event.rd_write = true;
  event.rd_value = result;
  state_.registers[decoded.rd] = result;
}
state_.pc = event.next_pc;
state_.retirement_count += 1U;
```

**Meaning —** A destination update is made only for nonzero rd. Regardless of whether the instruction writes a register, a valid instruction retires and advances architectural PC.

**Failure mode —** If unsupported instructions stop retiring rather than following the frozen M1 no-op rule, the event streams lose alignment.

## 6. Verilator clock/reset driver

**Code —** `sim/diff/rtl_driver.cpp`, `reset()` and the main loop of `run()`:

```cpp
dut_.clk = 0U;
dut_.reset = 1U;
dut_.eval();
// two rising edges while reset is asserted
```

**Meaning —** M1 reset is synchronous, so the driver must provide active clock edges while reset is high. Each case starts from a clean register/pipeline state.

At each running cycle it presents instruction bytes at `imem_addr`, a little-endian data word at `dmem_addr`, observes the current retirement before its committing edge, then applies the external store pins at the rising edge.

**Failure mode —** Reset without a clock edge leaves stale valid bits; sampling only after an edge shifts event association.

## 7. Independent RTL memory adapter

**Code —** `RtlDriver::run()` constructs two `Memory` copies; the RTL copy is written from `dmem_write_mask/data`.

**Meaning —** Both copies start with equal bytes, but they never share later writes. The DUT's load value comes from the RTL copy; the model's load comes from its own copy.

**Failure mode —** One shared memory can conceal a missing store because the other side updated the shared object.

## 8. Comparator

**Code —** `sim/diff/comparator.cpp`, `compare_events()`:

```cpp
compare_field(differences, "next_pc", expected.next_pc, observed.next_pc);
compare_field(differences, "mem_rmask", expected.mem_rmask, observed.mem_rmask);
compare_field(differences, "mem_wmask", expected.mem_wmask, observed.mem_wmask);
if (expected.rd_write || observed.rd_write)
  compare_field(differences, "rd_value", expected.rd_value, observed.rd_value);
```

**Meaning —** The checker compares architectural results and the metadata that describes accesses. It ignores destination data only when neither side says a register write occurred.

**Example —** A wrong branch target immediately appears as `next_pc` disagreement.

**Failure mode —** Omitting next PC can delay a control-flow error; comparing meaningless data can create false failures.

## 9. Random program generator and seeds

**Code —** `tools/programgen/generate.py`, `Rng.next_u64()`, `generate_case()`, and `write_corpus()`:

```python
x ^= x >> 12
x ^= (x << 25) & 0xFFFFFFFFFFFFFFFF
x ^= x >> 27
return (x * 0x2545F4914F6CDD1D) & 0xFFFFFFFFFFFFFFFF
```

**Meaning —** The specified xorshift64* algorithm turns the recorded seed into repeatable choices. Templates keep operations legal and loops bounded.

**Failure mode —** Reading system entropy or forgetting a case seed would make a random mismatch hard to reproduce.

## 10. Failure artifact writer

**Code —** `sim/diff/trace_writer.cpp`, `write_failure_artifact()`:

It writes JSON field differences and pre-step registers, plus raw image, readable disassembly, memory overrides, seed, and recent retirement records.

**Example —** The self-test artifact records expected x6 value 14 and observed value 7, marked `test_only_comparator_corruption=true`.

**Failure mode —** Without the initial memory image or exact case file, replay may not take the same path.

## Final hardware perspective

The C++ switch statements are not gates. The RTL's combinational decisions become muxes, comparators, adders, and control wires; pipeline state becomes flops. The simulator executes both the generated RTL model and the software oracle on the host CPU.
