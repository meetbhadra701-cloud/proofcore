module formal_memory (
  input  logic        clk,
  input  logic        reset,
  input  logic [31:0] imem_addr,
  output logic [31:0] imem_rdata,
  input  logic [31:0] dmem_addr,
  input  logic [3:0]  dmem_write_mask,
  input  logic [31:0] dmem_write_data,
  output logic [31:0] dmem_rdata
);
  localparam int WORDS = 8;
  logic [31:0] instruction_memory [0:WORDS-1];
  logic [31:0] data_memory [0:WORDS-1];

  for (genvar word_index = 0; word_index < WORDS; word_index++) begin : g_symbolic_memories
    (* anyconst *) logic [31:0] instruction_word;
    (* anyconst *) logic [31:0] initial_data_word;
    always_comb instruction_memory[word_index] = instruction_word;
    always_ff @(posedge clk) begin
      if (reset) data_memory[word_index] <= initial_data_word;
      else if (dmem_addr[4:2] == word_index[2:0]) begin
        for (int lane = 0; lane < 4; lane++) begin
          if (dmem_write_mask[lane])
            data_memory[word_index][lane*8 +: 8] <= dmem_write_data[lane*8 +: 8];
        end
      end
    end
  end

  always_comb begin
    imem_rdata = instruction_memory[imem_addr[4:2]];
    dmem_rdata = data_memory[dmem_addr[4:2]];
  end
endmodule
