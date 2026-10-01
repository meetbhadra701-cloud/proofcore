# 16 — Important code walkthrough

This is a guided tour of the decisions that make the processor behave correctly. File links are relative to the project. Read the relevant RTL beside each explanation; do not try to memorize the whole core at once.

## 1. PC update — `rtl/core.sv`, `always_ff` at the bottom

```systemverilog
always_ff @(posedge clk) begin
  if (reset) pc_q <= 32'b0;
  else if (redirect) pc_q <= redirect_target;
  else if (!load_use_stall) pc_q <= pc_q + 32'd4;
end
```

`pc_q` remembers where instruction fetch should read. Reset starts at zero. A valid EX redirect has first priority; this is why a taken branch can replace the normal next address. If the front end is stalled, PC holds. Otherwise PC advances four bytes because every ProofCore instruction is 32 bits.

**Example:** PC=`0x20`, ordinary instruction -> `0x24`; taken branch target=`0x40` -> `0x40`; load-use stall -> stays `0x20`.

**If wrong:** Advancing during a stall skips the held instruction. Letting sequential increment outrank redirect fetches the wrong path.

## 2. Fetch and IF/ID — `rtl/core.sv` plus `rtl/pipeline/if_id.sv`

```systemverilog
assign imem_addr = pc_q;
assign ifid_d.pc = pc_q;
assign ifid_d.insn = imem_rdata;
```

The memory sees the current PC; the IF/ID payload stores that same PC beside the returned instruction. Later stages use the carried PC to calculate PC-relative targets, link values, and retirement identity.

```systemverilog
if (reset || flush) begin
  valid_out <= 1'b0;
  data_out <= '0;
end else if (!hold) begin
  valid_out <= valid_in;
  data_out <= valid_in ? data_in : '0;
end
```

Reset and flush empty the stage. Hold preserves the old instruction during a load-use stall. Normal advance captures a new instruction. Flush comes before hold, so a redirect discards wrong-path work even if a stall were also asserted.

**If wrong:** Pairing an instruction with a different PC corrupts branch targets and retirement records. Forgetting to clear valid on flush can commit wrong-path work.

## 3. Decode — `rtl/decoder.sv`

```systemverilog
always_comb begin
  ctl = '0;
  ctl.alu_op = ALU_ADD;
  ctl.imm_kind = IMM_I;
  ctl.mem_size = SIZE_WORD;
  ctl.branch = BR_NONE;
  ctl.wb_sel = WB_ALU;
  unique case (opcode)
    ...
    default: ctl = '0;
  endcase
end
assign ctl_bits = ctl;
```

Starting from no side effects is a safety habit: a new or unsupported bit pattern remains inert until a matching case explicitly enables its controls. The struct's source-use fields tell the hazard unit which register fields really are operands.

**Example:** A load enables rs1, immediate use, memory read, and register write; a store enables rs1, rs2, immediate use, and memory write, but no destination write.

**If wrong:** An invalid funct7 accidentally accepted as ADD changes architectural state. A store mistakenly marked `reg_write` could corrupt a register.

## 4. Immediate generator — `rtl/imm_gen.sv`

```systemverilog
IMM_I: imm = {{20{insn[31]}}, insn[31:20]};
IMM_S: imm = {{20{insn[31]}}, insn[31:25], insn[11:7]};
IMM_B: imm = {{19{insn[31]}}, insn[31], insn[7], insn[30:25], insn[11:8], 1'b0};
IMM_U: imm = {insn[31:12], 12'b0};
IMM_J: imm = {{11{insn[31]}}, insn[31], insn[19:12], insn[20], insn[30:21], 1'b0};
```

The repeated sign bit keeps negative offsets negative at 32 bits. B/J formats put their least-significant offset bit at zero because their encoded offsets advance in two-byte units. U is already a high-positioned constant, so twelve zeros are appended.

**Example:** A 12-bit immediate `0xFFF` becomes `0xFFFFFFFF`, which means -1.

**If wrong:** Misplaced B bits break branches and can produce plausible but incorrect targets.

## 5. Register reads and WB write-through — `rtl/regfile.sv`

```systemverilog
if (raddr1 != 5'd0) begin
  rdata1 = regs[raddr1];
  if (we && (waddr == raddr1)) rdata1 = wdata;
end
```

The source address x0 leaves the default zero unchanged. For other addresses, read the stored value, then override it with the value being written at WB when addresses match. The override prevents ID from capturing stale data on a same-cycle write/read.

**If wrong:** Removing x0 handling makes x0 writable. Removing the match override can expose stale data in a pipeline timing corner.

