# 05 — RVFI: the architectural observation port

RVFI is a standard signal record describing one instruction as it retires:
the instruction, before/after PC, source values, destination result, and memory
effects. It lets a separate checker compare the implementation with an ISA
model without reading internal pipeline registers.

ProofCore exposes RVFI only under `PROOFCORE_RVFI`. `rtl/core.sv` derives it
from the valid MEM/WB payload, the same architectural event used by the M1
retirement record. The retirement order is 64 bits and advances only when a
valid instruction retires. The first valid event after reset has order zero.

The core carries the EX operands actually used into WB. This matters because
the register-file value captured in ID may be stale while a younger instruction
depends on an older producer. The core also carries the aligned data-memory
word observed by a load so RVFI can report the read lanes and data.

Fields follow upstream semantics: `rvfi_valid` qualifies the event;
`rvfi_pc_rdata` and `rvfi_pc_wdata` are the instruction's PC and next PC;
`rvfi_rd_addr` is zero when there is no architectural destination write or the
destination is x0; unused sources have address zero; x0 data is zero. Memory
address is aligned to a 32-bit word and masks identify active byte lanes.

M-mode and RV32 encodings are reported as `rvfi_mode=3` and `rvfi_ixl=1`. This
does not add privilege or trap hardware. `rvfi_trap`, `rvfi_halt`, and
`rvfi_intr` remain zero under the M1 no-trap subset.

`make diff-rvfi-directed` checks directed cases; `make diff-rvfi-random`
cross-checks a deterministic random corpus against the independent M2 model.
The upstream checks then use that interface for ISA semantics.

**Failure example:** If source data were read from ID instead of the forwarded
EX value, the `addi/add` dependency would report zero rather than seven in the
consumer's RVFI source field even if the ALU bypass itself computed correctly.

**Interview question:** How does RVFI separate a processor implementation
from an instruction-set model?

**Exercise:** Explain why `rvfi_rd_addr` and `rvfi_rd_wdata` are zero for an
instruction that writes x0.
