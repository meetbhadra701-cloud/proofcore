# Milestone 3 cover results

The final `make formal-cover` run used SymbiYosys `smtbmc z3`, depth 24, on the
stable symbolic 32-byte internal harness. All 13 cover statements were reached
in 12 preserved traces. The reported step is the formal-timestep frame; the
formal top toggles the processor clock from that timestep, so a frame number is
not directly a processor cycle count.

| Scenario | Result | Shortest observed frame | Trace |
| --- | --- | ---: | --- |
| Load-use stall | COVERED | 7 | `trace0.vcd` |
| Load side effect | COVERED | 7 | `trace0.vcd` |
| Store side effect | COVERED | 7 | `trace1.vcd` |
| JALR redirect | COVERED | 7 | `trace2.vcd` |
| Taken branch redirect | COVERED | 7 | `trace3.vcd` |
| Not-taken branch | COVERED | 7 | `trace4.vcd` |
| JAL redirect | COVERED | 7 | `trace5.vcd` |
| EX/MEM forwarding | COVERED | 9 | `trace6.vcd` |
| Retirement after load-use stall | COVERED | 11 | `trace7.vcd` |
| MEM/WB forwarding | COVERED | 11 | `trace8.vcd` |
| Retirement after control redirect | COVERED | 11 | `trace9.vcd` |
| EX/MEM versus MEM/WB priority conflict | COVERED | 11 | `trace10.vcd` |
| Attempted write to x0 retires without an architectural write | COVERED | 11 | `trace11.vcd` |

The VCD and Yosys witness traces are committed under
`artifacts/formal/m3-core-cover-depth24/`. The complete reached-cover list and
engine result are in `cover-summary.txt`; the command output is preserved in
`solver.log`. A cover says only that at least one scenario is reachable. The
assertion campaign is separate and bounded to 16 steps.
