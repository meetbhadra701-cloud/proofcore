# Independent RV32I Reference Model

`reference/src/rv32i_model.cpp` is a single-step architectural interpreter for ProofCore's frozen M1 subset. It owns only a PC, 32 integer registers, a sparse little-endian byte memory, and an instruction image. It has no pipeline, hazard, forwarding, or cycle concepts.

The decoder in `rv32i_decode.cpp` independently switches on the public RISC-V opcode/funct fields. No RTL package, generated decode table, or shared implementation helper is imported. Public ISA bit positions are necessarily common assumptions; those assumptions and the chosen M1 unsupported/misalignment behavior are recorded in `DECISIONS.md`.

Run the independent C++ unit tests with:

```sh
make reference-test
```

They compile under C++20 with `-Wall -Wextra -Wpedantic -Werror`. The suite covers every implemented operation family, signed and unsigned boundaries, immediate formats, subword memory behavior, control flow, x0, and the M1 deterministic no-effect policy.
