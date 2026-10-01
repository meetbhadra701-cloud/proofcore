#!/usr/bin/env python3
"""Build and run the cycle-level cocotb regression."""
import os
import shutil
from pathlib import Path

from cocotb_tools.runner import get_runner

ROOT = Path(__file__).resolve().parents[1]
RTL = [
    "rtl/proofcore_pkg.sv",
    "rtl/alu.sv",
    "rtl/imm_gen.sv",
    "rtl/regfile.sv",
    "rtl/decoder.sv",
    "rtl/forwarding.sv",
    "rtl/hazard_unit.sv",
    "rtl/pipeline/if_id.sv",
    "rtl/pipeline/id_ex.sv",
    "rtl/pipeline/ex_mem.sv",
    "rtl/pipeline/mem_wb.sv",
    "rtl/core.sv",
]

# The project remains usable with a normally installed simulator. In the
# constrained authoring image, apt packages are extracted under /tmp instead.
local_tools = Path("/tmp/proofcore-tools/root/usr")
if not shutil.which("verilator") and (local_tools / "bin/verilator").exists():
    os.environ["PATH"] = str(local_tools / "bin") + os.pathsep + os.environ.get("PATH", "")
    os.environ["VERILATOR_ROOT"] = str(local_tools / "share/verilator")

sim_name = os.environ.get("SIM", "verilator")
runner = get_runner(sim_name)
runner.build(
    sources=[ROOT / source for source in RTL],
    hdl_toplevel="core",
    build_dir=ROOT / "build" / sim_name,
    build_args=["--timing"],
    always=True,
)
runner.test(
    hdl_toplevel="core",
    test_module="test_core",
    test_dir=ROOT / "tb" / "cocotb",
)
