# Milestone 2 Teach-Back — UNASSESSED

Answer in your own words without using the implementation as a script. Every question stays **UNASSESSED** until you answer it; reading the learning pack is not an assessment.

## Beginner

1. **UNASSESSED** — What architectural state does the C++ model keep?
2. **UNASSESSED** — What does one call to `step()` do?
3. **UNASSESSED** — What does a retirement event identify?
4. **UNASSESSED** — Why does x0 remain zero?
5. **UNASSESSED** — What does the `next_pc` field mean?
6. **UNASSESSED** — What is a byte-addressed memory?
7. **UNASSESSED** — What does little-endian mean for the bytes `11 22 33 44`?

## Intermediate

8. **UNASSESSED** — Why does the reference model not represent IF/ID or ID/EX?
9. **UNASSESSED** — Why must the reference advance once per retirement rather than once per clock?
10. **UNASSESSED** — Which fields are compared when an instruction does not write a register?
11. **UNASSESSED** — Why does the checker ignore destination value when `rd_write` is false?
12. **UNASSESSED** — How do separate RTL and reference memories prevent duplicate-store masking?
13. **UNASSESSED** — How does a store's byte mask select memory lanes?
14. **UNASSESSED** — What does the load-use stall do to the RTL timeline while the reference waits?
15. **UNASSESSED** — Why does JAL need both a link result and a different next PC?

## Advanced

16. **UNASSESSED** — Why can independent code still share an ISA encoding assumption?
17. **UNASSESSED** — How does `uint32_t` addition implement RV32I wraparound without signed-overflow undefined behavior?
18. **UNASSESSED** — How does the model express signed SLT and unsigned SLTU over the same bits?
19. **UNASSESSED** — Why is the arithmetic shift amount masked to five bits?
20. **UNASSESSED** — How are B-immediate bits rearranged and sign-extended?
21. **UNASSESSED** — Why does JALR clear bit zero and then still need M1 target-alignment policy?
22. **UNASSESSED** — Why compare both store retirement metadata and final memory bytes?
23. **UNASSESSED** — What shared bug could remain even when 500 differential programs pass?

## Debugging

24. **UNASSESSED** — A mismatch has the same PC and instruction but wrong `rd_value`. Which evidence should you inspect next?
25. **UNASSESSED** — A mismatch is in `next_pc`. Which instruction families and decode fields are likely relevant?
26. **UNASSESSED** — How can a wrong-path instruction show up in the retirement stream?
27. **UNASSESSED** — What facts are stored in a failure artifact?
28. **UNASSESSED** — How do you replay the exact case rather than merely rerunning the same campaign size?
29. **UNASSESSED** — What does `TIMEOUT` mean in this harness?
30. **UNASSESSED** — Why is the self-test artifact not evidence that the processor has a hardware defect?

## C++ and decoding

31. **UNASSESSED** — Why is a signed C++ addition a poor way to model RV32I overflow?
32. **UNASSESSED** — What does `std::bit_cast<int32_t>` express in the signed comparator?
33. **UNASSESSED** — Why must shift-immediate upper bits be checked?
34. **UNASSESSED** — What happens to an unsupported encoding under the frozen M1 contract?
35. **UNASSESSED** — How does sign extension differ between LB and LBU?
36. **UNASSESSED** — What does the final `enforce_x0()` protect?

## Verilator and verification

37. **UNASSESSED** — What does Verilator generate for the harness to instantiate?
38. **UNASSESSED** — In what order does the driver supply memory, observe retirement, and clock the DUT?
39. **UNASSESSED** — Why does the test have both an instruction limit and a cycle limit?
40. **UNASSESSED** — What exact field does the comparator self-test alter, and where?
41. **UNASSESSED** — Why does `make verify` keep the M1 regression in addition to M2 tests?
42. **UNASSESSED** — Why is differential testing not a formal proof?

## Randomness and reproducibility

43. **UNASSESSED** — How does the generator make `--seed 12345` repeatable?
44. **UNASSESSED** — Why are generated backward loops initialized with a bounded counter?
45. **UNASSESSED** — What does the corpus manifest tell you beyond the base seed?
46. **UNASSESSED** — How would you add a random test tier without making it nondeterministic?
47. **UNASSESSED** — What should happen if a generated program never reaches its stop PC?

## Synthesis boundary

48. **UNASSESSED** — Which parts of M2 synthesize into ProofCore hardware, and which are host-side verification software?
49. **UNASSESSED** — What do M1 Verilator lint and Yosys synthesis checks say, and what do they not say?
50. **UNASSESSED** — Name one later milestone feature that M2 deliberately does not implement.
