# RVFI and `riscv-formal` setup

The `PROOFCORE_RVFI` define adds an optional one-retirement-per-cycle RVFI
interface to `core`. Ordinary M1/M2 builds leave the define off. The interface
is sampled from the same MEM/WB event as the M1 retirement record; source values
are the forwarded operands carried to WB, and load data is the aligned data word
observed at the memory stage.

The pinned framework is `third_party/riscv-formal` at
`c992aa61fdfe0846c5ed90324c596202a1c69b76`. SymbiYosys is pinned at
`b1a1e98cba941ec8433f8dc27f416cd7bb7f14be`; Yosys is 0.52 in the authoring
environment. `checks.cfg` selects the framework's `rv32i` instruction suite,
but only its 37 generated checks corresponding to ProofCore's supported M1
integer instructions are run. Applicable `reg`, `pc_fwd`, `pc_bwd`, and
`unique` checks are also run. CSR, privilege, trap, compressed, and unrelated
checks are not enabled.

Run `make formal-rvfi` to execute the configured suite. Outputs and per-check
logs are written under ignored `build/formal/`; the committed summary is
`formal/results/results.json`. Standard instruction checks use ABC `bmc3` from
Yosys. They are bounded checks at depth 12. This is not an unbounded proof.

The compact harness has eight symbolic instruction words and eight symbolic
data words. Instruction words are stable `anyconst` values; data begins
arbitrary and stores update enabled lanes for later reads. The wrapper constrains
addresses to the 32-byte model and excludes misaligned transfers/accesses
because M1 freezes those as no-effect operations but does not implement the
architectural traps modeled by the upstream checker. Read
`formal/ASSUMPTIONS.md` before interpreting a PASS.

For covers and negative-control traces, install/provide Z3 on `PATH`; in the
authoring environment the validated executable was Z3 4.13.3. Use
`make formal-cover` and `make formal-negative`. An expected failure in a
negative-control run is summarized as self-test `PASS` with its observed formal
status separately recorded.
