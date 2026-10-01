# Milestone 2 Differential-Verification Architecture

## Two implementations, one architectural boundary

```text
             ┌────────────────────┐
program ────▶│ ProofCore RTL       │── retirement pins ──┐
image        │ IF/ID/EX/MEM/WB    │                      │
             └─────────┬──────────┘                      ▼
                       │ separate RTL memory       ┌───────────────┐
                       ▼                            │ semantic      │── PASS / failure artifact
               byte-mask adapter                     │ comparator    │
                                                    └───────▲───────┘
             ┌────────────────────┐                      │
program ────▶│ C++ RV32I model    │── expected event ─────┘
image        │ one architectural │
             │ instruction/step   │
             └───────┬────────────┘
                     ▼
              separate reference memory
```

The reference model owns architectural state only. The RTL remains free to use valid bits, stalls, pipeline registers, and bypass muxes. Their only synchronization point is the ordered WB retirement stream.

## Memory contract

Both environments construct separate byte memories from the same deterministic initial image. Test-case overrides are applied independently to both copies before reset/run. The RTL adapter reads a little-endian 32-bit word at the aligned data-memory address and commits byte lanes from the RTL store port at the rising edge. The reference model reads and writes architectural byte addresses while executing one instruction. Thus no RTL store is applied to the reference memory, and no reference store is applied to the RTL memory. At each retirement, a load's destination value and read mask are compared; stores compare effective address, mask, and lane-positioned data. At the end of a program, all modeled memory bytes are compared.

## Comparison contract

Every valid RTL retirement consumes exactly one reference `step()`. The comparator checks PC, instruction, register-write enable and destination, written value when a write occurs, and architectural next PC. It checks read/write masks on every record; when a memory mask is nonzero it checks the effective byte address, and for stores it checks lane-positioned write data. It intentionally ignores `rd_value` when no register write occurs and ignores memory address/data when no memory access occurs. This avoids comparing interface don't-cares while preserving all M1 architectural decisions.

## Bounds and outcomes

Each case supplies an architectural `stop_pc`; a reference-side preflight determines how many instructions should retire to reach it, with a 100,000-instruction safety limit. RTL execution has a separate cycle bound. Results are classified as `PASS`, `FAIL`, `TIMEOUT`, or `SETUP_ERROR`; reaching neither stop point nor a proven comparison is never success.

Random programs contain bounded backward loops only. Forward branch/jump snippets either skip one known poison instruction or fall through. Every case has an explicit deterministic seed and appears in `manifest.json` with static instruction count and generation tier.
