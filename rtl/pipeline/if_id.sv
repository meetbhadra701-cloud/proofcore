module if_id (
  input  logic clk,
  input  logic reset,
  input  logic hold,
  input  logic flush,
  input  logic valid_in,
  input  logic [63:0] data_in,
  output logic valid_out,
  output logic [63:0] data_out
);
  always_ff @(posedge clk) begin
    if (reset || flush) begin
      valid_out <= 1'b0;
      data_out <= '0;
    end else if (!hold) begin
      valid_out <= valid_in;
      data_out <= valid_in ? data_in : '0;
    end
  end
endmodule
