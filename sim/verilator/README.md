# Verilator simulation

From the repository root, run `make test`. `scripts/run_cocotb.py` builds the `core` top with Verilator and runs `tb/cocotb/test_core.py`. Generated C++ and simulator artifacts stay under the ignored `build/verilator/` directory.
