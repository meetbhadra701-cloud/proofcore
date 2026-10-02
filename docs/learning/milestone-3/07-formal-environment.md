# 07 — The formal environment

A processor cannot be checked in isolation from its clock, reset, and memories.
The harness supplies those surroundings. A poor harness can create impossible
behavior, remove difficult behavior, or make a check pass without the core
ever executing.

`formal/harness/formal_top.sv` drives the internal-property clock from a
SymbiYosys global formal timestep and asserts synchronous reset for the first
core clock edge. This choice is important: simply marking an undriven core clock
as a global clock generated no real transitions. The repaired harness toggles a
core clock on each formal timestep and makes state changes visible to BMC.

`formal/harness/formal_memory.sv` contains eight 32-bit words for instructions
and eight for data. Every instruction word is arbitrary but stable. Initial
data words are arbitrary; byte writes update the selected lanes, so a later
read sees the earlier store. The wrapper keeps memory accesses inside 32 bytes.

The RVFI wrapper and internal-property harness are distinct. The first exposes
standard RVFI signals to the upstream checkers. The second connects internal
properties under `FORMAL`. This makes the contract and property families easier
to review separately.

This environment does not model bus waits, memory faults, caches, code writes,
external agents, or the full address space. Those are excluded by the contract
and assumptions, not proved correct.

**Failure example:** If a data read returned a new arbitrary word every cycle,
the solver could invent a different value after a store and the memory check
would not mean real load-after-store behavior.

**Interview question:** What is the difference between symbolic initial memory
and a nondeterministic value on every read?

**Exercise:** Describe how a masked store to lane 2 should affect a later word
read when the initial word is `0x11223344`.
