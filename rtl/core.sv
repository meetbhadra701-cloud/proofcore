module core (
  input  logic        clk,
  input  logic        reset,
  output logic [31:0] imem_addr,
  input  logic [31:0] imem_rdata,
  output logic [31:0] dmem_addr,
  output logic        dmem_read_enable,
  input  logic [31:0] dmem_rdata,
  output logic [3:0]  dmem_write_mask,
  output logic [31:0] dmem_write_data,
  output logic        retire_valid,
  output logic [31:0] retire_pc,
  output logic [31:0] retire_insn,
  output logic [4:0]  retire_rd,
  output logic        retire_rd_write,
  output logic [31:0] retire_rd_value,
  output logic [31:0] retire_mem_addr,
  output logic [3:0]  retire_mem_read_mask,
  output logic [3:0]  retire_mem_write_mask,
  output logic [31:0] retire_mem_write_data,
  output logic [31:0] retire_next_pc
);
  typedef proofcore_pkg::decode_t decode_t;
  typedef proofcore_pkg::ifid_t ifid_t;
  typedef proofcore_pkg::idex_t idex_t;
  typedef proofcore_pkg::exmem_t exmem_t;
  typedef proofcore_pkg::memwb_t memwb_t;
  typedef proofcore_pkg::retire_t retire_t;
  typedef proofcore_pkg::alu_op_e alu_op_e;
  typedef proofcore_pkg::imm_kind_e imm_kind_e;
  typedef proofcore_pkg::mem_size_e mem_size_e;
  typedef proofcore_pkg::branch_e branch_e;
  typedef proofcore_pkg::wb_sel_e wb_sel_e;
  typedef proofcore_pkg::fwd_sel_e fwd_sel_e;
  localparam mem_size_e SIZE_BYTE = proofcore_pkg::SIZE_BYTE;
  localparam mem_size_e SIZE_HALF = proofcore_pkg::SIZE_HALF;
  localparam mem_size_e SIZE_WORD = proofcore_pkg::SIZE_WORD;
  localparam branch_e BR_NONE = proofcore_pkg::BR_NONE;
  localparam branch_e BR_EQ = proofcore_pkg::BR_EQ;
  localparam branch_e BR_NE = proofcore_pkg::BR_NE;
  localparam branch_e BR_LT = proofcore_pkg::BR_LT;
  localparam branch_e BR_GE = proofcore_pkg::BR_GE;
  localparam branch_e BR_LTU = proofcore_pkg::BR_LTU;
  localparam branch_e BR_GEU = proofcore_pkg::BR_GEU;
  localparam fwd_sel_e FWD_EXMEM = proofcore_pkg::FWD_EXMEM;
  localparam fwd_sel_e FWD_MEMWB = proofcore_pkg::FWD_MEMWB;
  localparam wb_sel_e WB_LOAD = proofcore_pkg::WB_LOAD;
  localparam wb_sel_e WB_PC4 = proofcore_pkg::WB_PC4;
  logic [31:0] pc_q;
  logic ifid_valid, idex_valid, exmem_valid, memwb_valid;
  ifid_t ifid_q, ifid_d;
  idex_t idex_q, idex_d;
  exmem_t exmem_q, exmem_d;
  memwb_t memwb_q, memwb_d;
  retire_t retire_record;

  decode_t ifid_ctl;
  logic [25:0] ifid_ctl_bits;
  logic [31:0] ifid_imm;
  logic [31:0] rf_rdata1, rf_rdata2;
  logic load_use_stall;
  logic redirect;
  logic [31:0] redirect_target;
  logic [31:0] ex_rs1, ex_rs2, ex_alu_b, ex_alu_result, ex_store_payload;
  logic [31:0] ex_target_candidate, ex_effective_addr;
  logic ex_branch_taken, ex_transfer_requested, ex_transfer_misaligned, ex_data_misaligned;
  logic [31:0] ex_next_pc;
  logic ex_mem_read, ex_mem_write, ex_reg_write;
  logic [3:0] ex_mem_rmask, ex_mem_wmask;
  logic [31:0] ex_mem_wdata;
  fwd_sel_e fwd_rs1_sel, fwd_rs2_sel;
  logic [7:0] load_byte;
  logic [15:0] load_half;
  logic [31:0] load_value, wb_value;

  assign imem_addr = pc_q;
  assign ifid_d.pc = pc_q;
  assign ifid_d.insn = imem_rdata;

  decoder u_decoder (
    .opcode(ifid_q.insn[6:0]), .funct3(ifid_q.insn[14:12]), .funct7(ifid_q.insn[31:25]), .ctl_bits(ifid_ctl_bits)
  );
  assign ifid_ctl = ifid_ctl_bits;
  imm_gen u_imm_gen (.insn(ifid_q.insn[31:7]), .kind(ifid_ctl.imm_kind), .imm(ifid_imm));
  regfile u_regfile (
    .clk(clk), .reset(reset),
    .we(memwb_valid && memwb_q.reg_write && (memwb_q.rd != 5'd0)),
    .waddr(memwb_q.rd), .wdata(memwb_q.rd_value),
    .raddr1(ifid_q.insn[19:15]), .raddr2(ifid_q.insn[24:20]),
    .rdata1(rf_rdata1), .rdata2(rf_rdata2)
  );

  hazard_unit u_hazard (
    .ifid_valid(ifid_valid), .ifid_uses_rs1(ifid_ctl.uses_rs1), .ifid_uses_rs2(ifid_ctl.uses_rs2),
    .ifid_rs1(ifid_q.insn[19:15]), .ifid_rs2(ifid_q.insn[24:20]),
    .idex_valid(idex_valid), .idex_is_load(idex_q.ctl.mem_read), .idex_rd(idex_q.rd),
    .load_use_stall(load_use_stall)
  );

  always_comb begin
    idex_d = '0;
    idex_d.pc = ifid_q.pc;
    idex_d.pc4 = ifid_q.pc + 32'd4;
    idex_d.insn = ifid_q.insn;
    idex_d.rs1 = ifid_q.insn[19:15];
    idex_d.rs2 = ifid_q.insn[24:20];
    idex_d.rd = ifid_q.insn[11:7];
    idex_d.rs1_value = rf_rdata1;
    idex_d.rs2_value = rf_rdata2;
    idex_d.imm = ifid_imm;
    idex_d.ctl = ifid_ctl;
  end

  forwarding u_forwarding (
    .exmem_valid(exmem_valid), .exmem_reg_write(exmem_q.reg_write),
    .exmem_is_load(exmem_q.mem_read), .exmem_rd(exmem_q.rd),
    .memwb_valid(memwb_valid), .memwb_reg_write(memwb_q.reg_write), .memwb_rd(memwb_q.rd),
    .idex_rs1(idex_q.rs1), .idex_rs2(idex_q.rs2),
    .rs1_sel(fwd_rs1_sel), .rs2_sel(fwd_rs2_sel)
  );

  always_comb begin
    ex_rs1 = idex_q.rs1_value;
    ex_rs2 = idex_q.rs2_value;
    unique case (fwd_rs1_sel)
      FWD_EXMEM: ex_rs1 = exmem_q.alu_result;
      FWD_MEMWB: ex_rs1 = memwb_q.rd_value;
      default:   ex_rs1 = idex_q.rs1_value;
    endcase
    unique case (fwd_rs2_sel)
      FWD_EXMEM: ex_rs2 = exmem_q.alu_result;
      FWD_MEMWB: ex_rs2 = memwb_q.rd_value;
      default:   ex_rs2 = idex_q.rs2_value;
    endcase
    ex_alu_b = idex_q.ctl.use_imm ? idex_q.imm : ex_rs2;
  end

  alu u_alu (.a(ex_rs1), .b(ex_alu_b), .op(idex_q.ctl.alu_op), .result(ex_alu_result));

  always_comb begin
    ex_branch_taken = 1'b0;
    unique case (idex_q.ctl.branch)
      BR_EQ:  ex_branch_taken = (ex_rs1 == ex_rs2);
      BR_NE:  ex_branch_taken = (ex_rs1 != ex_rs2);
      BR_LT:  ex_branch_taken = ($signed(ex_rs1) < $signed(ex_rs2));
      BR_GE:  ex_branch_taken = ($signed(ex_rs1) >= $signed(ex_rs2));
      BR_LTU: ex_branch_taken = (ex_rs1 < ex_rs2);
      BR_GEU: ex_branch_taken = (ex_rs1 >= ex_rs2);
      default: ex_branch_taken = 1'b0;
    endcase

    ex_target_candidate = idex_q.pc + idex_q.imm;
    if (idex_q.ctl.is_jalr) ex_target_candidate = (ex_rs1 + idex_q.imm) & 32'hffff_fffe;
    redirect_target = ex_target_candidate;
    ex_transfer_requested = idex_q.ctl.is_jal || idex_q.ctl.is_jalr ||
                             ((idex_q.ctl.branch != BR_NONE) && ex_branch_taken);
    ex_transfer_misaligned = ex_transfer_requested && (ex_target_candidate[1:0] != 2'b00);
    redirect = idex_valid && idex_q.ctl.legal && ex_transfer_requested && !ex_transfer_misaligned;
    ex_next_pc = redirect ? ex_target_candidate : idex_q.pc4;

    ex_effective_addr = ex_alu_result;
    ex_data_misaligned = 1'b0;
    if (idex_q.ctl.mem_read || idex_q.ctl.mem_write) begin
      if ((idex_q.ctl.mem_size == SIZE_HALF) && ex_effective_addr[0]) ex_data_misaligned = 1'b1;
      if ((idex_q.ctl.mem_size == SIZE_WORD) && (ex_effective_addr[1:0] != 2'b00)) ex_data_misaligned = 1'b1;
    end

    ex_mem_read = idex_valid && idex_q.ctl.legal && idex_q.ctl.mem_read && !ex_data_misaligned;
    ex_mem_write = idex_valid && idex_q.ctl.legal && idex_q.ctl.mem_write && !ex_data_misaligned;
    ex_reg_write = idex_valid && idex_q.ctl.legal && idex_q.ctl.reg_write &&
                   !ex_data_misaligned && !ex_transfer_misaligned;
    ex_mem_rmask = 4'b0;
    ex_mem_wmask = 4'b0;
    unique case (idex_q.ctl.mem_size)
      SIZE_BYTE: begin
        ex_mem_rmask = ex_mem_read ? (4'b0001 << ex_effective_addr[1:0]) : 4'b0;
        ex_mem_wmask = ex_mem_write ? (4'b0001 << ex_effective_addr[1:0]) : 4'b0;
      end
      SIZE_HALF: begin
        ex_mem_rmask = ex_mem_read ? (4'b0011 << ex_effective_addr[1:0]) : 4'b0;
        ex_mem_wmask = ex_mem_write ? (4'b0011 << ex_effective_addr[1:0]) : 4'b0;
      end
      SIZE_WORD: begin
        ex_mem_rmask = ex_mem_read ? 4'b1111 : 4'b0;
        ex_mem_wmask = ex_mem_write ? 4'b1111 : 4'b0;
      end
      default: begin ex_mem_rmask = 4'b0; ex_mem_wmask = 4'b0; end
    endcase
    unique case (idex_q.ctl.mem_size)
      SIZE_BYTE: ex_store_payload = {24'b0, ex_rs2[7:0]};
      SIZE_HALF: ex_store_payload = {16'b0, ex_rs2[15:0]};
      SIZE_WORD: ex_store_payload = ex_rs2;
      default: ex_store_payload = 32'b0;
    endcase
    ex_mem_wdata = ex_store_payload << (8 * ex_effective_addr[1:0]);
  end

  always_comb begin
    exmem_d = '0;
    exmem_d.pc = idex_q.pc;
    exmem_d.pc4 = idex_q.pc4;
    exmem_d.insn = idex_q.insn;
    exmem_d.rd = (ex_reg_write && (idex_q.rd != 5'd0)) ? idex_q.rd : 5'd0;
    exmem_d.reg_write = ex_reg_write && (idex_q.rd != 5'd0);
    exmem_d.alu_result = idex_q.ctl.upper_lui ? idex_q.imm :
                         (idex_q.ctl.upper_auipc ? idex_q.pc + idex_q.imm :
                          (idex_q.ctl.is_jal || idex_q.ctl.is_jalr ? idex_q.pc4 : ex_alu_result));
    exmem_d.effective_addr = ex_effective_addr;
    exmem_d.mem_read = ex_mem_read;
    exmem_d.mem_write = ex_mem_write;
    exmem_d.load_unsigned = idex_q.ctl.load_unsigned;
    exmem_d.mem_size = idex_q.ctl.mem_size;
    exmem_d.wb_sel = idex_q.ctl.wb_sel;
    exmem_d.next_pc = ex_next_pc;
    exmem_d.mem_read_mask = ex_mem_rmask;
    exmem_d.mem_write_mask = ex_mem_wmask;
    exmem_d.mem_write_data = ex_mem_wdata;
  end

  assign dmem_addr = {exmem_q.effective_addr[31:2], 2'b00};
  assign dmem_read_enable = exmem_valid && exmem_q.mem_read;
  assign dmem_write_mask = (exmem_valid && exmem_q.mem_write) ? exmem_q.mem_write_mask : 4'b0;
  assign dmem_write_data = (exmem_valid && exmem_q.mem_write) ? exmem_q.mem_write_data : 32'b0;

  always_comb begin
    unique case (exmem_q.effective_addr[1:0])
      2'd0: load_byte = dmem_rdata[7:0];
      2'd1: load_byte = dmem_rdata[15:8];
      2'd2: load_byte = dmem_rdata[23:16];
      default: load_byte = dmem_rdata[31:24];
    endcase
    load_half = exmem_q.effective_addr[1] ? dmem_rdata[31:16] : dmem_rdata[15:0];
    load_value = 32'b0;
    unique case (exmem_q.mem_size)
      SIZE_BYTE: load_value = exmem_q.load_unsigned ? {24'b0, load_byte} : {{24{load_byte[7]}}, load_byte};
      SIZE_HALF: load_value = exmem_q.load_unsigned ? {16'b0, load_half} : {{16{load_half[15]}}, load_half};
      SIZE_WORD: load_value = dmem_rdata;
      default: load_value = 32'b0;
    endcase

    unique case (exmem_q.wb_sel)
      WB_LOAD: wb_value = load_value;
      WB_PC4:  wb_value = exmem_q.pc4;
      default: wb_value = exmem_q.alu_result;
    endcase
  end

  always_comb begin
    memwb_d = '0;
    memwb_d.pc = exmem_q.pc;
    memwb_d.insn = exmem_q.insn;
    memwb_d.rd = exmem_q.rd;
    memwb_d.reg_write = exmem_q.reg_write;
    memwb_d.rd_value = exmem_q.reg_write ? wb_value : 32'b0;
    memwb_d.mem_addr = (exmem_q.mem_read || exmem_q.mem_write) ? exmem_q.effective_addr : 32'b0;
    memwb_d.mem_read_mask = exmem_q.mem_read_mask;
    memwb_d.mem_write_mask = exmem_q.mem_write_mask;
    memwb_d.mem_write_data = exmem_q.mem_write ? exmem_q.mem_write_data : 32'b0;
    memwb_d.next_pc = exmem_q.next_pc;
  end

  always_comb begin
    retire_record = '0;
    if (memwb_valid) begin
      retire_record.valid = 1'b1;
      retire_record.pc = memwb_q.pc;
      retire_record.insn = memwb_q.insn;
      retire_record.rd = memwb_q.reg_write ? memwb_q.rd : 5'b0;
      retire_record.rd_write = memwb_q.reg_write;
      retire_record.rd_value = memwb_q.reg_write ? memwb_q.rd_value : 32'b0;
      retire_record.mem_addr = memwb_q.mem_addr;
      retire_record.mem_read_mask = memwb_q.mem_read_mask;
      retire_record.mem_write_mask = memwb_q.mem_write_mask;
      retire_record.mem_write_data = memwb_q.mem_write_data;
      retire_record.next_pc = memwb_q.next_pc;
    end
  end
  assign retire_valid = retire_record.valid;
  assign retire_pc = retire_record.pc;
  assign retire_insn = retire_record.insn;
  assign retire_rd = retire_record.rd;
  assign retire_rd_write = retire_record.rd_write;
  assign retire_rd_value = retire_record.rd_value;
  assign retire_mem_addr = retire_record.mem_addr;
  assign retire_mem_read_mask = retire_record.mem_read_mask;
  assign retire_mem_write_mask = retire_record.mem_write_mask;
  assign retire_mem_write_data = retire_record.mem_write_data;
  assign retire_next_pc = retire_record.next_pc;

  always_ff @(posedge clk) begin
    if (reset) pc_q <= 32'b0;
    else if (redirect) pc_q <= redirect_target;
    else if (!load_use_stall) pc_q <= pc_q + 32'd4;
  end

  if_id u_if_id (
    .clk(clk), .reset(reset), .hold(load_use_stall), .flush(redirect),
    .valid_in(1'b1), .data_in(ifid_d), .valid_out(ifid_valid), .data_out(ifid_q)
  );
  id_ex u_id_ex (
    .clk(clk), .reset(reset), .flush(redirect),
    .valid_in(ifid_valid && !load_use_stall), .data_in(idex_d),
    .valid_out(idex_valid), .data_out(idex_q)
  );
  ex_mem u_ex_mem (
    .clk(clk), .reset(reset), .valid_in(idex_valid), .data_in(exmem_d),
    .valid_out(exmem_valid), .data_out(exmem_q)
  );
  mem_wb u_mem_wb (
    .clk(clk), .reset(reset), .valid_in(exmem_valid), .data_in(memwb_d),
    .valid_out(memwb_valid), .data_out(memwb_q)
  );
endmodule
