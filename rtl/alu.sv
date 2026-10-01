module alu (
  input  logic [31:0] a,
  input  logic [31:0] b,
  input  logic [3:0] op,
  output logic [31:0] result
);
  localparam logic [3:0] ALU_ADD = proofcore_pkg::ALU_ADD;
  localparam logic [3:0] ALU_SUB = proofcore_pkg::ALU_SUB;
  localparam logic [3:0] ALU_SLL = proofcore_pkg::ALU_SLL;
  localparam logic [3:0] ALU_SLT = proofcore_pkg::ALU_SLT;
  localparam logic [3:0] ALU_SLTU = proofcore_pkg::ALU_SLTU;
  localparam logic [3:0] ALU_XOR = proofcore_pkg::ALU_XOR;
  localparam logic [3:0] ALU_SRL = proofcore_pkg::ALU_SRL;
  localparam logic [3:0] ALU_SRA = proofcore_pkg::ALU_SRA;
  localparam logic [3:0] ALU_OR = proofcore_pkg::ALU_OR;
  localparam logic [3:0] ALU_AND = proofcore_pkg::ALU_AND;
  always_comb begin
    result = 32'b0;
    unique case (op)
      ALU_ADD:  result = a + b;
      ALU_SUB:  result = a - b;
      ALU_SLL:  result = a << b[4:0];
      ALU_SLT:  result = {31'b0, ($signed(a) < $signed(b))};
      ALU_SLTU: result = {31'b0, (a < b)};
      ALU_XOR:  result = a ^ b;
      ALU_SRL:  result = a >> b[4:0];
      ALU_SRA:  result = $signed(a) >>> b[4:0];
      ALU_OR:   result = a | b;
      ALU_AND:  result = a & b;
      default:  result = 32'b0;
    endcase
  end
endmodule
