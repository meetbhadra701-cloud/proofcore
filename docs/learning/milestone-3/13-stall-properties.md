# 13 — Load-use stall properties

An ALU result is available near the end of EX, but a load's value is returned
from the memory interface in MEM. A consumer immediately behind a load would
reach EX before that load value is available in the bypass network.

```asm
lw  x5, 0(x1)
add x6, x5, x2
```

During the dependency cycle, `load_use_stall` must hold PC and IF/ID and insert
an invalid entry into ID/EX. The load continues into EX/MEM; the consumer remains
in IF/ID. On the next cycle, the load advances to MEM/WB and the consumer enters
EX, where MEM/WB forwarding supplies the returned value.

`rtl/core.sv` saves PC and IF/ID before a stall and checks that they remain
unchanged on the following sampled state. It also requires ID/EX to be invalid,
which is the bubble. A cover reaches a load-use hazard, and another cover
requires a later retirement after a stall. These show both trigger and recovery
are reachable. The 16-step bounded assertion run checks the relationships.

**Failure example:** If IF/ID advances while PC holds, the dependent instruction
can be lost or duplicated. If ID/EX is not bubbled, the same consumer can
execute once with stale data and again with correct data.

**Hardware view:** A hazard comparator, front-end enables, and valid-bit muxing
implement the stall. A stall does not stop the entire pipeline: the older load
continues toward WB.

**Interview question:** Why can forwarding solve ADDI-to-ADD but not this
immediate load-to-ADD timing?

**Exercise:** Draw five pipeline cycles for the code above and mark the one
cycle where PC and IF/ID hold.
