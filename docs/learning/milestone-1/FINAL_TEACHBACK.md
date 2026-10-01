# Final teach-back — ProofCore Milestone 1

Answer these in your own words, ideally without opening the implementation. Every item starts as **UNASSESSED**. Reading the answer keys elsewhere does not count as a human answer.

## Beginner

1. **UNASSESSED** — What does IF do?
2. **UNASSESSED** — What does ID do?
3. **UNASSESSED** — What does EX do?
4. **UNASSESSED** — What happens in MEM?
5. **UNASSESSED** — What happens in WB?
6. **UNASSESSED** — What does a 32-bit instruction have to do with the PC adding four?
7. **UNASSESSED** — Why does x0 always read zero?
8. **UNASSESSED** — What information does the PC identify?

## Intermediate

9. **UNASSESSED** — Why can a nearby ADD usually be handled by forwarding?
10. **UNASSESSED** — Why does EX/MEM win over MEM/WB when both match?
11. **UNASSESSED** — Why is an EX/MEM load's ALU result not the value to forward?
12. **UNASSESSED** — What exact stage/register conditions cause a load-use stall?
13. **UNASSESSED** — Which stage registers hold during a load-use stall, and which advance?
14. **UNASSESSED** — What is the difference between a bubble and a valid NOP?
15. **UNASSESSED** — How does JALR form its target and return address?
16. **UNASSESSED** — How do LB and LBU differ?

## Advanced

17. **UNASSESSED** — Why can a load-use dependency require a bubble in this five-stage design?
18. **UNASSESSED** — How does `uses_rs2` prevent a false hazard on an immediate instruction?
19. **UNASSESSED** — Give an example where signed and unsigned comparisons disagree.
20. **UNASSESSED** — Which younger instructions are invalidated when an EX branch is taken?
21. **UNASSESSED** — Why does a branch target use the branch instruction's PC?
22. **UNASSESSED** — How do the B-immediate bits encode a negative offset?
23. **UNASSESSED** — Why does the memory interface receive an aligned word address for subword accesses?
24. **UNASSESSED** — What does the retirement record expose that a final register dump cannot?

## Debugging

25. **UNASSESSED** — A dependent ADD gets an old value. Which forwarding conditions and mux input should you inspect?
26. **UNASSESSED** — A result after LW equals the effective address. What likely happened?
27. **UNASSESSED** — A branch is taken but a following store changes memory. Which valid bits and enables would you inspect?
28. **UNASSESSED** — A loop jumps to the wrong address only for negative branches. Which immediate fields should be checked?
29. **UNASSESSED** — A byte store changes a neighboring lane. What mask and shift signals are relevant?
30. **UNASSESSED** — A bubble writes a register. Which valid qualification is missing?
31. **UNASSESSED** — A program writes x0 and later behavior changes. Which register-file and forwarding guards are suspect?
32. **UNASSESSED** — Why should you locate the first retirement mismatch instead of only inspecting the final wrong result?

## SystemVerilog

33. **UNASSESSED** — Is `logic [31:0]` signed by default?
34. **UNASSESSED** — Why is `$signed(a) < $signed(b)` used for SLT?
35. **UNASSESSED** — What hardware can an incomplete `always_comb` assignment imply?
36. **UNASSESSED** — Why are nonblocking assignments used in clocked blocks?
37. **UNASSESSED** — What does a packed struct provide to the RTL author?
38. **UNASSESSED** — Why does SRA use `>>>` and a signed left operand?
39. **UNASSESSED** — What is the purpose of the default no-effect decode control?
40. **UNASSESSED** — Why are register indices five bits wide?

## Synthesis

41. **UNASSESSED** — What does an `always_ff @(posedge clk)` block roughly become?
42. **UNASSESSED** — What hardware does an operand forwarding selector roughly become?
43. **UNASSESSED** — Why is the register-file reset policy relevant to synthesis?
44. **UNASSESSED** — What does passing Yosys `check` mean, and what does it not mean?
45. **UNASSESSED** — Why is a generic cell count not a physical area result?
46. **UNASSESSED** — What does a byte write mask control in a memory system?
47. **UNASSESSED** — What accommodation was made for the installed Yosys frontend?
48. **UNASSESSED** — Which important physical-design and formal tools are outside this milestone?
