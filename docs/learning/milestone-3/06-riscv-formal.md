# 06 — The pinned `riscv-formal` framework

The upstream framework contains independent instruction models and checkers
that consume an RVFI interface. ProofCore pins the repository at
`c992aa61fdfe0846c5ed90324c596202a1c69b76` and runs 37 generated checks for
the instructions implemented in M1. It also runs applicable register, forward
PC, backward PC, and retirement-order consistency checks.

The configuration is `formal/rvfi/checks.cfg`. The staging script copies the
pinned source to ignored `build/formal/riscv-formal/`, installs ProofCore's
wrapper and config, generates checks, and executes each check. Results are
recorded one by one in `formal/results/results.json`; logs are under
`build/formal/`.

Each instruction check specializes the checker to one opcode/funct pattern,
then compares destination, next PC, sources, memory effects, and trap expectation
against the upstream model. It is bounded BMC at depth 12 with ABC `bmc3`, not
an inductive proof. The scope excludes SYSTEM, CSR, privilege, compressed,
fault, and unrelated instructions.

M1 does not implement misalignment traps. The wrapper therefore restricts
control-flow destinations and halfword/word accesses to naturally aligned
cases. This was discovered by the first control-flow run: upstream correctly
expected a trap for a target M1 documents as an inert transfer. The RTL was
kept unchanged and the formal scope was documented instead.

**Hardware view:** The instruction model is verification logic; it is not
synthesized into ProofCore. The core is checked against this independent model.

**Interview question:** Why is it inaccurate to say “full RV32I verified” for
this campaign?

**Exercise:** Find one instruction in the M1 table and locate its generated
check name under the staged formal build.
