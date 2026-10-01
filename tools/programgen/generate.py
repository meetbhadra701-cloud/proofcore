#!/usr/bin/env python3
"""Generate reproducible, legal ProofCore M1 instruction images."""
from __future__ import annotations

import argparse
import json
from pathlib import Path


class Rng:
    """Small explicitly specified xorshift64* generator for stable seed replay."""

    def __init__(self, seed: int) -> None:
        self.state = seed & 0xFFFFFFFFFFFFFFFF
        if self.state == 0:
            self.state = 0x9E3779B97F4A7C15

    def next_u64(self) -> int:
        x = self.state
        x ^= x >> 12
        x ^= (x << 25) & 0xFFFFFFFFFFFFFFFF
        x ^= x >> 27
        self.state = x & 0xFFFFFFFFFFFFFFFF
        return (self.state * 0x2545F4914F6CDD1D) & 0xFFFFFFFFFFFFFFFF

    def pick(self, values):
        return values[self.next_u64() % len(values)]


def enc_i(opcode: int, rd: int, funct3: int, rs1: int, immediate: int) -> int:
    return (((immediate & 0xFFF) << 20) | (rs1 << 15) | (funct3 << 12) |
            (rd << 7) | opcode) & 0xFFFFFFFF


def enc_r(rd: int, funct3: int, rs1: int, rs2: int, funct7: int = 0) -> int:
    return ((funct7 << 25) | (rs2 << 20) | (rs1 << 15) |
            (funct3 << 12) | (rd << 7) | 0x33) & 0xFFFFFFFF


def enc_s(funct3: int, rs1: int, rs2: int, immediate: int) -> int:
    imm = immediate & 0xFFF
    return (((imm >> 5) << 25) | (rs2 << 20) | (rs1 << 15) |
            (funct3 << 12) | ((imm & 0x1F) << 7) | 0x23) & 0xFFFFFFFF


def enc_b(funct3: int, rs1: int, rs2: int, immediate: int) -> int:
    imm = immediate & 0x1FFF
    return (((imm >> 12 & 1) << 31) | ((imm >> 5 & 0x3F) << 25) |
            (rs2 << 20) | (rs1 << 15) | (funct3 << 12) |
            ((imm >> 1 & 0xF) << 8) | ((imm >> 11 & 1) << 7) | 0x63) & 0xFFFFFFFF


def enc_j(rd: int, immediate: int) -> int:
    imm = immediate & 0x1FFFFF
    return (((imm >> 20 & 1) << 31) | ((imm >> 1 & 0x3FF) << 21) |
            ((imm >> 11 & 1) << 20) | ((imm >> 12 & 0xFF) << 12) |
            (rd << 7) | 0x6F) & 0xFFFFFFFF


def lui(rd: int, upper20: int) -> int:
    return ((upper20 & 0xFFFFF) << 12) | (rd << 7) | 0x37


def init_base(words: list[int]) -> None:
    words.append(lui(10, 1))  # x10 = 0x1000, initialized data region


def tier_arithmetic(rng: Rng) -> list[int]:
    w: list[int] = []
    w += [lui(1, 0x80000), enc_i(0x13, 2, 0, 0, -1), enc_i(0x13, 3, 0, 0, 1)]
    r_ops = [(0, 0), (0, 0x20), (1, 0), (2, 0), (3, 0), (4, 0), (5, 0),
             (5, 0x20), (6, 0), (7, 0)]
    for index, (funct3, funct7) in enumerate(r_ops):
        rd = 4 + index
        w.append(enc_r(rd, funct3, rng.pick([1, 2, 3]), rng.pick([1, 2, 3]), funct7))
    i_ops = [(0, 13), (2, -1), (3, -1), (4, 0x55), (6, 0x35), (7, 0x55),
             (1, 31), (5, 31), (5, 0x401)]
    for index, (funct3, imm) in enumerate(i_ops):
        rd = 15 + index
        w.append(enc_i(0x13, rd, funct3, rng.pick([1, 2, 3]), imm))
    # More random register ALU operations retain deterministic edge operands.
    for _ in range(8):
        funct3, funct7 = rng.pick(r_ops)
        w.append(enc_r(rng.pick(range(1, 32)), funct3, rng.pick([1, 2, 3]),
                       rng.pick([1, 2, 3]), funct7))
    return w