## 6. ID/EX capture — `rtl/core.sv` and `rtl/pipeline/id_ex.sv`

```systemverilog
idex_d.pc = ifid_q.pc;
idex_d.pc4 = ifid_q.pc + 32'd4;
idex_d.insn = ifid_q.insn;
idex_d.rs1_value = rf_rdata1;
idex_d.rs2_value = rf_rdata2;
idex_d.imm = ifid_imm;
idex_d.ctl = ifid_ctl;
```

ID/EX carries the values EX needs; later stages should not decode the instruction again. On a load-use stall, `valid_in` is false, so the load still advances to EX/MEM while a bubble enters ID/EX.

**If wrong:** Omitting the instruction's PC makes PC-relative work incorrect. Omitting a control such as `mem_size` makes a later stage guess what the instruction meant.

## 7. Forwarding compare and priority — `rtl/forwarding.sv`

```systemverilog
if (exmem_valid && exmem_reg_write && !exmem_is_load && (exmem_rd != 5'd0)) begin
  if (exmem_rd == idex_rs1) rs1_sel = FWD_EXMEM;
  if (exmem_rd == idex_rs2) rs2_sel = FWD_EXMEM;
end
if (memwb_valid && memwb_reg_write && (memwb_rd != 5'd0)) begin
  if ((rs1_sel == FWD_RF) && (memwb_rd == idex_rs1)) rs1_sel = FWD_MEMWB;
  if ((rs2_sel == FWD_RF) && (memwb_rd == idex_rs2)) rs2_sel = FWD_MEMWB;
end
```

`valid` prevents a bubble from acting as a producer. `reg_write` says an architectural value exists. `rd != 0` protects x0. `!exmem_is_load` matters because EX/MEM has a load address, not load data. The MEM/WB conditions only fill a still-register-file-selected mux, so the newer EX/MEM match wins.

**Example:** Two back-to-back writes to x1 produce 1 then 2. A consumer sees both matches and selects 2.

**If wrong:** Removing EX/MEM priority computes with the stale value 1. Forwarding a load's address instead of its data creates a plausible but incorrect result.

## 8. Load-use detection — `rtl/hazard_unit.sv`

```systemverilog
if (ifid_valid && idex_valid && idex_is_load && (idex_rd != 5'd0)) begin
  load_use_stall = (ifid_uses_rs1 && (ifid_rs1 == idex_rd)) ||
                   (ifid_uses_rs2 && (ifid_rs2 == idex_rd));
end
```

The producer is a real load in EX. The consumer is a real instruction in ID. The destination is nonzero. A dependency exists only if the consumer actually uses the matching source.

**Example:** `lw x5,...; add x6,x5,x5` matches both source terms but inserts only one bubble.

**If wrong:** Comparing unused immediate bits creates false stalls; omitting rs2 misses store or arithmetic dependencies.

## 9. ALU operand muxes — `rtl/core.sv`

```systemverilog
unique case (fwd_rs1_sel)
  FWD_EXMEM: ex_rs1 = exmem_q.alu_result;
  FWD_MEMWB: ex_rs1 = memwb_q.rd_value;
  default:   ex_rs1 = idex_q.rs1_value;
endcase
ex_alu_b = idex_q.ctl.use_imm ? idex_q.imm : ex_rs2;
```

The selector chooses a value; it is not the value itself. Operand B then chooses between the forwarded rs2 value and the instruction immediate. Loads/stores need an immediate address offset; R-type arithmetic needs rs2.

**If wrong:** Using rs2 for ADDI adds a register number's value instead of its immediate. Feeding the raw register-file value to EX loses forwarded dependencies.

## 10. Branch condition and target — `rtl/core.sv`

```systemverilog
BR_LT:  ex_branch_taken = ($signed(ex_rs1) < $signed(ex_rs2));
BR_LTU: ex_branch_taken = (ex_rs1 < ex_rs2);
...
if (idex_q.ctl.is_jalr)
  ex_target_candidate = (ex_rs1 + idex_q.imm) & 32'hffff_fffe;
```

The compare uses forwarded values. Signed branch cases explicitly cast; unsigned cases do not. JALR computes from its rs1 operand and clears bit zero. The contract suppresses a transfer if the resulting target is not four-byte aligned.

**If wrong:** BLTU implemented with signed casts disagrees for values with the high bit set. Using current fetch PC instead of `idex_q.pc` makes branch targets depend on pipeline position.

## 11. EX/MEM controls, store mask, and payload — `rtl/core.sv`

