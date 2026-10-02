# 10 — RVFI memory semantics

ProofCore's core memory interface presents the aligned 32-bit word containing
an access. The effective byte address selects lane 0, 1, 2, or 3. RVFI uses the
aligned word address with byte masks that identify the actual operation.

For a byte load at address `0x101`, RVFI reports aligned address `0x100` and
read mask `0010`. A halfword load at `0x102` reports mask `1100`; a word load at
`0x100` reports `1111`. Stores use the same lane masks. Store data is shifted
into those byte lanes, so a byte `0xAA` stored at lane 2 appears in bits
`[23:16]` of `rvfi_mem_wdata`.

For loads, `rvfi_mem_rdata` is the aligned word from before any write at that
edge. Load sign/zero extension happens separately into `rvfi_rd_wdata`. For
stores, write masks and lane-positioned data represent the memory operation;
there is no register destination write. Non-memory instructions report zero
masks, address, and memory data.

The implementation carries the raw memory word through MEM/WB in
`rtl/proofcore_pkg.sv` and `rtl/core.sv`. Directed and random differential
checks compare all meaningful RVFI memory fields. `formal/harness/formal_memory.sv`
keeps later reads consistent with masked stores.

**Failure example:** If a byte store at lane 2 reports mask `0001`, it may write
the wrong byte even though the source register contained the correct value.
If an LB reports the extended result as `rvfi_mem_rdata`, an ISA checker can no
longer reconstruct which raw bytes were read.

**Interview question:** Why does RVFI report an aligned word address and a
separate byte mask?

**Exercise:** An LH at address `0x106` reads from aligned word `0x104`. Write
its read mask and identify the raw halfword bits in the memory word.
