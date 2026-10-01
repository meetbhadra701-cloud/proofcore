module forwarding (
  input  logic       exmem_valid,
  input  logic       exmem_reg_write,
  input  logic       exmem_is_load,
  input  logic [4:0] exmem_rd,
  input  logic       memwb_valid,
  input  logic       memwb_reg_write,
  input  logic [4:0] memwb_rd,
  input  logic [4:0] idex_rs1,
  input  logic [4:0] idex_rs2,
  output logic [1:0] rs1_sel,
  output logic [1:0] rs2_sel
);
  localparam logic [1:0] FWD_RF = proofcore_pkg::FWD_RF;
  localparam logic [1:0] FWD_EXMEM = proofcore_pkg::FWD_EXMEM;
  localparam logic [1:0] FWD_MEMWB = proofcore_pkg::FWD_MEMWB;
  always_comb begin
    rs1_sel = FWD_RF;
    rs2_sel = FWD_RF;
    if (exmem_valid && exmem_reg_write && !exmem_is_load && (exmem_rd != 5'd0)) begin
      if (exmem_rd == idex_rs1) rs1_sel = FWD_EXMEM;
      if (exmem_rd == idex_rs2) rs2_sel = FWD_EXMEM;
    end
    if (memwb_valid && memwb_reg_write && (memwb_rd != 5'd0)) begin
      if ((rs1_sel == FWD_RF) && (memwb_rd == idex_rs1)) rs1_sel = FWD_MEMWB;
      if ((rs2_sel == FWD_RF) && (memwb_rd == idex_rs2)) rs2_sel = FWD_MEMWB;
    end
  end
endmodule