def tier_dependencies(rng: Rng) -> list[int]:
    w = [enc_i(0x13, 1, 0, 0, rng.pick([1, 7, -1, 31]))]
    # Consecutive writes to the same destination exercise youngest-producer priority.
    w += [enc_i(0x13, 1, 0, 1, 1), enc_i(0x13, 1, 0, 1, 2),
          enc_r(2, 0, 1, 1), enc_r(3, 0, 2, 1)]
    for _ in range(10):
        destination = rng.pick(range(1, 16))
        source = rng.pick([1, 2, 3, destination])
        w.append(enc_i(0x13, destination, 0, source, rng.pick([-16, -1, 0, 1, 15])))
        w.append(enc_r(rng.pick(range(16, 32)), rng.pick([0, 1, 2, 3, 4, 5, 6, 7]),
                       destination, rng.pick([1, 2, 3])))
    # Explicit x0 attempted write and false dependency cases.
    w += [enc_i(0x13, 0, 0, 0, 123), enc_r(4, 0, 0, 1)]
    return w


def tier_memory(rng: Rng) -> list[int]:
    w: list[int] = []
    init_base(w)
    w += [enc_i(0x13, 1, 0, 0, rng.pick([-128, 0x55, -1])),
          enc_i(0x13, 2, 0, 0, rng.pick([0x123, -7, 0x7F]))]
    # Every width and signedness; all effective addresses are naturally aligned.
    w += [enc_i(0x03, 3, 0, 10, 0), enc_i(0x03, 4, 4, 10, 1),
          enc_i(0x03, 5, 1, 10, 2), enc_i(0x03, 6, 5, 10, 2),
          enc_i(0x03, 7, 2, 10, 4)]
    w += [enc_s(0, 10, 1, 8), enc_i(0x03, 8, 0, 10, 8),
          enc_i(0x03, 9, 4, 10, 8), enc_s(1, 10, 2, 10),
          enc_i(0x03, 11, 1, 10, 10), enc_i(0x03, 12, 5, 10, 10),
          enc_s(2, 10, 2, 12), enc_i(0x03, 13, 2, 10, 12)]
    # Negative S-immediate path with a known aligned effective address.
    w += [enc_i(0x13, 14, 0, 10, 16), enc_s(2, 14, 2, -4),
          enc_i(0x03, 15, 2, 10, 12)]
    for _ in range(4):
        offset = rng.pick([0, 4, 8, 12, 16, 20])
        w.append(enc_i(0x03, rng.pick(range(16, 32)), 2, 10, offset))
    return w


def tier_load_use(rng: Rng) -> list[int]:
    w = [lui(10, 1), enc_i(0x13, 2, 0, 0, rng.pick([7, -1, 31]))]
    w += [enc_i(0x03, 5, 2, 10, 0), enc_r(6, 0, 5, 2),  # rs1 consumer
          enc_i(0x03, 7, 2, 10, 4), enc_r(8, 0, 2, 7),  # rs2 consumer
          enc_i(0x03, 9, 2, 10, 8), enc_r(11, 0, 9, 9), # both operands
          enc_i(0x03, 0, 2, 10, 12), enc_r(12, 0, 0, 2)]
    return w


