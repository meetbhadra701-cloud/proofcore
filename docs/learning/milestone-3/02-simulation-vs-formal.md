# 02 — Simulation versus formal verification

Simulation executes a finite test program with particular inputs. Formal
verification represents inputs symbolically and asks whether any allowed
choice can violate a property. Neither method makes a poorly modeled design
correct: both depend on the environment and the rule being meaningful.

M2's 500 deterministic programs are valuable because they model complete
architectural execution, real memory updates, and long combinations of
instructions. M3's per-instruction proofs systematically explore operands and
pipeline states within a much smaller bounded window. Simulation often gives
an intuitive program-level story; formal often finds corner cases beyond the
chosen corpus.

For example, M2 can run `addi x5,x0,7; add x6,x5,x5` and compare the final
retirements. The formal check can explore many values for x5 and the consumer,
while internal assertions examine the forwarding condition. Formal's small
memory and depth limits mean it does not replace the M2 long-run campaign.

The M3 cross-check is empirical too: `make diff-rvfi-directed` and
`make diff-rvfi-random` compare every relevant RVFI field with the independent
reference model. This catches instrumentation bugs before trusting RVFI as the
formal boundary.

The hardware is unchanged by either verification method. A testbench and solver
are host-side tools; synthesis still sees the synthesizable RTL, not the proof
engine.

**Failure example:** A directed test may never choose operands where signed and
unsigned comparisons disagree. A symbolic instruction check can choose
`0x80000000` and `1` and expose a signedness error.

**Interview question:** What classes of bugs are easier to expose with formal
analysis, and what does simulation still contribute?

**Exercise:** Name one M1 behavior that the 32-byte formal harness cannot
exercise but the 500-program M2 suite can.
