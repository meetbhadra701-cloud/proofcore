# Final Milestone 3 teach-back

Every question is **UNASSESSED**. The human should answer without treating the
learning pack as proof of understanding. Discuss answers with an engineer and
change a status only after the human has provided their own explanation.

## Beginner

1. **UNASSESSED** — What does a processor retirement event represent?
2. **UNASSESSED** — What is a symbolic value?
3. **UNASSESSED** — What is an assertion?
4. **UNASSESSED** — What is an assumption?
5. **UNASSESSED** — What is a cover?
6. **UNASSESSED** — Why does x0 always read zero?
7. **UNASSESSED** — What is a counterexample?
8. **UNASSESSED** — What does RVFI expose?
9. **UNASSESSED** — What is the difference between a pipeline clock cycle and an instruction retirement?
10. **UNASSESSED** — What does a cover trace show?

## Formal foundations

11. **UNASSESSED** — How is formal search different from running one simulation test?
12. **UNASSESSED** — What does SAT mean in a property check?
13. **UNASSESSED** — What role does bit-vector width play in a 32-bit addition?
14. **UNASSESSED** — What does bounded model checking explore?
15. **UNASSESSED** — What is an inductive proof?
16. **UNASSESSED** — Why is a timeout not a pass?
17. **UNASSESSED** — How can an overstrong assumption make a result meaningless?
18. **UNASSESSED** — Why should important properties have matching covers?
19. **UNASSESSED** — What is vacuity?
20. **UNASSESSED** — Why are assumptions part of a proof claim?

## RTL and pipeline properties

21. **UNASSESSED** — Why does EX/MEM forwarding have priority over MEM/WB?
22. **UNASSESSED** — Why must x0 be excluded as a forwarding producer?
23. **UNASSESSED** — Why can an ALU dependency forward without a stall?
24. **UNASSESSED** — Why does a load-use dependency need a bubble in this design?
25. **UNASSESSED** — Which state must hold during the load-use stall?
26. **UNASSESSED** — Which younger instructions are flushed on a taken EX redirect?
27. **UNASSESSED** — Why must pipeline stages carry explicit valid bits?
28. **UNASSESSED** — What condition authorizes a data-memory write?
29. **UNASSESSED** — How do a source/destination comparator and mux implement forwarding?
30. **UNASSESSED** — What is checked by the internal retirement-valid assertion?

## RVFI

31. **UNASSESSED** — Why is RVFI source data captured after forwarding?
32. **UNASSESSED** — What does `rvfi_order` count?
33. **UNASSESSED** — Why do bubbles not increment RVFI order?
34. **UNASSESSED** — Why is `rvfi_rd_addr` zero for an x0 write?
35. **UNASSESSED** — Why does RVFI use a byte mask for memory operations?
36. **UNASSESSED** — What does `rvfi_mem_rdata` represent for a byte load?
37. **UNASSESSED** — Why are unused source register addresses set to zero?
38. **UNASSESSED** — What do M-mode and RV32 encodings mean in this interface?
39. **UNASSESSED** — Why do zero trap outputs not prove trap compliance?
40. **UNASSESSED** — How does the M2 reference model help validate RVFI?

## Assumptions, cover, and scope

41. **UNASSESSED** — Why is formal memory limited to 32 bytes?
42. **UNASSESSED** — What behavior is excluded by stable symbolic instruction memory?
43. **UNASSESSED** — Why are misaligned control-flow targets excluded from upstream checks?
44. **UNASSESSED** — Which bugs could the alignment assumptions hide?
45. **UNASSESSED** — What does the upstream `assume(spec_valid)` select?
46. **UNASSESSED** — Why does the data memory preserve byte writes between reads?
47. **UNASSESSED** — What does a successful EX/MEM forwarding cover establish?
48. **UNASSESSED** — What does that cover not establish?
49. **UNASSESSED** — Why are internal covers separate from ISA instruction checks?
50. **UNASSESSED** — What is the exact formal result scope for ProofCore M3?

## SAT/SMT and debugging

51. **UNASSESSED** — How does a 32-bit bit-vector differ from an unbounded integer?
52. **UNASSESSED** — What is the difference between ABC `bmc3` and Z3 SMTBMC in this project?
53. **UNASSESSED** — Why did an undriven clock risk a vacuous PASS?
54. **UNASSESSED** — What should be inspected first in an assertion counterexample?
55. **UNASSESSED** — How can a failure belong to the harness rather than RTL?
56. **UNASSESSED** — Why preserve a useful waveform after fixing a formal issue?
57. **UNASSESSED** — How can an SBY `ERROR` differ from a property `FAIL`?
58. **UNASSESSED** — Why can a deeper bounded check be expensive?
59. **UNASSESSED** — How does a negative control test the verification flow?
60. **UNASSESSED** — What evidence is needed before labeling a failure an RTL bug?

## Interview scenarios

61. **UNASSESSED** — How would you formally check forwarding priority?
62. **UNASSESSED** — How would you prove a flushed store never changes memory?
63. **UNASSESSED** — What would you do if `reg_ch0` failed but directed simulation passed?
64. **UNASSESSED** — How could an assumption hide a wrong JALR implementation?
65. **UNASSESSED** — Why is M3 not “full RV32I formally verified”?
66. **UNASSESSED** — What should Milestone 4 do first with a preserved formal witness?
67. **UNASSESSED** — What does the x0 negative-control trace demonstrate?
68. **UNASSESSED** — Which part of M3 is AI-generated and what evidence was run?
69. **UNASSESSED** — What must remain unassessed until the human answers?
70. **UNASSESSED** — How would you explain the M3 result accurately to a verification interviewer?