def tier_control(rng: Rng) -> list[int]:
    w = [enc_i(0x13, 1, 0, 0, 1), enc_i(0x13, 2, 0, 0, 1)]
    branch_ops = [0, 1, 4, 5, 6, 7]
    for index, funct3 in enumerate(branch_ops):
        # Alternating true/false outcomes, with a wrong-path register write
        # between branch and target for every predicate.
        left, right = (1, 1) if funct3 in (0, 5, 7) else (1, 2)
        if left == 1 and right == 2:
            w.append(enc_i(0x13, 2, 0, 0, 2))
        w.append(enc_b(funct3, 1, right, 8))
        w.append(enc_i(0x13, 20 + index, 0, 0, 99))
        w.append(enc_i(0x13, 20 + index, 0, 0, index + 1))
    # JAL with a wrong-path write.
    w += [enc_j(0, 8), enc_i(0x13, 15, 0, 0, 99), enc_i(0x13, 15, 0, 0, 7)]
    # JALR target is absolute, aligned, and jumps over a poison instruction.
    jalr_pc = len(w) * 4
    target_pc = jalr_pc + 12
    w += [enc_i(0x13, 16, 0, 0, target_pc | 1), enc_i(0x67, 17, 0, 16, 0),
          enc_i(0x13, 18, 0, 0, 99), enc_i(0x13, 18, 0, 0, 7)]
    # A bounded backwards loop: decrement once per pass and exit at zero.
    loop_count = rng.pick([1, 2, 3, 4])
    w += [enc_i(0x13, 19, 0, 0, loop_count)]
    loop_pc = len(w) * 4
    w += [enc_i(0x13, 19, 0, 19, -1), enc_b(1, 19, 0, loop_pc - (loop_pc + 4))]
    return w


def tier_mixed(rng: Rng) -> list[int]:
    w: list[int] = []
    init_base(w)
    w += [enc_i(0x13, 1, 0, 0, rng.pick([4, 9, -3])), enc_i(0x13, 2, 0, 1, 3),
          enc_s(2, 10, 2, 0), enc_i(0x03, 3, 2, 10, 0), enc_r(4, 0, 3, 1),
          enc_i(0x03, 5, 2, 10, 4), enc_r(6, 0, 5, 4)]
    # Branch decision depends on recent arithmetic/load data. Target skips poison.
    w += [enc_b(0, 6, 6, 8), enc_s(2, 10, 2, 16), enc_i(0x13, 7, 0, 0, 11)]
    w += [enc_j(0, 8), enc_i(0x13, 8, 0, 0, 99), enc_i(0x13, 9, 0, 0, 13)]
    for _ in range(4):
        rd = rng.pick(range(10, 20))
        w.append(enc_i(0x13, rd, 0, rng.pick([1, 2, 4, 6, 9]), rng.pick([-8, -1, 1, 7])))
    return w


GENERATORS = [tier_arithmetic, tier_dependencies, tier_memory,
              tier_load_use, tier_control, tier_mixed]
TIER_NAMES = ["A-arithmetic", "B-dependencies", "C-memory", "D-load-use",
              "E-control", "F-mixed"]


def generate_case(seed: int, index: int) -> tuple[str, list[int]]:
    tier_index = index % len(GENERATORS)
    words = GENERATORS[tier_index](Rng(seed ^ (0xD1B54A32D192ED03 * (index + 1))))
    return TIER_NAMES[tier_index], words


