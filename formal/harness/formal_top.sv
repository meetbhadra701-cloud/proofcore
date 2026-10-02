module formal_top;
  logic clk = 1'b0;
  (* gclk *) reg formal_timestep;
  logic reset = 1'b1;
  wire [31:0] imem_addr;
  wire [31:0] imem_rdata;
  wire [31:0] dmem_addr;
  wire        dmem_read_enable;
  wire [31:0] dmem_rdata;
  wire [3:0]  dmem_write_mask;
  wire [31:0] dmem_write_data;
  wire        retire_valid;
  wire [31:0] retire_pc;
  wire [31:0] retire_insn;
  wire [4:0]  retire_rd;
  wire        retire_rd_write;
  wire [31:0] retire_rd_value;
  wire [31:0] retire_mem_addr;
  wire [3:0]  retire_mem_read_mask;
  wire [3:0]  retire_mem_write_mask;
  wire [31:0] retire_mem_write_data;
  wire [31:0] retire_next_pc;

  always_ff @(posedge formal_timestep) begin
    clk <= !clk;
    if (clk) reset <= 1'b0;
  end

  formal_memory u_memory (
    .clk(clk), .reset(reset),
    .imem_addr(imem_addr), .imem_rdata(imem_rdata),
    .dmem_addr(dmem_addr), .dmem_write_mask(dmem_write_mask),
    .dmem_write_data(dmem_write_data), .dmem_rdata(dmem_rdata)
  );

  core u_core (
    .clk(clk), .reset(reset),
    .imem_addr(imem_addr), .imem_rdata(imem_rdata),
    .dmem_addr(dmem_addr), .dmem_read_enable(dmem_read_enable), .dmem_rdata(dmem_rdata),
    .dmem_write_mask(dmem_write_mask), .dmem_write_data(dmem_write_data),
    .retire_valid(retire_valid), .retire_pc(retire_pc), .retire_insn(retire_insn),
    .retire_rd(retire_rd), .retire_rd_write(retire_rd_write),
    .retire_rd_value(retire_rd_value), .retire_mem_addr(retire_mem_addr),
    .retire_mem_read_mask(retire_mem_read_mask),
    .retire_mem_write_mask(retire_mem_write_mask),
    .retire_mem_write_data(retire_mem_write_data), .retire_next_pc(retire_next_pc)
  );

  always_comb if (!reset) begin
    assume(imem_addr[31:5] == 0);
    assume(dmem_addr[31:5] == 0);
  end
endmodule
