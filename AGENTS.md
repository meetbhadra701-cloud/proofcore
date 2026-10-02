# Instructions for ProofCore coding agents

## Project goal and source of truth

ProofCore is both an engineering project and a learning project. The engineering phase may proceed autonomously through all of Milestone 1. After the implementation and verification gates pass, create the full learning pack requested in the user specification. Do not claim that the human understands the design until they complete teach-back themselves.

`SPEC.md` is the frozen behavioral contract. `PLAN.md` defines the implementation sequence and validation commands. `STATUS.md` records actual progress and results. `DECISIONS.md` records choices and changes. Keep them consistent so the repository carries its own project memory.

## Correctness and scope rules

- Never silently weaken correctness checks to make a test pass.
- Work in scoped engineering increments, but do not stop at educational checkpoints during M1. Complete the authorized milestone and then teach it.
- Do not implement later systems during M1: RVFI, ProofTrace, ISA-level formal proof infrastructure, causal localization, mutation testing, witness minimization, synthesis checkpoint equivalence, PassWitness, physical-design/PPA, or AI RTL optimization.
- Make changes to RTL only when valid is true. Bubbles are explicit invalid state, not an instruction-bit convention.
- Keep widths and signedness intentional. Prefer named types, explicit casts, `always_comb`, and `always_ff`.
- Do not copy another processor implementation. If external code is consulted, record what was used, why, and its attribution/license obligations in `DECISIONS.md`.
- Run the validation tools available in the environment. TIMEOUT, UNKNOWN, NOT RUN, UNSUPPORTED, or missing tools are not PASS. Report limitations honestly.
- Do not suppress or downgrade a failing check without proving the check itself is invalid; fix the implementation or document a real spec correction with evidence.
- Keep source comments focused on invariants, timing, signedness, and non-obvious hardware behavior. Put teaching explanations in `docs/learning/`.
- Do not claim formal correctness without an actual proof and evidence.

## Learning and contribution records

After engineering is complete, create every document listed in the owner's learning-pack specification, including detailed code walkthroughs and full instruction timelines. Include concrete values, cycle tables where relevant, failure examples, test explanations, hardware interpretations, exercises, and interview answers in clearly separated sections.

Create `docs/AI_CONTRIBUTION.md` that accurately distinguishes AI-generated work, validation performed, and the human's subsequent study. Keep every teach-back question marked UNASSESSED until the human answers it.

At completion, distinguish `MILESTONE 1: ENGINEERING COMPLETE` from `MILESTONE 1: HUMAN LEARNING UNASSESSED`.

## Milestone 2 durable invariants

- Preserve M1 as the frozen architectural contract unless a demonstrated contract defect requires a documented correction. Never redesign the processor for stylistic preference.
- The C++ reference is an architectural interpreter only: no pipeline registers, cycle state, forwarding, stalls, flushes, or RTL-generated decode tables.
- Keep the ISA reference decoder independent from `rtl/`; shared assumptions are public RISC-V opcode/field encodings and explicitly documented M1 subset policies.
- Keep RTL-adapter memory and reference memory as distinct copies with identical deterministic initial bytes. Each copy changes only from its own implementation's store semantics. Do not route one model's stores into the other's memory.
- Advance the reference once per valid architectural retirement, never once per RTL cycle. Compare only meaningful retirement fields and document every don't-care decision.
- Every random program has a fixed recorded seed, a bounded architectural exit, and a safety cycle/instruction limit. A timeout is a failure state, never a pass.
- Comparator corruption is allowed only behind an explicit self-test-only mode and must produce an ordinary structured failure artifact. Normal regression runs must not activate it.
- Preserve all M1 tests and add M2 checks alongside them. `make verify` must continue running every completed milestone gate.
- Never silently weaken correctness checks to make a test pass. When the model, generator, comparator, or RTL disagrees, identify and document the root cause before changing expectations.
- Failure artifacts must be sufficient to reconstruct the program and initial memory and must not claim minimization or causal localization.
- Do not implement RVFI, formal proofs/SVA, ProofTrace, mutation testing, witness minimization, PPA/OpenROAD, causal localization, or AI optimization in M2.

## Milestone 3 authorization and invariants

Milestone 3 supersedes the M1/M2 non-goal lists above for this milestone only:
optional RVFI, bounded `riscv-formal` checks, ProofCore internal properties,
covers, and narrowly scoped test-only negative controls are authorized. Do not
start ProofTrace, causal localization, a Bug Zoo, witness minimization,
PassWitness, synthesis-stage equivalence, PPA/OpenROAD, or AI optimization.

- Preserve the frozen M1 behavior and all M1/M2 regressions. RVFI remains off in
  ordinary builds unless `PROOFCORE_RVFI` is explicitly defined.
- Never silently weaken correctness checks to make a test pass. When a formal
  check fails, preserve its trace/log where feasible, establish whether it is a
  design defect, harness defect, or unsupported architectural behavior, then
  fix the appropriate cause and rerun the check.
- State every formal assumption in `formal/ASSUMPTIONS.md`, including why it is
  required, what it excludes, whether it matches `SPEC.md`, and defects it
  could hide. Audit assumptions and assertion antecedents for vacuity.
- Report bounded BMC, induction/unbounded proof, and cover reachability as
  different evidence. A BMC pass never implies unbounded correctness.
- Keep intentional corruption behind explicit test-only defines. Normal RTL,
  lint, synthesis, simulation, and formal campaigns must not enable them.
- After M3 engineering gates pass, create the complete requested M3 learning
  pack and leave every teach-back item `UNASSESSED` until the human answers it.
- Do not claim human understanding from generated documentation or passing
  checks.
- After engineering gates pass, create the full M2 learning pack and leave every teach-back question `UNASSESSED` until answered by the human.