def write_corpus(output: Path, count: int, seed: int) -> dict:
    output.mkdir(parents=True, exist_ok=True)
    for old in output.glob("*.case"):
        old.unlink()
    inventory = []
    for index in range(count):
        case_seed = (seed + index) & 0xFFFFFFFFFFFFFFFF
        tier, words = generate_case(case_seed, index)
        path = output / f"case_{index:04d}.case"
        with path.open("w", encoding="ascii") as stream:
            stream.write(f"seed {case_seed}\n")
            stream.write(f"tier {tier}\n")
            stream.write(f"stop_pc {len(words) * 4:x}\n")
            stream.write("instructions\n")
            for word in words:
                stream.write(f"{word:08x}\n")
        inventory.append({"case": path.name, "seed": case_seed, "tier": tier,
                          "instructions": len(words)})
    distribution = {}
    for item in inventory:
        tier = item["tier"]
        if tier not in distribution:
            distribution[tier] = {"programs": 0, "static_instructions": 0}
        distribution[tier]["programs"] += 1
        distribution[tier]["static_instructions"] += item["instructions"]
    manifest = {"algorithm": "xorshift64*; words encoded by Python integer helpers",
                "base_seed": seed, "programs": count, "distribution": distribution,
                "cases": inventory}
    (output / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    return manifest


def write_case(path: Path, seed: int, tier: str, words: list[int], memory=None) -> None:
    with path.open("w", encoding="ascii") as stream:
        stream.write(f"seed {seed}\ntier {tier}\nstop_pc {len(words) * 4:x}\ninstructions\n")
        for word in words:
            stream.write(f"{word:08x}\n")
        if memory:
            stream.write("memory\n")
            for address, value in sorted(memory.items()):
                stream.write(f"{address:x} {value & 0xFF:x}\n")


def write_m1_directed(output: Path) -> dict:
    """Port the nine M1 Cocotb scenarios into machine-code differential cases."""
    output.mkdir(parents=True, exist_ok=True)
    for old in output.glob("*.case"):
        old.unlink()

    alu = [lui(1, 0x80000), enc_i(0x13, 2, 0, 0, -1),
           enc_r(3, 0, 1, 2), enc_r(4, 0, 2, 1, 0x20),
           enc_r(5, 1, 1, 2), enc_r(6, 2, 1, 2), enc_r(7, 3, 1, 2),
           enc_r(8, 4, 1, 2), enc_r(9, 5, 1, 2), enc_r(10, 5, 1, 2, 0x20),
           enc_r(11, 6, 1, 2), enc_r(12, 7, 1, 2), enc_i(0x13, 13, 0, 0, -2048),
           enc_i(0x13, 14, 2, 1, -1), enc_i(0x13, 15, 3, 1, -1),
           enc_i(0x13, 16, 4, 1, -1), enc_i(0x13, 17, 6, 1, 0x7ff),
           enc_i(0x13, 18, 7, 1, -1), enc_i(0x13, 19, 1, 2, 0),
           enc_i(0x13, 20, 5, 2, 31), enc_i(0x13, 21, 5, 2, 0x401),
           lui(22, 0x12345), (1 << 12) | (23 << 7) | 0x17,
           enc_i(0x13, 24, 0, 0, 1), enc_r(25, 2, 1, 24), enc_r(26, 3, 1, 24)]
    forward = [enc_i(0x13, 1, 0, 0, 1), enc_i(0x13, 1, 0, 1, 1),
               enc_r(2, 0, 1, 1), enc_i(0x13, 0, 0, 1, 99), enc_r(3, 0, 0, 1)]
    load_use = [enc_i(0x13, 1, 0, 0, 128), enc_i(0x03, 2, 2, 1, 0),
                enc_r(3, 0, 2, 2), enc_i(0x13, 4, 0, 2, 1),
                enc_i(0x03, 0, 2, 1, 0), enc_r(5, 0, 0, 0)]
    control = [enc_i(0x13, 1, 0, 0, 1), enc_i(0x13, 2, 0, 0, 1),
               enc_b(0, 1, 2, 8), enc_i(0x13, 3, 0, 0, 99),
               enc_b(1, 1, 2, 8), enc_b(4, 1, 2, 8), enc_b(5, 1, 2, 8),
               enc_s(2, 0, 1, 0), enc_b(6, 1, 2, 8), enc_b(7, 1, 2, 8),
               enc_i(0x13, 3, 0, 0, 88), enc_j(4, 8), enc_i(0x13, 5, 0, 0, 77),
               enc_i(0x13, 6, 0, 0, 9), enc_i(0x13, 7, 0, 0, 65),
               enc_i(0x67, 8, 0, 7, 0), enc_i(0x13, 9, 0, 0, 3)]
    branches = [enc_i(0x13, 1, 0, 0, -1), enc_i(0x13, 2, 0, 0, 1),
                enc_b(0, 1, 2, 8), enc_i(0x13, 3, 0, 0, 3), enc_b(1, 1, 2, 8),
                enc_i(0x13, 4, 0, 0, 88), enc_b(4, 1, 2, 8),
                enc_i(0x13, 5, 0, 0, 88), enc_b(5, 1, 2, 8),
                enc_i(0x13, 6, 0, 0, 6), enc_b(6, 1, 2, 8),
                enc_i(0x13, 7, 0, 0, 7), enc_b(7, 1, 2, 8),
                enc_i(0x13, 8, 0, 0, 88), enc_i(0x13, 9, 0, 0, 9)]
    backward = [enc_i(0x13, 1, 0, 0, 2), enc_i(0x13, 1, 0, 1, -1),
                enc_b(1, 1, 0, -4), enc_i(0x13, 2, 0, 0, 7)]
    load_branch = [enc_i(0x13, 1, 0, 0, 128), enc_i(0x03, 2, 2, 1, 0),
                   enc_b(0, 2, 0, 8), enc_i(0x13, 3, 0, 0, 88),
                   enc_i(0x13, 4, 0, 0, 5)]
    memory = [enc_i(0x13, 1, 0, 0, 128), enc_i(0x03, 2, 0, 1, 0),
              enc_i(0x03, 3, 4, 1, 0), enc_i(0x03, 4, 1, 1, 0),
              enc_i(0x03, 5, 5, 1, 2), enc_i(0x03, 8, 1, 1, 2),
              enc_i(0x03, 6, 2, 1, 0), enc_i(0x13, 7, 0, 0, -86),
              enc_s(0, 1, 7, 9), enc_s(1, 1, 7, 10), enc_s(2, 1, 6, 36),
              enc_s(0, 1, 7, -1)]
    subset = [0xffffffff, enc_i(0x13, 1, 0, 0, 128), enc_i(0x03, 2, 2, 1, 1),
              enc_s(1, 1, 1, 1), enc_b(0, 0, 0, 2), enc_i(0x13, 3, 0, 0, 7)]

    zero_load = {128: 0x80, 129: 0x7f, 130: 0, 131: 0x80}
    zero_branch = {128: 0, 129: 0, 130: 0, 131: 0}
    memory_load = {128: 0x80, 129: 0x7f, 130: 0, 131: 0x80}
    cases = [
        ("M1-alu", alu, {}), ("M1-forwarding", forward, {}),
        ("M1-load-use", load_use, zero_load), ("M1-control-jumps-flush", control, {}),
        ("M1-branch-predicates", branches, {}), ("M1-negative-branch-loop", backward, {}),
        ("M1-load-to-branch", load_branch, zero_branch),
        ("M1-memory-subwords", memory, memory_load),
        ("M1-unsupported-misaligned", subset, {128: 0x11, 129: 0x22, 130: 0x33, 131: 0x44}),
    ]
    inventory = []
    for index, (name, words, initial) in enumerate(cases):
        case_seed = 0x4D310000 + index
        path = output / f"case_{index:04d}.case"
        write_case(path, case_seed, name, words, initial)
        inventory.append({"case": path.name, "seed": case_seed, "tier": name,
                          "instructions": len(words)})
    manifest = {"source": "directed translations of tb/cocotb/test_core.py scenarios",
                "programs": len(cases), "cases": inventory}
    (output / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    return manifest


def write_selftest_case(output: Path, seed: int) -> dict:
    output.mkdir(parents=True, exist_ok=True)
    for old in output.glob("*.case"):
        old.unlink()
    words = [enc_i(0x13, 5, 0, 0, 7), enc_r(6, 0, 5, 5)]
    path = output / "case_0000.case"
    write_case(path, seed, "comparator-selftest", words)
    manifest = {"source": "fixed self-test: addi x5,x0,7; add x6,x5,x5",
                "programs": 1, "cases": [{"case": path.name, "seed": seed,
                                              "tier": "comparator-selftest", "instructions": 2}]}
    (output / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    return manifest


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--count", type=int, default=500)
    parser.add_argument("--seed", type=int, default=12345)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--m1-directed", action="store_true")
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args()
    if args.m1_directed and args.selftest:
        parser.error("--m1-directed and --selftest are mutually exclusive")
    if args.count < 1 and not args.m1_directed:
        parser.error("--count must be positive")
    if args.selftest:
        manifest = write_selftest_case(args.out, args.seed)
        print(f"generated comparator self-test programs={manifest['programs']} seed={args.seed} out={args.out}")
    elif args.m1_directed:
        manifest = write_m1_directed(args.out)
        print(f"generated M1 directed programs={manifest['programs']} out={args.out}")
    else:
        manifest = write_corpus(args.out, args.count, args.seed)
        print(f"generated programs={manifest['programs']} base_seed={args.seed} out={args.out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
