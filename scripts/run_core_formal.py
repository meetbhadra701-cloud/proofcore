#!/usr/bin/env python3
"""Run ProofCore's assertion and cover harness with pinned Yosys/ABC."""

from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import time


ROOT = Path(__file__).resolve().parents[1]
YOSYS = Path("/tmp/proofcore-tools/root/usr/bin/yosys")
SBY = ROOT / "third_party" / "sby" / "sbysrc" / "sby.py"
BUILD = ROOT / "build" / "formal"
MANIFEST = ROOT / "formal" / "results" / "results.json"


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--mode", choices=("prove", "cover"), required=True)
    parser.add_argument("--negative-x0", action="store_true",
                        help="Enable the test-only x0 corruption and require a formal FAIL.")
    args = parser.parse_args()
    if args.negative_x0 and args.mode != "prove":
        parser.error("--negative-x0 requires --mode prove")
    if not YOSYS.exists() or not SBY.exists():
        print("SETUP_ERROR: pinned Yosys/SymbiYosys tools are not available", file=sys.stderr)
        return 2

    mode = "cover" if args.mode == "cover" else "bmc"
    if (mode == "cover" or args.negative_x0) and not shutil.which("z3"):
        print("SETUP_ERROR: this check requires z3 on PATH", file=sys.stderr)
        return 2
    depth = 24 if mode == "cover" else 16
    run_name = "negative-x0-selftest" if args.negative_x0 else f"core-properties-{args.mode}"
    run_dir = BUILD / run_name
    run_dir.mkdir(parents=True, exist_ok=True)
    template = (ROOT / "formal" / "properties" / "core-properties.sby.in").read_text(encoding="utf-8")
    engine_section = template.split("[engines]", 1)[1].split("[", 1)[0]
    engine = next(line.strip() for line in engine_section.splitlines() if line.strip())
    if mode == "cover" or args.negative_x0:
        template = template.replace("abc bmc3", "smtbmc z3")
        engine = "smtbmc z3"
    negative_define = "-DPROOFCORE_NEGATIVE_X0" if args.negative_x0 else ""
    sby_text = (template.replace("@MODE@", mode).replace("@DEPTH@", str(depth))
                .replace("@ROOT@", ROOT.as_posix()).replace("@NEGATIVE_DEFINE@", negative_define))
    sby_file = run_dir / f"{run_name}.sby"
    sby_file.write_text(sby_text, encoding="utf-8")

    env = os.environ.copy()
    env["PATH"] = os.pathsep.join((str(YOSYS.parent), env.get("PATH", "")))
    env["YOSYS"] = str(YOSYS)
    env["PYTHONPATH"] = os.pathsep.join(("/tmp/proofcore-tools/root/usr/share/yosys", env.get("PYTHONPATH", "")))
    started = time.monotonic()
    with (run_dir / "solver.log").open("w", encoding="utf-8") as output:
        process = subprocess.run([sys.executable, str(SBY), "-f", str(sby_file)],
                                 cwd=run_dir, env=env, stdout=output, stderr=subprocess.STDOUT)
    duration = time.monotonic() - started
    status_path = run_dir / run_name / "status"
    status_line = status_path.read_text(encoding="utf-8").strip().split() if status_path.exists() else []
    status = status_line[0].upper() if status_line else "SETUP_ERROR"
    formal_status = status
    if status not in {"PASS", "FAIL", "UNKNOWN", "TIMEOUT", "ERROR", "COVER"}:
        status = "UNKNOWN"
    if mode == "cover" and status == "PASS":
        status = "COVERED"
    if args.negative_x0 and formal_status == "FAIL":
        status = "PASS"
    record = {
        "name": run_name,
        "category": "Negative-control self-test" if args.negative_x0 else
                    ("ProofCore internal properties" if mode == "bmc" else "ProofCore cover reachability"),
        "mode": "bounded model checking" if mode == "bmc" else "bounded cover",
        "depth": depth,
        "solver": engine,
        "status": status,
        **({"expected_status": "FAIL", "observed_status": formal_status} if args.negative_x0 else {}),
        "runtime_seconds": round(duration, 3),
        "assumptions": ["initial synchronous reset", "32-byte stable symbolic instruction/data windows"],
        "artifact_path": str((run_dir / run_name).relative_to(ROOT)),
        "log_path": str((run_dir / "solver.log").relative_to(ROOT)),
    }
    payload = json.loads(MANIFEST.read_text(encoding="utf-8")) if MANIFEST.exists() else {"checks": []}
    rows = [row for row in payload.get("checks", []) if row.get("name") != run_name]
    rows.append(record)
    payload["checks"] = rows
    MANIFEST.parent.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    print(f"{status} {run_name}: {duration:.2f}s (depth {depth})")
    expected_result = formal_status == "FAIL" if args.negative_x0 else status in {"PASS", "COVERED"}
    if (process.returncode != 0 and not (args.negative_x0 and formal_status == "FAIL")) or not expected_result:
        print(f"See {run_dir / 'solver.log'}", file=sys.stderr)
        return 1
    if args.negative_x0:
        print("SELFTEST PASS: test-only x0 corruption was detected by the expected assertion failure")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
