# 12 — Forwarding properties

Forwarding solves a dependency before the producer reaches the register file.
The consumer's source index is compared against valid older destinations. If
EX/MEM matches and its result is ready, that source wins. Otherwise a matching
MEM/WB value may be selected. A destination of x0 is never a producer.

Consider:

```asm
addi x5, x0, 7
add  x6, x5, x5
```

The ADD needs 7 on both inputs although ID may have captured an older x5 value.
When ADD reaches EX, the preceding ADDI is the newer eligible EX/MEM producer.
Both operand muxes select 7, so x6 becomes 14. A formal property checks that
when `fwd_rs1_sel == FWD_EXMEM`, `ex_rs1 == exmem_q.alu_result`; the analogous
property is checked for rs2 and MEM/WB. A separate condition checks priority
when both stages match the same nonzero destination.

Loads are deliberately excluded from EX/MEM value forwarding because their data
is not ready there. The load-use property and stall allow the result to become
available through MEM/WB.

The covers demonstrate EX/MEM forwarding, MEM/WB forwarding, and a priority
conflict are reachable in symbolic instruction memory. The RVFI differential
also verifies that the value eventually reported as the consumer's source is
the architectural one.

**Failure example:** If MEM/WB wins over a newer EX/MEM write to the same `rd`,
the consumer gets an older value. If an x0 write is forwarded, a false
dependency can turn architectural zero into arbitrary data.

**Hardware view:** Register-index comparators feed a priority encoder/control
unit, which drives two operand muxes.

**Interview question:** Why does EX/MEM have priority when both producer stages
match?

**Exercise:** For `addi x1,x0,1; addi x1,x1,1; add x2,x1,x1`, state which value
the final ADD must receive and from which stage.
