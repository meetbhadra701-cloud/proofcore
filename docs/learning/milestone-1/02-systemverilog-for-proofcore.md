# 02 — SystemVerilog used in ProofCore

## What is it?

SystemVerilog describes hardware. A variable in this language usually becomes a wire, a flip-flop, or a group of logic cells. It is not a software program that runs one statement at a time on a processor.

## Why does ProofCore need it?

We use SystemVerilog to describe the datapath and control: values moving through the pipeline, decisions about instructions, and the registers that remember values between clock edges.

## Mental model

`always_comb` describes logic that continuously reacts to inputs, like a group of connected gates. `always_ff @(posedge clk)` describes storage that changes at a clock edge. In sequential blocks, `<=` schedules all selected registers to update together after the edge. That models flip-flops.

## Actual implementation

`logic` declares a one-bit or vector signal. `logic [31:0]` is 32 bits wide. Enums name small control values, such as the ALU operation. Packed structs group related bits, for example all the fields in ID/EX. These types are in [rtl/proofcore_pkg.sv](../../../rtl/proofcore_pkg.sv).

Some module ports are packed vectors instead of typed structs because the installed Yosys frontend accepts package-defined structs internally but does not accept package imports and typed struct ports. The core still uses named structs internally, and Verilator checks widths at the connections.

## Important code idea

In [rtl/alu.sv](../../../rtl/alu.sv), `always_comb` assigns a default result and then chooses one result in a `case`. A default matters: if a branch forgets to assign an output, hardware may need to remember its old value, which creates an unintended latch.

For signed comparison, the code writes `$signed(a) < $signed(b)`. A plain `logic [31:0]` is a bit pattern; it does not automatically mean a signed two's-complement number.

## Concrete example

The bit pattern `0xFFFFFFFF` is 4,294,967,295 if unsigned and -1 if signed. Comparing it with `1` gives opposite orderings. The ALU and branch logic explicitly cast values when a signed comparison is required.

## What could go wrong?

If the left shift amount uses all 32 bits, a large value can select an unintended shift. RV32I uses only five shift bits, `[4:0]`. If a signed value is first widened as unsigned, the upper bits become zeros instead of copies of the sign bit.

## How our tests catch it

Boundary values include `0x80000000`, `0xFFFFFFFF`, and shifts by 0 and 31. Verilator lint also rejects inferred latches and unexplained width/signedness problems under `-Wall`.

## Hardware interpretation

An enum is a small binary encoding. A packed struct is just adjacent bits with names. A `case` in combinational logic usually becomes a mux and decode gates. An `always_ff` variable becomes flip-flops.

## Interview relevance

Expect questions about blocking versus nonblocking assignment, latch inference, signed vectors, and what `always_comb` promises to tools.

## Exercises

1. Is `logic [31:0] a` signed or unsigned by default?
2. What hardware can an incomplete `always_comb` assignment infer?

## Answers

<details><summary>Check your reasoning</summary>

It is unsigned unless explicitly cast or declared signed. An incomplete assignment can require a latch to preserve the previous value.

</details>
