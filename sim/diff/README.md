# Retirement Lockstep Differential Harness

The Verilator harness instantiates `core`, supplies combinational instruction/data reads, and applies byte-masked stores to a host-side RTL memory at the same active edge as the RTL. A separate copy of the deterministic initial memory belongs to the reference model. The reference applies stores once when its one-instruction `step()` produces the matching architectural retirement. The two memories are compared byte-by-byte at the end of every program, and load results are compared at retirement.

The comparator is retirement-driven: it ignores bubbles and cycle counts, but checks the ordered architectural PC, instruction, destination write, destination/value when written, next PC, memory read/write masks, and meaningful memory addresses/store data. Unused fields are not compared as if they were architectural facts.

Commands:

```sh
make diff-directed                 # 9 translations of the M1 directed Cocotb scenarios
make diff-random                   # 500 programs, base seed 12345 by default
make diff-random M2_SEED=98765 M2_PROGRAMS=1000
make comparator-selftest           # expected test-only failure and artifact creation
```

The self-test flag is an explicit command-line mode; ordinary runs never corrupt events. A mismatch creates `failure.json`, `failure.md`, `program.case`, `program.bin`, `program.hex`, `program.s`, `memory.hex`, `seed.txt`, and `retire-trace.jsonl` under the selected artifact directory. The disassembler covers this subset only. A failure artifact supports replay; it does not minimize the program or localize root cause.
