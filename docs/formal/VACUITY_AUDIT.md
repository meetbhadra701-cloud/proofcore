# Milestone 3 vacuity audit

A green assertion run is useful only if its triggering situations are possible.
The internal cover job uses stable symbolic instructions/data and records a
separate trace for each goal. Every cover completed by depth 24; the exact
statement, reached depth, and trace path are in `COVERAGE.md` and the SBY log.

| Assertion family | Can trigger? | Reachability evidence | Assumption interaction |
| --- | --- | --- | --- |
| x0 reads remain zero | Yes; every supported instruction can encode x0 as a source. | Register-source paths are present in the instruction covers; x0-write attempt is an explicit cover. | No assumption constrains x0 data. |
| RVFI order matches an independent retirement counter | Yes; valid retirements occur in the core model. | Retirement after ordinary flow, a load-use stall, and a redirect is covered; upstream `unique_ch0` also passes. | No assumption constrains `rvfi_valid` or `rvfi_order` in the DUT. |
| Retirement and register/memory effects are valid-gated | Yes; covers reach ALU, load, and store retirement. | All 13 internal cover statements are reported reached by the cover engine. | Address bounds do not force valid low. |
| EX/MEM forwarding and MEM/WB forwarding select a producer | Yes; each forwarding source is covered independently. | Dedicated covers for EX/MEM and MEM/WB selection. | No assumptions prescribe mux selection or matching register values. |
| EX/MEM wins when both eligible producers match | Yes; a conflict-state cover reaches matching valid destinations in EX/MEM and MEM/WB. | Dedicated priority-conflict cover. | Load producers are excluded from EX/MEM eligibility by the RTL condition, not by an assumption. |
| Load-use stall holds front end and inserts bubble | Yes; the `load_use_stall` cover reaches a producer/consumer dependency. | Stall cover and subsequent-retirement-after-stall cover. | No assumption suppresses loads or dependencies. |
| Redirect flush removes younger instructions | Yes; taken branch/JAL/JALR covers reach redirect. | Taken branch, JAL, JALR, and later retirement-after-redirect are covered. | For internal covers, the memory is bounded but the target does not need to be a specific constant. ISA-level aligned-target assumptions do not control this independent cover harness. |
| Memory side effects correspond to valid memory operations | Yes; load and store covers reach each side-effect class. | Explicit load/store covers and retirement-side-effect assertions. | Data address bounds limit locations only; they do not require masks to zero. |

The upstream per-instruction checker intentionally assumes its current sampled
retirement matches the requested instruction model (`assume(spec_valid)`).
This is how `insn_add_ch0`, for example, asks for ADD semantics. It is not used
as evidence that all instruction types occurred in one arbitrary run; every
supported instruction has its own check record. The internal cover job provides
independent evidence that broad classes of dependencies and side effects are
reachable.

Residual limitation: the cover goals establish existence within the finite
symbolic model. They do not provide a completeness measure for the entire ISA,
all register values, or every pipeline interleaving. Bounded cover depth is
reported as reachability evidence only.
