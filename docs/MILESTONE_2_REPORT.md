# ProofCore Milestone 2 Engineering Report

## Result

**MILESTONE 2: ENGINEERING COMPLETE**

**MILESTONE 2: HUMAN LEARNING UNASSESSED**

Milestone 2 adds an independent C++ RV32I architectural reference, a Verilator retirement-lockstep harness, deterministic program generation, semantic event comparison, and replayable failure artifacts. M1 RTL was not modified. The final `make verify` passed all M1 and M2 gates.

## Verified baseline

M1 had no clean Git commit when M2 began. Its complete verified tree was committed and tagged before M2 work:

- Commit: `54500b0a10da917249087fe246179b71066c1ced`
- Tag: `milestone-1`
- Baseline `make verify`: PASS before M2 implementation.
- No force-push, history rewrite, or remote push occurred.

## Reference-model architecture

`reference/src/rv32i_decode.cpp` decodes public RV32I opcode/funct fields through its own C++ control flow. `reference/src/rv32i_model.cpp` executes one complete instruction per `step()` using architectural PC/register/memory state. It contains no pipeline registers, cycle state, forwarding, stall, flush, or RTL-generated tables. C++ unsigned arithmetic supplies defined 32-bit wraparound; signed comparisons and SRA deliberately reinterpret 32-bit values.

The reference matches the frozen M1 subset exactly: all implemented integer ALU/immediate operations, aligned subword loads/stores, branches, JAL/JALR, LUI/AUIPC, and deterministic inert behavior for unsupported encodings and out-of-contract misaligned operations. It does not model traps or privilege state.

The C++ unit test executable covers every supported operation family and significant signedness, immediate, control, x0, and memory cases. It builds with C++20 and `-Wall -Wextra -Wpedantic -Werror`.

## Differential harness and retirement contract

`sim/diff/rtl_driver.cpp` instantiates Verilator's generated `Vcore`, applies synchronous reset, supplies instruction/data memory inputs, and samples valid retirement records. The C++ model steps exactly once per valid RTL retirement; clock cycles with bubbles or stalls do not advance it.

The comparator checks retirement PC, instruction bits, destination write enable/index, destination value when written, next PC, and read/write masks. It compares the effective memory address when an access occurs and store data for stores. It does not compare unused values when there is no corresponding architectural effect. This event schema mirrors the M1 retirement record and is not RVFI.

## Memory synchronization

The host maintains two distinct little-endian byte memories. Both are constructed independently from the same deterministic 256-byte initial image and the same case-specific overrides. The RTL copy changes only from the core's byte mask and store data at the active edge. The reference copy changes only when its architectural step executes a store. The reference never updates the RTL copy, and vice versa. Load result/mask/address, store address/mask/lane data, and final memory bytes are checked.

## Directed and randomized campaigns

`make diff-directed` passes nine machine-code cases translated from the M1 Cocotb scenarios: 90 retirement comparisons across 148 RTL cycles.

The fixed-seed default random run passes 500 programs over seeds 12345–12844, totaling 11,760 static instructions, 11,188 dynamic retirement comparisons, and 15,521 RTL cycles. There were zero failures and zero timeouts. The generator distributed cases across six tiers: arithmetic (84), dependencies (84), memory (83), load-use (83), control flow (83), and mixed programs (83). The generated manifest records every per-case seed, tier, and static instruction count.

Backward loops are bounded by small initialized counters. Each case has a reference architectural stop PC, an instruction limit, and an independent RTL cycle limit. A timeout is never classified as a pass.

## Comparator self-test and failure artifacts

`make comparator-selftest` deliberately alters only the sampled comparison copy, leaving both RTL and reference semantics unchanged. In the self-test program `addi x5,x0,7; add x6,x5,x5`, it changes observed `x6=14` to `x6=7` at retirement 2. The ordinary comparator reports `rd_value`, exits nonzero, and writes the standard artifact. The artifact was JSON-parsed and replayed with the explicit test-only flag.

Artifacts include `failure.json`, `failure.md`, a replayable `program.case`, raw `program.bin`, `program.hex`, human-readable subset `program.s`, complete initial `memory.hex`, `seed.txt`, and recent `retire-trace.jsonl`. These files preserve the failure; they do not minimize the witness or identify its cause.

## Validation evidence

| Gate | Observed result |
| --- | --- |
| M1 Cocotb | PASS — one regression, nine scenarios, 498 ns |
| M1 Verilator lint | PASS — Verilator 5.032, `-Wall`, zero warnings |
| M1 Yosys synthesis/check | PASS — Yosys 0.52, final `check` has zero problems |
| C++ reference unit tests | PASS — four suites with strict warnings as errors |
| M1-derived differential corpus | PASS — nine programs, 90 retirements |
| Deterministic random differential | PASS — 500 programs, 11,188 retirements |
| Comparator self-test | PASS — injected mismatch found and artifact replayed |
| Integrated `make verify` | PASS |

The generic Yosys result has 8,556 cells; it is not a technology-mapped area or timing result. The M2 reference model and harness are host-side verification software and do not synthesize into the core.

## Bugs discovered during development

The campaign found no RTL defect, and M2 changed no RTL. Test construction caught incorrect initial expectations for a shift/bitwise unit case and for a JALR target: clearing bit zero turns `0x101` into aligned `0x100`, so `0x102` is needed to exercise the M1 suppressed-transfer policy. A C++ variable-scope compile issue was fixed before integration. A disassembler operand-order issue was corrected before the final campaign so failure assembly is readable. These were development/test issues, not architectural RTL failures; checks were not weakened.

## Known limitations

- Differential evidence is empirical, not formal proof, exhaustive verification, or a claim that ProofCore is bug-free.
- Independent implementations necessarily share public RV32I opcode and field-position facts; a shared ISA misunderstanding remains possible.
- The random generator uses structured templates and bounded loops, rather than arbitrary legal instruction streams.
- The model implements only the documented M1 subset and its deterministic unsupported/misalignment no-effect policy.
- No traps, privileged behavior, RVFI, formal properties, ProofTrace, mutation testing, minimization, causal localization, synthesis checkpoint equivalence, PPA, or optimization is included.
- The subset disassembler is not a general RISC-V disassembler.

## Reproduction commands and tools

```sh
make verify-m1
make reference-test
make diff-directed
make comparator-selftest
make diff-random
make verify
```

Observed tools: Verilator 5.032, GCC/G++ 15.2.0, Yosys 0.52, Python CLI 3.14.4, and Cocotb 2.1.0 (the simulator reported embedded Python 3.14.0). The bundled Verilator runtime emits two GCC 15 `%p` format warnings, so that specific third-party warning is not promoted to an error in the co-simulation build; the reference unit build uses the complete `-Werror` flags.

## Learning pack and Milestone 3 starting point

The teaching sequence is in `docs/learning/milestone-2/`, with the annotated implementation trace in `12-important-code-walkthrough.md`, the arithmetic/memory/branch example in `13-end-to-end-example.md`, and 50 `UNASSESSED` questions in `FINAL_TEACHBACK.md`. The human has not yet demonstrated teach-back knowledge.

Milestone 3 should begin by defining a small retirement-to-ISA-reference contract and adding assertion-based checks around the existing interface, then consider an RVFI adapter. Keep differential testing as an independent regression and do not treat this M2 campaign as a formal proof.
