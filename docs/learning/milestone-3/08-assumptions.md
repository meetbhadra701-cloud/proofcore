# 08 — Assumptions and proof scope

An assumption describes which environment behaviors are allowed. It is not a
shortcut for making a DUT rule true. Every assumption removes possible traces,
so the engineer must say why it is needed and what bug could disappear with it.

ProofCore's custom assumptions bound instruction and data addresses to the
32-byte memories. The ISA wrapper also excludes misaligned transfers and
halfword/word accesses because M1 has no trap behavior. The memory model itself
uses unconstrained initial words and persistent stores. Reset is initialized
and deasserted using the synchronous M1 protocol. Upstream per-instruction
checkers select the instruction being verified with their own `spec_valid`
assumption.

No assumption says an ALU result is right, that a branch is taken, that a
forwarding selector is correct, or that retirement data matches the model. The
alignment assumptions are derived using RVFI instruction/source values; that
does narrow traces and is called out as a blind spot in the audit.

Use `formal/ASSUMPTIONS.md` as the canonical inventory and
`docs/formal/assumption-audit.md` for the review. A future expansion should
remove restrictions only after the environment can model the newly included
behavior, especially trap semantics and larger memory.

**Failure example:** An assumption that forces all destinations to be aligned
could hide a bug in JALR bit clearing. Here the assumption is necessary because
the core explicitly lacks the trap behavior for an unaligned destination, but
that is a reason to keep the limitation visible—not to call the result complete
RISC-V compliance.

**Interview question:** How can an assumption make a proof vacuous or too weak?

**Exercise:** Choose one assumption in the table and name a realistic RTL bug it
could exclude.
