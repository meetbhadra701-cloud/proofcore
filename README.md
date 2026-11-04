# ProofCore

A five-stage, single-issue RV32I-subset processor used to study a practical verification question: how much confidence can simulation, differential testing, and bounded formal checks establish about the same RTL?

**Status:** Milestones 1–3 are engineering complete; Milestone 4 has not started. The documented human teach-back remains unassessed. This project was developed with Codex assistance; [the contribution record](docs/AI_CONTRIBUTION.md) distinguishes generated implementation, validation, and human study.

## Design and verification

The in-order pipeline has IF, ID, EX, MEM, and WB stages, explicit valid bits, forwarding, load-use stalls, branch redirects, and an optional RVFI retirement interface. It implements the 37 instructions listed in the [frozen specification](SPEC.md), with the exclusions stated below.

| Layer | Purpose | Evidence |
| --- | --- | --- |
| Directed simulation | Exercise pipeline, control, and memory behavior | Nine Cocotb scenarios passed in the recorded M1 run. |
| Differential testing | Compare retirement events and final memory against an independent C++ architectural model | Nine directed programs and 500 seeded random programs passed in the recorded M2 run. |
| Bounded formal | Check supported instruction semantics, internal properties, and reachability | 37 instruction and four consistency checks passed at depth 12; internal properties passed at depth 16; 13 cover statements were reached at depth 24. |
| Negative controls | Check whether the verification layers detect deliberate faults | Two injected faults produced their expected formal failures. |

These are [recorded results](docs/verification/milestone-3.md), not a new run performed for this README. The [machine-readable formal manifest](formal/results/results.json) identifies check status, depth, solver, and assumptions. A bounded pass does **not** establish unbounded correctness or complete RV32I compliance. Generic Yosys cell counts are structural synthesis results, not PPA measurements.

## Reproduce

Clone submodules, then use a Linux environment with Python 3, a C++20 compiler, Verilator, Yosys, Cocotb, SymbiYosys, and the solvers required by the selected formal target. The recorded M3 campaign used Verilator 5.032, Yosys 0.52, ABC `bmc3`, and Z3 4.13.3; pinned `riscv-formal` and SymbiYosys revisions are submodules. See [the verification record](docs/verification/milestone-3.md) and [formal assumptions](formal/ASSUMPTIONS.md) before comparing results.

```sh
git clone --recurse-submodules https://github.com/meetbhadra701-cloud/proofcore.git
cd proofcore
make verify-m1          # lint, Cocotb directed tests, synthesis/check
make verify-m2          # C++ reference, directed/random differential tests
make verify-m3-sim      # M2 checks plus RVFI differential tests
make verify-formal      # bounded ISA, internal, cover, and negative controls
```

`make verify` is the ordinary M1/M2 regression plus RVFI directed comparison; it does not run the entire formal campaign. Formal targets need the pinned submodules and local tool paths described in [`formal/rvfi/README.md`](formal/rvfi/README.md). Missing tools, timeouts, and unsupported checks are not passes.

<details>
<summary>Architecture, repository map, and scope</summary>

The RTL pipeline retires valid instructions in program order at WB. The independent C++ model advances once per retirement, and the Verilator driver compares meaningful architectural fields and separate memory state. Optional RVFI fields feed the pinned `riscv-formal` checks. Internal SVA covers x0 behavior, retirement qualification, forwarding, stalls, redirects, and memory side effects. Reachability covers and fault-injection controls help audit vacuity.

| Path | Contents |
| --- | --- |
| [`rtl/`](rtl/) | Core, pipeline registers, decode, ALU, forwarding, and hazard logic |
| [`tb/`](tb/) and [`scripts/`](scripts/) | Directed simulation and validation runners |
| [`reference/`](reference/) and [`sim/diff/`](sim/diff/) | Independent C++ model and retirement comparator |
| [`formal/`](formal/) | RVFI harness, properties, assumptions, results |
| [`docs/`](docs/) | Architecture decisions, verification reports, learning material |
| [`third_party/`](third_party/) | Pinned external formal dependencies |

The formal environment assumes bounded addresses, aligned control flow and halfword/word accesses, and finite symbolic memories. The core omits traps for illegal or misaligned instructions, privilege modes, caches, wait states, interrupts, and full address-space behavior. [STATUS.md](STATUS.md) tracks current progress and known limitations; [DECISIONS.md](DECISIONS.md) records design choices.

</details>
