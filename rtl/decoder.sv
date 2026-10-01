module decoder (
  input  logic [6:0]  opcode,
  input  logic [2:0]  funct3,
  input  logic [6:0]  funct7,
  output logic [25:0] ctl_bits
);
  typedef proofcore_pkg::decode_t decode_t;
  typedef proofcore_pkg::alu_op_e alu_op_e;
  typedef proofcore_pkg::imm_kind_e imm_kind_e;
  typedef proofcore_pkg::mem_size_e mem_size_e;
  typedef proofcore_pkg::branch_e branch_e;
  typedef proofcore_pkg::wb_sel_e wb_sel_e;
  localparam alu_op_e ALU_ADD = proofcore_pkg::ALU_ADD;
  localparam alu_op_e ALU_SUB = proofcore_pkg::ALU_SUB;
  localparam alu_op_e ALU_SLL = proofcore_pkg::ALU_SLL;
  localparam alu_op_e ALU_SLT = proofcore_pkg::ALU_SLT;
  localparam alu_op_e ALU_SLTU = proofcore_pkg::ALU_SLTU;
  localparam alu_op_e ALU_XOR = proofcore_pkg::ALU_XOR;
  localparam alu_op_e ALU_SRL = proofcore_pkg::ALU_SRL;
  localparam alu_op_e ALU_SRA = proofcore_pkg::ALU_SRA;
  localparam alu_op_e ALU_OR = proofcore_pkg::ALU_OR;
  localparam alu_op_e ALU_AND = proofcore_pkg::ALU_AND;
  localparam imm_kind_e IMM_I = proofcore_pkg::IMM_I;
  localparam imm_kind_e IMM_S = proofcore_pkg::IMM_S;
  localparam imm_kind_e IMM_B = proofcore_pkg::IMM_B;
  localparam imm_kind_e IMM_U = proofcore_pkg::IMM_U;
  localparam imm_kind_e IMM_J = proofcore_pkg::IMM_J;
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
  localparam wb_sel_e WB_ALU = proofcore_pkg::WB_ALU;
  localparam wb_sel_e WB_LOAD = proofcore_pkg::WB_LOAD;
  localparam wb_sel_e WB_PC4 = proofcore_pkg::WB_PC4;
  localparam wb_sel_e WB_UPPER = proofcore_pkg::WB_UPPER;
  decode_t ctl;
  always_comb begin
    ctl = '0;
    ctl.alu_op = ALU_ADD;
    ctl.imm_kind = IMM_I;
    ctl.mem_size = SIZE_WORD;
    ctl.branch = BR_NONE;
    ctl.wb_sel = WB_ALU;

    unique case (opcode)
      7'b0110011: begin // OP
        ctl.uses_rs1 = 1'b1; ctl.uses_rs2 = 1'b1; ctl.reg_write = 1'b1; ctl.legal = 1'b1;
        unique case ({funct7, funct3})
          {7'b0000000,3'b000}: ctl.alu_op = ALU_ADD;
          {7'b0100000,3'b000}: ctl.alu_op = ALU_SUB;
          {7'b0000000,3'b001}: ctl.alu_op = ALU_SLL;
          {7'b0000000,3'b010}: ctl.alu_op = ALU_SLT;
          {7'b0000000,3'b011}: ctl.alu_op = ALU_SLTU;
          {7'b0000000,3'b100}: ctl.alu_op = ALU_XOR;
          {7'b0000000,3'b101}: ctl.alu_op = ALU_SRL;
          {7'b0100000,3'b101}: ctl.alu_op = ALU_SRA;
          {7'b0000000,3'b110}: ctl.alu_op = ALU_OR;
          {7'b0000000,3'b111}: ctl.alu_op = ALU_AND;
          default: ctl = '0;
        endcase
      end
      7'b0010011: begin // OP-IMM
        ctl.uses_rs1 = 1'b1; ctl.use_imm = 1'b1; ctl.reg_write = 1'b1; ctl.legal = 1'b1;
        unique case (funct3)
          3'b000: ctl.alu_op = ALU_ADD;
          3'b010: ctl.alu_op = ALU_SLT;
          3'b011: ctl.alu_op = ALU_SLTU;
          3'b100: ctl.alu_op = ALU_XOR;
          3'b110: ctl.alu_op = ALU_OR;
          3'b111: ctl.alu_op = ALU_AND;
          3'b001: if (funct7 == 7'b0000000) ctl.alu_op = ALU_SLL; else ctl = '0;
          3'b101: begin
            if (funct7 == 7'b0000000) ctl.alu_op = ALU_SRL;
            else if (funct7 == 7'b0100000) ctl.alu_op = ALU_SRA;
            else ctl = '0;
          end
          default: ctl = '0;
        endcase
      end
      7'b0000011: begin // LOAD
        ctl.uses_rs1 = 1'b1; ctl.use_imm = 1'b1; ctl.reg_write = 1'b1;
        ctl.mem_read = 1'b1; ctl.wb_sel = WB_LOAD; ctl.legal = 1'b1;
        unique case (funct3)
          3'b000: ctl.mem_size = SIZE_BYTE;
          3'b001: ctl.mem_size = SIZE_HALF;
          3'b010: ctl.mem_size = SIZE_WORD;
          3'b100: begin ctl.mem_size = SIZE_BYTE; ctl.load_unsigned = 1'b1; end
          3'b101: begin ctl.mem_size = SIZE_HALF; ctl.load_unsigned = 1'b1; end
          default: ctl = '0;
        endcase
      end
      7'b0100011: begin // STORE
        ctl.uses_rs1 = 1'b1; ctl.uses_rs2 = 1'b1; ctl.use_imm = 1'b1;
        ctl.mem_write = 1'b1; ctl.imm_kind = IMM_S; ctl.legal = 1'b1;
        unique case (funct3)
          3'b000: ctl.mem_size = SIZE_BYTE;
          3'b001: ctl.mem_size = SIZE_HALF;
          3'b010: ctl.mem_size = SIZE_WORD;
          default: ctl = '0;
        endcase
      end
      7'b1100011: begin // BRANCH
        ctl.uses_rs1 = 1'b1; ctl.uses_rs2 = 1'b1; ctl.imm_kind = IMM_B; ctl.legal = 1'b1;
        unique case (funct3)
          3'b000: ctl.branch = BR_EQ;
          3'b001: ctl.branch = BR_NE;
          3'b100: ctl.branch = BR_LT;
          3'b101: ctl.branch = BR_GE;
          3'b110: ctl.branch = BR_LTU;
          3'b111: ctl.branch = BR_GEU;
          default: ctl = '0;
        endcase
      end
      7'b1101111: begin // JAL
        ctl.imm_kind = IMM_J; ctl.is_jal = 1'b1; ctl.reg_write = 1'b1;
        ctl.wb_sel = WB_PC4; ctl.legal = 1'b1;
      end
      7'b1100111: begin // JALR
        if (funct3 == 3'b000) begin
          ctl.uses_rs1 = 1'b1; ctl.imm_kind = IMM_I; ctl.is_jalr = 1'b1;
          ctl.reg_write = 1'b1; ctl.wb_sel = WB_PC4; ctl.legal = 1'b1;
        end
      end
      7'b0110111: begin // LUI
        ctl.imm_kind = IMM_U; ctl.upper_lui = 1'b1; ctl.reg_write = 1'b1;
        ctl.wb_sel = WB_UPPER; ctl.legal = 1'b1;
      end
      7'b0010111: begin // AUIPC
        ctl.imm_kind = IMM_U; ctl.upper_auipc = 1'b1; ctl.reg_write = 1'b1;
        ctl.wb_sel = WB_UPPER; ctl.legal = 1'b1;
      end
      default: ctl = '0;
    endcase
  end
  assign ctl_bits = ctl;
endmodule
