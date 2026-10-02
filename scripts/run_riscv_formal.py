#!/usr/bin/env python3
"""Stage the pinned riscv-formal checkout and run its generated RV32I checks."""

from __future__ import annotations

import argparse
import fnmatch
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import time


ROOT = Path(__file__).resolve().parents[1]
FORMAL_ROOT = ROOT / "third_party" / "riscv-formal"
SBY_ROOT = ROOT / "third_party" / "sby"
BUILD_ROOT = ROOT / "build" / "formal"
RUN_ROOT = BUILD_ROOT / "riscv-formal"
RESULTS = ROOT / "formal" / "results" / "results.json"
YOSYS = Path("/tmp/proofcore-tools/root/usr/bin/yosys")


def git_revision(path: Path) -> str:
    return subprocess.check_output(["git", "-C", str(path), "rev-parse", "HEAD"], text=True).strip()


def run(command: list[str], *, cwd: Path, env: dict[str, str], log: Path) -> tuple[int, float]:
    started = time.monotonic()
    with log.open("w", encoding="utf-8") as output:
        process = subprocess.run(command, cwd=cwd, env=env, stdout=output, stderr=subprocess.STDOUT)
    return process.returncode, time.monotonic() - started


def parse_sby_status(status_path: Path) -> str:
    if not status_path.exists():
        return "SETUP_ERROR"
    status = status_path.read_text(encoding="utf-8").strip().split()
    if not status:
        return "SETUP_ERROR"
    value = status[0].upper()
    return value if value in {"PASS", "FAIL", "UNKNOWN", "TIMEOUT", "ERROR"} else "UNKNOWN"


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--mode", choices=("insn",), default="insn")
    parser.add_argument("--check", help="Run one generated check target by name.")
    parser.add_argument("--negative-rvfi", action="store_true",
                        help="Corrupt RVFI destination data for addi and require the ISA check to fail.")
    args = parser.parse_args()

    for dependency in (FORMAL_ROOT, SBY_ROOT):
        if not (dependency / ".git").exists():
            subprocess.run(["git", "submodule", "update", "--init", str(dependency.relative_to(ROOT))],
                           cwd=ROOT, check=True)
    if not YOSYS.exists():
        print("SETUP_ERROR: expected bundled Yosys path is unavailable", file=sys.stderr)
        return 2

    if not RUN_ROOT.exists():
        shutil.copytree(FORMAL_ROOT, RUN_ROOT, ignore=shutil.ignore_patterns(".git", "__pycache__"))
    core_dir = RUN_ROOT / "cores" / "proofcore"
    core_dir.mkdir(parents=True, exist_ok=True)
    shutil.copy2(ROOT / "formal" / "rvfi" / "proofcore_rvfi_wrapper.sv", core_dir / "wrapper.sv")
    cfg = (ROOT / "formal" / "rvfi" / "checks.cfg").read_text(encoding="utf-8")
    cfg = cfg.replace("@REPO_ROOT@", ROOT.as_posix())
    if args.negative_rvfi:
        cfg = cfg.replace("`define RISCV_FORMAL_ALIGNED_MEM",
                          "`define RISCV_FORMAL_ALIGNED_MEM\n`define PROOFCORE_RVFI_NEGATIVE")
    selected_solver = next(line.split()[1] for line in cfg.splitlines()
                           if line.strip().startswith("solver "))
    (core_dir / "checks.cfg").write_text(cfg, encoding="utf-8")

    bin_dir = BUILD_ROOT / "bin"
    bin_dir.mkdir(parents=True, exist_ok=True)
    sby_launcher = bin_dir / "sby"
    sby_launcher.write_text(
        "#!/bin/sh\nexec python3 " + (SBY_ROOT / "sbysrc" / "sby.py").as_posix() + " \"$@\"\n",
        encoding="utf-8",
    )
    sby_launcher.chmod(0o755)

    env = os.environ.copy()
    env["PATH"] = os.pathsep.join((str(bin_dir), str(YOSYS.parent), env.get("PATH", "")))
    env["YOSYS"] = str(YOSYS)
    env["PYTHONPATH"] = os.pathsep.join(("/tmp/proofcore-tools/root/usr/share/yosys", env.get("PYTHONPATH", "")))

    rc, generate_time = run(
        [sys.executable, "../../checks/genchecks.py"], cwd=core_dir, env=env,
        log=BUILD_ROOT / "riscv-formal-generate.log")
    if rc != 0:
        print(f"SETUP_ERROR: riscv-formal check generation failed; see {BUILD_ROOT / 'riscv-formal-generate.log'}")
        return rc

    checks_dir = core_dir / "checks"
    if selected_solver == "bmc3":
        # ABC's BMC engine checks every frame; the riscv-formal generator's
        # `skip` option is supported only by smtbmc/btor and would be invalid.
        for sby_file in checks_dir.glob("*.sby"):
            lines = [line for line in sby_file.read_text(encoding="utf-8").splitlines()
                     if not line.strip().startswith("skip ")]
            sby_file.write_text("\n".join(lines) + "\n", encoding="utf-8")
    makefile = (checks_dir / "makefile").read_text(encoding="utf-8")
    targets = [line.split(":", 1)[0] for line in makefile.splitlines()
               if line and not line.startswith(("\t", ".")) and ":" in line
               and line.split(":", 1)[0] != "all" and "/" not in line.split(":", 1)[0]]
    targets = list(dict.fromkeys(targets))
    if args.check:
        targets = [target for target in targets if fnmatch.fnmatchcase(target, args.check)]
        if not targets:
            print(f"SETUP_ERROR: no generated check matches {args.check}")
            return 2
    else:
        targets = [target for target in targets if target.startswith("insn_") or target in {"reg_ch0", "pc_fwd_ch0", "pc_bwd_ch0", "unique_ch0", "cover_ch0"}]
    if args.negative_rvfi:
        targets = [target for target in targets if target == "insn_addi_ch0"]
        if not targets:
            print("SETUP_ERROR: generated addi check was not found", file=sys.stderr)
            return 2
        check_file = checks_dir / "insn_addi_ch0.sby"
        check_text = check_file.read_text(encoding="utf-8").replace("abc bmc3", "smtbmc z3")
        check_file.write_text(check_text, encoding="utf-8")

    run_records: list[dict[str, object]] = []
    failed_command = False
    for index, target in enumerate(targets):
        started = time.monotonic()
        check_rc, duration = run([str(sby_launcher), "-f", f"{target}.sby"],
                                 cwd=checks_dir, env=env,
                                 log=BUILD_ROOT / f"riscv-formal-{target}.log")
        status = parse_sby_status(checks_dir / target / "status")
        if check_rc != 0 and status == "PASS":
            status = "SETUP_ERROR"
        formal_status = status
        if args.negative_rvfi and status == "FAIL":
            status = "PASS"
        failed_command |= (check_rc != 0 and not (args.negative_rvfi and formal_status == "FAIL"))
        run_records.append({
            "name": f"negative-rvfi-{target}" if args.negative_rvfi else target,
            "category": "Negative-control self-test" if args.negative_rvfi else
                        ("ISA instruction" if target.startswith("insn_") else "RVFI consistency"),
            "mode": "bounded model checking",
            "depth": 12 if target != "cover_ch0" else 20,
            "solver": "smtbmc z3" if args.negative_rvfi else
                      ("ABC bmc3 bundled with Yosys" if selected_solver == "bmc3" else selected_solver),
            "status": status,
            **({"expected_status": "FAIL", "observed_status": formal_status} if args.negative_rvfi else {}),
            "runtime_seconds": round(duration, 3),
            "assumptions": ["single 32-byte instruction/data address window", "stable symbolic initial memory", "riscv-formal check constraints"],
            "artifact_path": str((checks_dir / target).relative_to(ROOT)),
            "log_path": str((BUILD_ROOT / f"riscv-formal-{target}.log").relative_to(ROOT)),
        })
        print(f"{status} {target} ({duration:.2f}s)")

    previous = json.loads(RESULTS.read_text(encoding="utf-8")) if RESULTS.exists() else {}
    current_names = {str(row["name"]) for row in run_records}
    preserved = [row for row in previous.get("checks", []) if row.get("name") not in current_names]
    payload = {
        "framework": "YosysHQ/riscv-formal",
        "framework_commit": git_revision(FORMAL_ROOT),
        "symbiyosys_commit": git_revision(SBY_ROOT),
        "yosys": subprocess.check_output([str(YOSYS), "-V"], text=True).strip(),
        "solver": "ABC bmc3 (part of " + subprocess.check_output([str(YOSYS), "-V"], text=True).strip() + ")"
                  if selected_solver == "bmc3" else selected_solver,
        "mode": "bounded model checking; no unbounded result is claimed",
        "generation_runtime_seconds": round(generate_time, 3),
        "checks": preserved + run_records,
    }
    RESULTS.parent.mkdir(parents=True, exist_ok=True)
    RESULTS.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    pass_count = sum(row["status"] == "PASS" for row in run_records)
    fail_count = sum(row["status"] == "FAIL" for row in run_records)
    print(f"riscv-formal: PASS={pass_count} FAIL={fail_count} total={len(run_records)}")
    expected_results = all(row["observed_status"] == "FAIL" for row in run_records) if args.negative_rvfi else \
        all(row["status"] == "PASS" for row in run_records)
    if args.negative_rvfi and expected_results:
        print("SELFTEST PASS: corrupted RVFI destination data was detected by insn_addi_ch0")
    return 1 if failed_command or not expected_results else 0


if __name__ == "__main__":
    raise SystemExit(main())
