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