```systemverilog
ex_mem_write = idex_valid && idex_q.ctl.legal && idex_q.ctl.mem_write && !ex_data_misaligned;
...
SIZE_BYTE: ex_mem_wmask = ex_mem_write ? (4'b0001 << ex_effective_addr[1:0]) : 4'b0;
SIZE_HALF: ex_mem_wmask = ex_mem_write ? (4'b0011 << ex_effective_addr[1:0]) : 4'b0;
SIZE_WORD: ex_mem_wmask = ex_mem_write ? 4'b1111 : 4'b0;
```

Validity, decode legality, store intent, and alignment must all allow the write. Offset bits shift the byte/half mask into the correct lanes. Before shifting data, byte stores keep only source bits `[7:0]` and half stores keep `[15:0]`.

**Example:** For a byte store at lane 1 with source `0xFFFFFFAA`, write mask is `0010`, write data is `0x0000AA00`.

**If wrong:** A missing valid guard lets a bubble store. An unshifted mask updates the wrong bytes. Unmasked source data makes the retirement record misleading.

## 12. Load extraction and sign extension — `rtl/core.sv`

```systemverilog
SIZE_BYTE: load_value = exmem_q.load_unsigned
  ? {24'b0, load_byte} : {{24{load_byte[7]}}, load_byte};
SIZE_HALF: load_value = exmem_q.load_unsigned
  ? {16'b0, load_half} : {{16{load_half[15]}}, load_half};
SIZE_WORD: load_value = dmem_rdata;
```

The byte/half selector chooses a lane from the aligned word. Unsigned loads put zeros in the high bits. Signed loads copy the selected sign bit. A word uses all 32 returned bits.

**If wrong:** LBU of `0x80` becomes `0xFFFFFF80` instead of `0x00000080`, or LB of `0x80` becomes positive 128.

## 13. MEM/WB and register writeback — `rtl/core.sv`

```systemverilog
memwb_d.rd_value = exmem_q.reg_write ? wb_value : 32'b0;
...
.we(memwb_valid && memwb_q.reg_write && (memwb_q.rd != 5'd0))
```

MEM/WB stores the final result after load selection. At the rising edge, the register file writes only when this valid WB instruction requests a nonzero destination. Loads, links, LUI, AUIPC, and ALU results select their appropriate values before capture.

**If wrong:** Selecting the ALU address for a load writes the address instead of memory data. Enabling writes on invalid state commits bubbles.

## 14. Flush — `rtl/core.sv` plus pipeline stages

```systemverilog
.hold(load_use_stall), .flush(redirect)
...
.flush(redirect), .valid_in(ifid_valid && !load_use_stall)
```

IF/ID is held on a load-use interlock and flushed on redirect. ID/EX receives invalid input for a stall and is flushed for a redirect. EX/MEM still captures the instruction already in EX, preserving older work.

**If wrong:** Holding ID/EX during a load-use stall pins the load in EX. Flushing EX/MEM destroys an older result. Missing IF/ID flush permits wrong-path state changes.

## 15. Retirement record — `rtl/core.sv`

```systemverilog
always_comb begin
  retire_record = '0;
  if (memwb_valid) begin
    retire_record.valid = 1'b1;
    retire_record.pc = memwb_q.pc;
    retire_record.insn = memwb_q.insn;
    ...
    retire_record.next_pc = memwb_q.next_pc;
  end
end
```

The zero default means no retirement has no stale output fields. A valid MEM/WB instruction produces one record with PC, instruction, write information, memory effects, and next PC. The record is exposed as individual ports for tool compatibility.

**If wrong:** Reporting from ID or EX would include work that can still be stalled or flushed. Calling this RVFI would falsely claim a standard interface that this record does not implement.

## What the whole walkthrough teaches

Most pipeline bugs are mismatches between *which instruction* a value belongs to and *when* that value is available. Carry the PC/control with the instruction, qualify effects with valid, select the newest available operand, wait for loads, and flush only younger work. Those rules explain nearly every important condition in the core.

## Exercises

1. Remove `exmem_is_load` from forwarding. Which load-use program fails first, and what wrong value is selected?
2. Remove the `uses_rs2` guard in hazard detection. Construct a load whose destination matches immediate bits in an instruction that does not read rs2.
3. Which one condition protects memory from an invalid pipeline stage?

<details><summary>Answers</summary>

1. The dependent ADD can receive the load's effective address from EX/MEM instead of loaded data.
2. An immediate instruction can falsely stall if its encoded immediate's low five bits equal the load destination.
3. The data-memory write mask is qualified by both `exmem_valid` and `exmem_q.mem_write`.

</details>
