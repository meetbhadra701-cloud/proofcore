module imm_gen (
  input  logic [31:7]  insn,
  input  logic [2:0] kind,
  output logic [31:0]  imm
);
  localparam logic [2:0] IMM_I = proofcore_pkg::IMM_I;
  localparam logic [2:0] IMM_S = proofcore_pkg::IMM_S;
  localparam logic [2:0] IMM_B = proofcore_pkg::IMM_B;
  localparam logic [2:0] IMM_U = proofcore_pkg::IMM_U;
  localparam logic [2:0] IMM_J = proofcore_pkg::IMM_J;
  always_comb begin
    unique case (kind)
      IMM_I: imm = {{20{insn[31]}}, insn[31:20]};
      IMM_S: imm = {{20{insn[31]}}, insn[31:25], insn[11:7]};
      IMM_B: imm = {{19{insn[31]}}, insn[31], insn[7], insn[30:25], insn[11:8], 1'b0};
      IMM_U: imm = {insn[31:12], 12'b0};
      IMM_J: imm = {{11{insn[31]}}, insn[31], insn[19:12], insn[20], insn[30:21], 1'b0};
      default: imm = 32'b0;
    endcase
  end
endmodule
