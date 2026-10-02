module mem_wb (
  input  logic clk,
  input  logic reset,
  input  logic valid_in,
`ifdef PROOFCORE_RVFI
  input  logic [303:0] data_in,
`else
  input  logic [205:0] data_in,
`endif
  output logic valid_out,
`ifdef PROOFCORE_RVFI
  output logic [303:0] data_out
`else
  output logic [205:0] data_out
`endif
);
  always_ff @(posedge clk) begin
    if (reset) begin
      valid_out <= 1'b0;
      data_out <= '0;
    end else begin
      valid_out <= valid_in;
      data_out <= valid_in ? data_in : '0;
    end
  end
endmodule
