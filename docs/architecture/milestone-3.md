# Milestone 3 RVFI architecture

M3 adds an observation interface at the processor's existing WB retirement
point. It does not move execution, change the M1 retirement record, add pipeline
stages, or enable RVFI in normal builds.

```mermaid
flowchart LR
  ID[ID register reads] --> EX[EX forwarding muxes]
  EX -->|used rs1/rs2 values| XM[EX/MEM payload]
  XM --> MW[MEM/WB payload]
  MEM[Memory read word] -->|load pre-state word| MW
  MW --> RET[Valid retirement]
  RET --> M1[M1 retirement record]
  RET --> RVFI[Optional RVFI ports]
  RVFI --> RF[riscv-formal checks]
```

`PROOFCORE_RVFI` conditionally extends the EX/MEM and MEM/WB payloads with the
decoder's source-use bits, the actual forwarded operands, and (for loads) the
aligned memory word observed before a write edge. At retirement, unused
register sources are reported as address zero; x0 source data is zero; writes
to x0 have `rvfi_rd_addr=0` and `rvfi_rd_wdata=0`. Memory masks are lane masks
for the aligned 32-bit word and non-memory events have zero masks/data/address.

The order register resets to zero and increments when the MEM/WB valid bit is
set. Therefore the event observed during the first post-reset valid retirement
has order zero; invalid bubbles and flushed instructions do not advance the
counter. The upstream `unique` check and an independent internal shadow counter
check order behavior.

`rvfi_mode=2'b11` and `rvfi_ixl=2'b01` are the upstream encodings for M-mode and
RV32. They describe the interface's intended modeling scope; they do not add
privilege or trap logic. `rvfi_trap`, `rvfi_halt`, and `rvfi_intr` remain zero
under the frozen M1 policy. See `formal/ASSUMPTIONS.md` for the resulting
non-claims.
