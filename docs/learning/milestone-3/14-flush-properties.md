# 14 — Redirects and flush properties

Branches, JAL, and JALR resolve in EX. When a transfer is taken, the core
changes the fetch PC and invalidates younger IF/ID and ID/EX entries. The
redirecting instruction remains valid and later retires with its correct next
PC. Invalidating validity is what prevents a wrong-path store or register write
from escaping.

For the taken sequence:

```asm
beq x1, x2, target
sw  x5, 0(x10)   # wrong path when x1 == x2
target:
addi x6, x0, 9
```

The store may already have been fetched and decoded when BEQ resolves. The
flush removes it before it can become a valid EX/MEM store. The property checks
that after `redirect`, PC equals the target, IF/ID and ID/EX are invalid, and
EX/MEM still contains the redirecting instruction. Other invariants require
memory writes and retirements to be valid-gated.

Covers reach taken branch, not-taken branch, JAL, JALR, and retirement after a
redirect. Upstream PC checks validate the retired next-PC relationships for
supported aligned cases.

**Failure example:** Clearing instruction bits to zero without clearing the
valid bit could allow an accidental operation to retire. Conversely, clearing
valid too early could discard the branch itself. The state checks protect both
younger flushing and preservation of the redirecting instruction.

**Hardware view:** Redirect target selection is a PC mux; flush is control into
pipeline register valid inputs. There is no prediction or speculative
architectural commit in this machine.

**Interview question:** Which younger stages must be flushed when a branch
resolves in EX, and why?

**Exercise:** Add a wrong-path `addi x7,x0,99` before the target and state what
the retirement stream must contain.
