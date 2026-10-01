module regfile (
  input  logic        clk,
  input  logic        reset,
  input  logic        we,
  input  logic [4:0]  waddr,
  input  logic [31:0] wdata,
  input  logic [4:0]  raddr1,
  input  logic [4:0]  raddr2,
  output logic [31:0] rdata1,
  output logic [31:0] rdata2
);
  logic [31:0] regs [1:31];
  integer i;

  always_ff @(posedge clk) begin
    if (reset) begin
      for (i = 1; i < 32; i = i + 1) regs[i] <= 32'b0;
    end else if (we && (waddr != 5'd0)) begin
      regs[waddr] <= wdata;
    end
  end

  always_comb begin
    rdata1 = 32'b0;
    rdata2 = 32'b0;
    if (raddr1 != 5'd0) begin
      rdata1 = regs[raddr1];
      if (we && (waddr == raddr1)) rdata1 = wdata;
    end
    if (raddr2 != 5'd0) begin
      rdata2 = regs[raddr2];
      if (we && (waddr == raddr2)) rdata2 = wdata;
    end
  end
endmodule
