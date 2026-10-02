package proofcore_pkg;
  typedef enum logic [3:0] {
    ALU_ADD, ALU_SUB, ALU_SLL, ALU_SLT, ALU_SLTU, ALU_XOR,
    ALU_SRL, ALU_SRA, ALU_OR, ALU_AND
  } alu_op_e;

  typedef enum logic [2:0] { IMM_I, IMM_S, IMM_B, IMM_U, IMM_J } imm_kind_e;
  typedef enum logic [1:0] { SIZE_BYTE, SIZE_HALF, SIZE_WORD } mem_size_e;
  typedef enum logic [2:0] { BR_NONE, BR_EQ, BR_NE, BR_LT, BR_GE, BR_LTU, BR_GEU } branch_e;
  typedef enum logic [1:0] { WB_ALU, WB_LOAD, WB_PC4, WB_UPPER } wb_sel_e;
  typedef enum logic [1:0] { FWD_RF, FWD_EXMEM, FWD_MEMWB } fwd_sel_e;

  typedef struct packed {
    logic       legal;
    logic       uses_rs1;
    logic       uses_rs2;
    logic       reg_write;
    logic       mem_read;
    logic       mem_write;
    logic       load_unsigned;
    logic       use_imm;
    logic       upper_lui;
    logic       upper_auipc;
    logic       is_jal;
    logic       is_jalr;
    alu_op_e    alu_op;
    imm_kind_e  imm_kind;
    mem_size_e  mem_size;
    branch_e    branch;
    wb_sel_e    wb_sel;
  } decode_t;

  typedef struct packed {
    logic [31:0] pc;
    logic [31:0] insn;
  } ifid_t;

  typedef struct packed {
    logic [31:0] pc;
    logic [31:0] pc4;
    logic [31:0] insn;
    logic [4:0]  rs1;
    logic [4:0]  rs2;
    logic [4:0]  rd;
    logic [31:0] rs1_value;
    logic [31:0] rs2_value;
    logic [31:0] imm;
    decode_t     ctl;
  } idex_t;

  typedef struct packed {
    logic [31:0] pc;
    logic [31:0] pc4;
    logic [31:0] insn;
    logic [4:0]  rd;
    logic        reg_write;
    logic [31:0] alu_result;
    logic [31:0] effective_addr;
    logic        mem_read;
    logic        mem_write;
    logic        load_unsigned;
    mem_size_e   mem_size;
    wb_sel_e     wb_sel;
    logic [31:0] next_pc;
    logic [3:0]  mem_read_mask;
    logic [3:0]  mem_write_mask;
    logic [31:0] mem_write_data;
`ifdef PROOFCORE_RVFI
    logic        uses_rs1;
    logic        uses_rs2;
    logic [31:0] rs1_value;
    logic [31:0] rs2_value;
`endif
  } exmem_t;

  typedef struct packed {
    logic [31:0] pc;
    logic [31:0] insn;
    logic [4:0]  rd;
    logic        reg_write;
    logic [31:0] rd_value;
    logic [31:0] mem_addr;
    logic [3:0]  mem_read_mask;
    logic [3:0]  mem_write_mask;
    logic [31:0] mem_write_data;
    logic [31:0] next_pc;
`ifdef PROOFCORE_RVFI
    logic        uses_rs1;
    logic        uses_rs2;
    logic [31:0] rs1_value;
    logic [31:0] rs2_value;
    logic [31:0] mem_read_data;
`endif
  } memwb_t;

  typedef struct packed {
    logic        valid;
    logic [31:0] pc;
    logic [31:0] insn;
    logic [4:0]  rd;
    logic        rd_write;
    logic [31:0] rd_value;
    logic [31:0] mem_addr;
    logic [3:0]  mem_read_mask;
    logic [3:0]  mem_write_mask;
    logic [31:0] mem_write_data;
    logic [31:0] next_pc;
  } retire_t;
endpackage
