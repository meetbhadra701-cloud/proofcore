module hazard_unit (
  input  logic       ifid_valid,
  input  logic       ifid_uses_rs1,
  input  logic       ifid_uses_rs2,
  input  logic [4:0] ifid_rs1,
  input  logic [4:0] ifid_rs2,
  input  logic       idex_valid,
  input  logic       idex_is_load,
  input  logic [4:0] idex_rd,
  output logic       load_use_stall
);
  always_comb begin
    load_use_stall = 1'b0;
    if (ifid_valid && idex_valid && idex_is_load && (idex_rd != 5'd0)) begin
      load_use_stall = (ifid_uses_rs1 && (ifid_rs1 == idex_rd)) ||
                       (ifid_uses_rs2 && (ifid_rs2 == idex_rd));
    end
  end
endmodule
