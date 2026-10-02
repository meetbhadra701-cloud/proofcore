# 20 — Formal verification interview guide

The answers describe only behavior demonstrated by ProofCore M3. They do not
claim full ISA, trap, or unbounded proof coverage.

## Apple formal / CAD / silicon

**Q: How is formal verification different from simulation?**

**A:** Simulation executes selected tests. Formal represents choices
symbolically and searches all traces in the modeled scope for an assertion
violation. ProofCore keeps simulation because formal depth and memory are
bounded, and because a full program-level differential trace is valuable.

**Q: What would you inspect first if a formal property passes immediately?**

**A:** Check that the clock advances, reset exits, the assertion is elaborated,
its antecedent can occur, and no assumption removes the behavior. Pair the
assertion with a cover and inspect the SBY log.

## NVIDIA hardware verification

**Q: Why carry forwarded values into the RVFI record?**

**A:** RVFI source data describes architectural values consumed by the
instruction. A raw ID-stage register read can be stale. ProofCore carries the
forwarded EX operands with that instruction and reports them at retirement.

**Q: What do you do with a counterexample?**

**A:** Identify the failing property, read reset/clock and relevant inputs in
the waveform, map them to the RTL condition, reproduce in simulation if useful,
then classify it as RTL, property, harness, or contract failure before fixing.

## Qualcomm design verification / CAD

**Q: Why is an assumption audit part of the result?**

**A:** Assumptions reduce the set of executions. A proof can become too weak if
the assumption excludes the bug. Each ProofCore assumption states why needed,
what it excludes, spec alignment, and possible blind spots.

**Q: What is vacuity?**

**A:** A property passes because its trigger never occurs. ProofCore pairs key
pipeline properties with covers for forwarding, stalls, redirects, memory
operations, and retirement recovery.

## Amazon Annapurna ASIC

**Q: What structures synthesize from forwarding logic?**

**A:** Destination/source equality comparators and valid/write qualification
feed a priority-control network that selects among register-file, EX/MEM, and
MEM/WB values for each ALU operand.

**Q: How do you ensure a wrong-path store cannot commit?**

**A:** A taken EX redirect flushes younger IF/ID and ID/EX validity. Memory
write masks also require a valid store in EX/MEM. Assertions check both the
flush state and valid-gated side-effect condition; covers show redirects happen.

## AMD design verification

**Q: Why does load-use usually need a bubble?**

**A:** The load's value arrives from MEM after its consumer would need it at EX.
Holding PC and IF/ID while inserting an invalid ID/EX entry lets the load
advance; MEM/WB forwarding then supplies the value.

**Q: What does depth-16 BMC PASS mean?**

**A:** No counterexample was found in the first 16 modeled steps under the
recorded assumptions. It does not prove behavior at all future depths. An
inductive proof is needed for an unbounded claim.

## Whiteboard exercises with answer outlines

**Design a forwarding unit.** Compare each valid producer rd to each used
consumer rs, exclude x0 and unavailable loads, choose EX/MEM before MEM/WB. The
result is two independent selector outputs.

**Detect load-use.** Stall only when valid ID/EX is a load with nonzero rd and a
valid IF/ID instruction actually uses a matching rs1 or rs2.

**Draw an ADD dependency.** For `addi x5,x0,7; add x6,x5,x5`, the ADD reaches EX
while x5 is being produced. Both operand muxes select 7; the result is 14.

**Explain a wrong-path commit.** If the flush fails to clear younger valid
state, the younger store can reach EX/MEM and assert a mask. Inspect redirect,
IF/ID/ID/EX valid bits, EX/MEM valid/write controls, and retirement.

**Find a signedness bug.** Compare `0x80000000` against `1`: signed less-than is
true; unsigned less-than is false. Use an explicit signed cast only for signed
operations.

**Explain setup/hold conceptually.** Setup is input stability before a sampling
edge; hold is stability after it. ProofCore M3 checks logical cycle behavior,
not static timing or setup/hold closure.

**Explain a negative-control run.** The formal build defines an intentional
corruption only for the self-test, expects the solver to report FAIL, preserves
the trace, and reports the self-test as passed only when that expected failure
is observed.
