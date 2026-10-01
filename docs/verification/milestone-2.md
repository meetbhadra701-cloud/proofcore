# Milestone 2 Verification Record

## Baseline

- Verified M1 snapshot commit: `54500b0a10da917249087fe246179b71066c1ced`
- Tag: `milestone-1`
- Baseline command before M2 changes: `make verify` — PASS.
- Final integrated command after M2 changes: `make verify` — PASS.
- Repository began without a commit; the clean tested M1 tree was committed and tagged before M2 implementation. No remote push or history rewrite was performed.

## Tool versions

- Verilator: 5.032
- GCC/G++: 15.2.0
- Yosys: 0.52 (ABC available)
- Python CLI: 3.14.4
- Cocotb: 2.1.0 (simulator reported embedded Python 3.14.0)
- Build tools were available in the authoring environment; no CI workflow was added.

## Reference model

- Command: `make reference-test`
- Implementation: independent C++20 sequential architectural model and separately written decoder.
- Compiler flags: `-Wall -Wextra -Wpedantic -Werror`.
- Result: PASS, four test suites covering every implemented instruction family, signed/unsigned and immediate boundaries, control transfer, x0, subword memory, and M1 unsupported/misalignment policies.
- No RTL source files or generated RTL decode/control tables are included by the reference implementation.

## Directed differential run

- Command: `make diff-directed`
- Programs: 9 direct translations of the M1 Cocotb scenarios.
- Retirement comparisons: 90.
- RTL cycles across cases: 148 (reset cycles excluded).
- Result: PASS; no failures or timeouts.
- Coverage includes ALU, forwarding priority, load-use, all branch predicates, negative branch loop, load-to-branch, jumps/flush, wrong-path store suppression, subword operations, unsupported instruction, and misalignment behavior.

## Random differential run

- Command: `make diff-random`
- Programs: 500.
- Seed set: 12345 through 12844 inclusive; xorshift64* generator, exact cases captured in `build/m2-random/manifest.json` for the recorded run.
- Static instruction words generated: 11,760 total.
- Dynamic retirement comparisons: 11,188.
- RTL cycles across cases: 15,521 (reset cycles excluded).
- Failures: 0.
- Timeouts: 0.
- Result: PASS.

| Tier | Programs | Static instruction words |
| --- | ---: | ---: |
| A — arithmetic | 84 | 2,520 |
| B — dependencies | 84 | 2,268 |
| C — memory | 83 | 1,909 |
| D — load-use | 83 | 830 |
| E — control flow | 83 | 2,739 |
| F — mixed | 83 | 1,494 |

The generator was also run twice with the same seed/count and produced byte-identical case directories.

## Comparator self-test

- Command: `make comparator-selftest`
- Test image: `addi x5,x0,7; add x6,x5,x5`.
- Deliberate sampled-event corruption at retirement 2 changes observed `x6=14` to `x6=7` at PC `0x00000004`.
- Expected behavior: executable returns nonzero and standard artifact is written.
- Actual artifact: `build/m2-selftest-failures/seed-12345-cycle-6-ret-2/`.
- `failure.json` parsed successfully and recorded `rd_value` mismatch, expected 14, observed 7, full 256-byte default initial memory, pre-step registers, seed, and recent retirements. Program image, disassembly, and replay `.case` are present.
- The artifact was replayed with the explicit self-test flag and reproduced the expected nonzero mismatch.
- Result: PASS (the injected differential run fails as designed; the Make self-test target passes).
- Normal runs do not activate corruption.

## M1 regression, lint, and synthesis

- `make test`: PASS — one Cocotb regression test, nine directed scenarios, simulation duration 498 ns.
- `make lint`: PASS — Verilator 5.032 `--lint-only --sv -Wall`, zero warnings.
- `make synth-check`: PASS — Yosys 0.52 hierarchy, process, synthesis, and structural checks; final `check` reports zero problems. Generic synthesis output: 8,556 cells; this is not a PPA result.
- The final `make verify` reran these gates plus all M2 gates and passed.
- Verilator's bundled C++ runtime has two GCC 15.2 `%p` format warnings; the co-simulation compilation downgrades only that third-party format warning from an error. The reference model/unit binary remains compiled with full `-Werror`.

## Bugs and fixes during development

- No RTL defect was found by the M2 differential campaign; no RTL file was changed.
- Initial reference test expectations for SLL and bitwise results were corrected after decoding the actual test operands.
- A JALR alignment test was corrected: target `0x101` becomes `0x100` after clearing bit zero and is aligned; the test now uses `0x102` to exercise M1's suppressed misaligned-transfer behavior.
- A local C++ build error from a shift amount scoped only inside the load path was fixed before integration.
- The C++ disassembler's store/branch operand order was corrected so artifact assembly is useful to a human reader.
- None of these were RTL functional failures; no validation check was weakened.

## Not run / limits

- Formal proof and equivalence checking: NOT RUN.
- Spike or another external ISA simulator: NOT RUN; M2 does not depend on downloading one.
- The 500 programs are a constrained, deterministic empirical campaign, not exhaustive coverage. The independent model necessarily shares public ISA encoding facts and mirrors the frozen M1 no-effect policies.
- No trap support, RVFI, ProofTrace, minimization, causal localization, mutation system, synthesis-stage equivalence, PPA, or AI optimization was implemented.
