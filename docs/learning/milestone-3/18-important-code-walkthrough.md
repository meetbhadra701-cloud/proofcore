# 18 — Important code walkthrough

This follows one instruction's observation path through the actual M3 changes.
Signal names below are from the checked-in RTL. The complete architectural
contract remains in `SPEC.md`.

## 1. Optional interface and pipeline metadata

`rtl/core.sv`, module `core`, places the RVFI ports inside `` `ifdef
PROOFCORE_RVFI ``. The ordinary M1/M2 top interface is unchanged unless that
define is present. `rtl/proofcore_pkg.sv` extends `exmem_t` and `memwb_t` only
under the same define with `uses_rs1`, `uses_rs2`, `rs1_value`, `rs2_value`, and
`mem_read_data`. The first four fields preserve what the instruction actually
used; the last preserves the raw loaded memory word.

If these fields were reconstructed only from the final instruction bits, the
RVFI checker would see stale register-file values for forwarded dependencies.

## 2. Decode and register reads

The decoder receives opcode/funct fields from `ifid_q.insn`. `uses_rs1` and
`uses_rs2` identify architectural sources; they matter because instruction
bits in an unused field are not a real dependency. The register file provides
two combinational values. `idex_d` captures those values, PC, instruction,
immediate, destination, and controls.

For `addi x5,x0,7`, the first source address is zero and reads as zero. For the
following ADD, ID may capture an old x5 value, which is why this capture is not
yet the final RVFI source value.

## 3. Forwarded EX operands

The forwarding unit compares ID/EX source indices against valid EX/MEM and
MEM/WB destinations. In the operand `always_comb`, `FWD_EXMEM` selects
`exmem_q.alu_result`; `FWD_MEMWB` selects `memwb_q.rd_value`; otherwise the
captured register-file value is used. Each operand has its own selector. A load
is excluded from EX/MEM forwarding because its value is not ready there.

For `addi x5,x0,7; add x6,x5,x5`, both ADD inputs select 7 from the preceding
ALU result. If the mux defaulted to the raw register read, the sum could be
wrong even though decode indices matched.

## 4. Execute, branch, and PC

The ALU uses forwarded `ex_rs1` and either forwarded rs2 or an immediate.
Branch comparators use explicit `$signed` casts for BLT/BGE; unsigned forms
compare the same vectors without that cast. The target is `PC+imm` for branch
and JAL, while JALR adds its immediate to forwarded rs1 and clears bit zero.
`redirect` requires a valid legal instruction, a requested transfer, and an
aligned target. The PC register prioritizes redirect over a load-use stall.

If redirect did not take priority, a wrong-path fetch could survive a stall.
The M1 contract suppresses misaligned transfers rather than raising a trap.

## 5. Memory address, mask, and load data

`ex_effective_addr` comes from the ALU. The memory-size case shifts a byte mask
to the lane selected by address bits `[1:0]`; halfword and word cases create
two- and four-lane masks. Store data is truncated to the access width and
shifted into the selected byte lanes. In MEM, load byte/halfword selection
extracts those lanes and sign- or zero-extends according to the load type.

The formal RVFI path captures `dmem_rdata` while the load is in EX/MEM, before
the edge updates the data memory. If a store reported an unshifted source value,
the checker would compare the wrong lanes. If LB zero-extended when it should
sign-extend, `rd_wdata` would differ even when raw memory bytes were correct.

## 6. EX/MEM and MEM/WB capture

The pipeline registers carry only later-stage needs. EX/MEM stores the actual
forwarded operand values and source-use bits in RVFI builds. MEM/WB transfers
them with the instruction and captures the raw load word only for a memory
read. `valid_out` controls whether that payload has architectural meaning.
Reset clears valid state; invalid payload bits cannot request an effect.

If validity were ignored, a bubble's leftover payload could write a register
or memory. If source values were taken after the wrong edge, the load's
`rvfi_mem_rdata` could describe a later memory value.

## 7. Retirement and RVFI

The M1 retirement record is built from MEM/WB when `memwb_valid` is set. RVFI
is built from the same event. `rvfi_pc_rdata` is the retiring PC;
`rvfi_pc_wdata` is its architectural next PC. `rvfi_rd_addr` is zero unless
that instruction writes a nonzero register. The memory address is aligned to a
word; masks select lanes. Unused source addresses/data and inactive memory
fields are zeroed.

The order counter resets to zero and increments once per valid MEM/WB event.
The formal-only shadow counter advances independently from that event and
asserts equality with the public `rvfi_order`. A bubble must not consume an
order value. The upstream `unique_ch0` check supplies a second order check.

## 8. Formal properties and harness

In `rtl/regfile.sv`, the formal assertion checks each read port returns zero
when its address is x0. In `rtl/core.sv`, other `FORMAL` assertions check
retirement validity, memory side-effect gating, forwarding results/priority,
stall hold/bubble behavior, and redirect target/flush state. The formal cover
block asks for these important operating conditions to occur.

`formal/harness/formal_top.sv` toggles the processor clock from the SBY global
formal timestep and supplies reset. `formal/harness/formal_memory.sv` supplies
stable symbolic instructions and symbolic data with persistent byte writes.
Without the clock toggle, the pipeline would not advance and sequential checks
could pass without testing a running core.

## Worked sequence

```asm
addi x5, x0, 7
add  x6, x5, x5
```

The ADD reads x5 twice. Its EX forwarding mux selects the ADDI result 7 for both
inputs, so the architectural result is 14. At WB, the M1 record reports x6=14;
RVFI reports both source addresses as x5, both source values as 7, and the same
destination result. The upstream instruction check tests ADD semantics, while
the RVFI differential checks the observation against the independent M2 model.

## What this walkthrough does not establish

The internal assertion campaign is depth-16 BMC; upstream checks are depth 12;
covers are depth 24. None is unbounded. The finite memory and alignment
assumptions exclude cases documented in `formal/ASSUMPTIONS.md`. The RTL is not
claimed to be formally correct outside those boundaries.
