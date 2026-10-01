# Program inventory

The regression builds instruction words with the format encoders in `tb/cocotb/test_core.py`, then serves them through the combinational instruction-memory port. This keeps the tests independent of a cross-assembler while retaining the exact bit patterns under test.

The regression runs nine directed programs:

1. All register and immediate ALU operations, signed/unsigned disagreement, and edge shifts.
2. EX/MEM priority over MEM/WB, two-source forwarding, a dependency chain, and an attempted x0 write.
3. Load-use on both ALU operands, load-to-immediate use, and the x0 false dependency.
4. Taken control flow, wrong-path register/store suppression, JAL, and odd-address JALR bit-zero clearing.
5. Both taken and not-taken outcomes for every branch predicate.
6. A backward B-immediate loop using a negative offset.
7. A load-dependent branch with one interlock and redirect flush.
8. LB/LBU/LH/LHU/LW, SB/SH/SW, lane masks, and negative S-immediate.
9. Unsupported encoding and misaligned load/store/target no-effect behavior.
