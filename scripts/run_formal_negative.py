#!/usr/bin/env python3
"""Run narrow test-only corruptions and require each formal layer to catch them."""

from __future__ import annotations

import os
from pathlib import Path
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[1]


def main() -> int:
    commands = [
        [sys.executable, str(ROOT / "scripts" / "run_riscv_formal.py"), "--negative-rvfi"],
        [sys.executable, str(ROOT / "scripts" / "run_core_formal.py"), "--mode", "prove", "--negative-x0"],
    ]
    for command in commands:
        result = subprocess.run(command, cwd=ROOT, env=os.environ.copy())
        if result.returncode:
            print(f"NEGATIVE_CONTROL_FAIL: {' '.join(command)} returned {result.returncode}", file=sys.stderr)
            return result.returncode
    print("PASS: both deliberate formal corruptions were detected as expected")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
