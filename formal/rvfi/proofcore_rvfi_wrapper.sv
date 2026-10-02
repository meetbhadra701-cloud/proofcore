module rvfi_wrapper (
  input         clock,
  input         reset,
  `RVFI_OUTPUTS
);
  localparam logic [6:0] OPCODE_LOAD  = 7'b0000011;
  localparam logic [6:0] OPCODE_STORE = 7'b0100011;
  localparam logic [6:0] OPCODE_BRANCH = 7'b1100011;
  localparam logic [6:0] OPCODE_JALR = 7'b1100111;
  localparam logic [6:0] OPCODE_JAL = 7'b1101111;
  wire [31:0] imem_addr;
  wire [31:0] imem_rdata;
  wire [31:0] dmem_addr;
  wire        dmem_read_enable;
  wire [31:0] dmem_rdata;
  wire [3:0]  dmem_write_mask;
  wire [31:0] dmem_write_data;
  wire [31:0] rvfi_imm_i = {{20{rvfi_insn[31]}}, rvfi_insn[31:20]};
  wire [31:0] rvfi_imm_s = {{20{rvfi_insn[31]}}, rvfi_insn[31:25], rvfi_insn[11:7]};
  wire [31:0] rvfi_imm_b = {{19{rvfi_insn[31]}}, rvfi_insn[31], rvfi_insn[7],
                             rvfi_insn[30:25], rvfi_insn[11:8], 1'b0};
  wire [31:0] rvfi_imm_j = {{11{rvfi_insn[31]}}, rvfi_insn[31], rvfi_insn[19:12],
                             rvfi_insn[20], rvfi_insn[30:21], 1'b0};
  wire [31:0] rvfi_branch_target = rvfi_pc_rdata + rvfi_imm_b;
  wire [31:0] rvfi_jal_target = rvfi_pc_rdata + rvfi_imm_j;
  wire [31:0] rvfi_jalr_target = (rvfi_rs1_rdata + rvfi_imm_i) & 32'hffff_fffe;
  wire [31:0] rvfi_load_addr = rvfi_rs1_rdata + rvfi_imm_i;
  wire [31:0] rvfi_store_addr = rvfi_rs1_rdata + rvfi_imm_s;
  logic rvfi_branch_taken;

  always_comb begin
    rvfi_branch_taken = 1'b0;
    unique case (rvfi_insn[14:12])
      3'b000: rvfi_branch_taken = (rvfi_rs1_rdata == rvfi_rs2_rdata);
      3'b001: rvfi_branch_taken = (rvfi_rs1_rdata != rvfi_rs2_rdata);
      3'b100: rvfi_branch_taken = ($signed(rvfi_rs1_rdata) < $signed(rvfi_rs2_rdata));
      3'b101: rvfi_branch_taken = ($signed(rvfi_rs1_rdata) >= $signed(rvfi_rs2_rdata));
      3'b110: rvfi_branch_taken = (rvfi_rs1_rdata < rvfi_rs2_rdata);
      3'b111: rvfi_branch_taken = (rvfi_rs1_rdata >= rvfi_rs2_rdata);
      default: rvfi_branch_taken = 1'b0;
    endcase
  end

  formal_memory u_formal_memory (
    .clk(clock), .reset(reset),
    .imem_addr(imem_addr), .imem_rdata(imem_rdata),
    .dmem_addr(dmem_addr), .dmem_write_mask(dmem_write_mask),
    .dmem_write_data(dmem_write_data), .dmem_rdata(dmem_rdata)
  );

  core u_core (
    .clk(clock), .reset(reset),
    .imem_addr(imem_addr), .imem_rdata(imem_rdata),
    .dmem_addr(dmem_addr), .dmem_read_enable(dmem_read_enable), .dmem_rdata(dmem_rdata),
    .dmem_write_mask(dmem_write_mask), .dmem_write_data(dmem_write_data),
    .retire_valid(), .retire_pc(), .retire_insn(), .retire_rd(),
    .retire_rd_write(), .retire_rd_value(), .retire_mem_addr(),
    .retire_mem_read_mask(), .retire_mem_write_mask(), .retire_mem_write_data(),
    .retire_next_pc(),
    `RVFI_CONN
  );

  // The formal memory is deliberately finite: 32-byte code/data windows are
  // modeled with stable symbolic words, while all wider addresses are excluded.
  always_comb if (!reset) begin
    assume(imem_addr[31:5] == 0);
    assume(dmem_addr[31:5] == 0);
  end

  // M1 suppresses misaligned control transfers and data accesses without traps.
  // The upstream ISA model requires architectural misalignment traps, so checks
  // are restricted to naturally aligned supported executions rather than
  // pretending that the no-trap M1 policy implements those traps.
  always_comb if (!reset && rvfi_valid) begin
    unique case (rvfi_insn[6:0])
      OPCODE_BRANCH: if (rvfi_branch_taken) assume(rvfi_branch_target[1:0] == 2'b00);
      OPCODE_JAL: assume(rvfi_jal_target[1:0] == 2'b00);
      OPCODE_JALR: assume(rvfi_jalr_target[1:0] == 2'b00);
      OPCODE_LOAD: begin
        if (rvfi_insn[14:12] == 3'b001 || rvfi_insn[14:12] == 3'b101)
          assume(rvfi_load_addr[0] == 1'b0);
        if (rvfi_insn[14:12] == 3'b010)
          assume(rvfi_load_addr[1:0] == 2'b00);
      end
      OPCODE_STORE: begin
        if (rvfi_insn[14:12] == 3'b001)
          assume(rvfi_store_addr[0] == 1'b0);
        if (rvfi_insn[14:12] == 3'b010)
          assume(rvfi_store_addr[1:0] == 2'b00);
      end
      default: begin end
    endcase
  end
endmodule
