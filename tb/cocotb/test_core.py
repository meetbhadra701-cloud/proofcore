import cocotb
from cocotb.triggers import Timer


def enc_i(op, rd, f3, rs1, imm):
    return ((imm & 0xFFF) << 20) | (rs1 << 15) | (f3 << 12) | (rd << 7) | op


def enc_r(rd, f3, rs1, rs2, f7=0):
    return (f7 << 25) | (rs2 << 20) | (rs1 << 15) | (f3 << 12) | (rd << 7) | 0x33


def enc_s(f3, rs1, rs2, imm):
    imm &= 0xFFF
    return ((imm >> 5) << 25) | (rs2 << 20) | (rs1 << 15) | (f3 << 12) | ((imm & 0x1F) << 7) | 0x23


def enc_b(f3, rs1, rs2, imm):
    imm &= 0x1FFF
    return (((imm >> 12) & 1) << 31) | (((imm >> 5) & 0x3F) << 25) | (rs2 << 20) | (rs1 << 15) | (f3 << 12) | (((imm >> 1) & 0xF) << 8) | (((imm >> 11) & 1) << 7) | 0x63


def enc_u(op, rd, upper20):
    return ((upper20 & 0xFFFFF) << 12) | (rd << 7) | op


def enc_j(rd, imm):
    imm &= 0x1FFFFF
    return (((imm >> 20) & 1) << 31) | (((imm >> 1) & 0x3FF) << 21) | (((imm >> 11) & 1) << 20) | (((imm >> 12) & 0xFF) << 12) | (rd << 7) | 0x6F


def addi(rd, rs1, imm): return enc_i(0x13, rd, 0, rs1, imm)
def load(rd, rs1, imm, f3): return enc_i(0x03, rd, f3, rs1, imm)


class Bench:
    def __init__(self, dut):
        self.dut = dut
        self.imem = {}
        self.mem = bytearray(4096)
        self.retirements = []
        self.cycles = 0

    async def tick(self):
        d = self.dut
        d.clk.value = 0
        await Timer(1, unit="ns")
        pc = int(d.imem_addr.value)
        d.imem_rdata.value = self.imem.get(pc, 0xFFFFFFFF)
        addr = int(d.dmem_addr.value) & 0xFFF
        word = int.from_bytes(self.mem[addr:addr + 4], "little")
        d.dmem_rdata.value = word
        await Timer(1, unit="ns")

        # WB outputs describe the instruction committing at this rising edge.
        if (not int(d.reset.value)) and int(d.retire_valid.value):
            self.retirements.append({
                "pc": int(d.retire_pc.value),
                "insn": int(d.retire_insn.value),
                "rd": int(d.retire_rd.value),
                "rd_write": int(d.retire_rd_write.value),
                "rd_value": int(d.retire_rd_value.value),
                "mem_addr": int(d.retire_mem_addr.value),
                "rmask": int(d.retire_mem_read_mask.value),
                "wmask": int(d.retire_mem_write_mask.value),
                "wdata": int(d.retire_mem_write_data.value),
                "next_pc": int(d.retire_next_pc.value),
            })

        mask = int(d.dmem_write_mask.value)
        wdata = int(d.dmem_write_data.value)
        if mask:
            for lane in range(4):
                if mask & (1 << lane):
                    self.mem[addr + lane] = (wdata >> (8 * lane)) & 0xFF

        d.clk.value = 1
        await Timer(1, unit="ns")
        self.cycles += 1

    async def reset(self, program, initial_memory=None):
        d = self.dut
        self.imem = {4 * i: word & 0xFFFFFFFF for i, word in enumerate(program)}
        self.mem = bytearray(4096)
        if initial_memory:
            for addr, value in initial_memory.items():
                self.mem[addr] = value
        self.retirements = []
        self.cycles = 0
        d.clk.value = 0
        d.reset.value = 1
        d.imem_rdata.value = 0xFFFFFFFF
        d.dmem_rdata.value = 0
        for _ in range(2):
            await self.tick()
        d.reset.value = 0
        self.retirements = []
        self.cycles = 0

    async def run_through(self, final_pc, max_cycles=300):
        for _ in range(max_cycles):
            await self.tick()
            if any(r["pc"] == final_pc for r in self.retirements):
                return
        raise AssertionError(f"instruction at {final_pc:#x} did not retire within {max_cycles} cycles")


def retire_pcs(b):
    return [r["pc"] for r in b.retirements]


def writes(b):
    return {r["rd"]: r["rd_value"] for r in b.retirements if r["rd_write"]}


@cocotb.test()
async def rv32i_foundation_regression(dut):
    b = Bench(dut)

    # All ten register ALU operations, immediate operations, signed edges, and shift boundaries.
    program = [
        enc_u(0x37, 1, 0x80000),                 # x1 = 0x80000000
        addi(2, 0, -1),                          # x2 = 0xffffffff
        enc_r(3, 0, 1, 2),                       # ADD
        enc_r(4, 0, 2, 1, 0x20),                 # SUB
        enc_r(5, 1, 1, 2),                       # SLL
        enc_r(6, 2, 1, 2),                       # SLT
        enc_r(7, 3, 1, 2),                       # SLTU
        enc_r(8, 4, 1, 2),                       # XOR
        enc_r(9, 5, 1, 2),                       # SRL
        enc_r(10, 5, 1, 2, 0x20),                # SRA
        enc_r(11, 6, 1, 2),                      # OR
        enc_r(12, 7, 1, 2),                      # AND
        addi(13, 0, -2048),                      # negative immediate boundary
        enc_i(0x13, 14, 2, 1, -1),               # SLTI
        enc_i(0x13, 15, 3, 1, -1),               # SLTIU
        enc_i(0x13, 16, 4, 1, -1),               # XORI
        enc_i(0x13, 17, 6, 1, 0x7FF),             # ORI
        enc_i(0x13, 18, 7, 1, -1),               # ANDI
        enc_i(0x13, 19, 1, 2, 0),                # SLLI by zero
        enc_i(0x13, 20, 5, 2, 31),               # SRLI by 31
        enc_i(0x13, 21, 5, 2, (0x20 << 5) | 31), # SRAI by 31
        enc_u(0x37, 22, 0x12345),                # LUI
        enc_u(0x17, 23, 1),                      # AUIPC at PC 88
        addi(24, 0, 1),                          # comparison operand
        enc_r(25, 2, 1, 24),                     # SLT: signed negative < positive
        enc_r(26, 3, 1, 24),                     # SLTU: unsigned high bit set > 1
    ]
    await b.reset(program)
    await b.run_through((len(program) - 1) * 4)
    result = writes(b)
    expected = {
        1: 0x80000000, 2: 0xFFFFFFFF, 3: 0x7FFFFFFF, 4: 0x7FFFFFFF,
        5: 0, 6: 1, 7: 1, 8: 0x7FFFFFFF, 9: 1, 10: 0xFFFFFFFF,
        11: 0xFFFFFFFF, 12: 0x80000000, 13: 0xFFFFF800, 14: 1,
        15: 1, 16: 0x7FFFFFFF, 17: 0x800007FF, 18: 0x80000000,
        19: 0xFFFFFFFF, 20: 1, 21: 0xFFFFFFFF, 22: 0x12345000, 23: 88 + 0x1000,
        24: 1, 25: 1, 26: 0,
    }
    assert result == expected, f"ALU/register results differ: {result!r}"
    assert retire_pcs(b) == [4 * i for i in range(len(program))]
    assert b.cycles == len(program) + 4, "independent arithmetic should retire without stalls"

    # EX/MEM priority over MEM/WB, both operands, dependency chain, and x0 write suppression.
    program = [
        addi(1, 0, 1),
        addi(1, 1, 1),
        enc_r(2, 0, 1, 1),
        enc_i(0x13, 0, 0, 1, 99),
        enc_r(3, 0, 0, 1),
    ]
    await b.reset(program)
    await b.run_through(16)
    result = writes(b)
    assert result == {1: 2, 2: 4, 3: 2}, result
    assert [r["rd"] for r in b.retirements if r["insn"] == program[3]] == [0]
    assert b.cycles == len(program) + 4

    # Load-use on both operands, load-use on rs1, and x0 false dependency.
    program = [
        addi(1, 0, 128),
        load(2, 1, 0, 2),
        enc_r(3, 0, 2, 2),
        addi(4, 2, 1),
        load(0, 1, 0, 2),
        enc_r(5, 0, 0, 0),
    ]
    await b.reset(program, {128: 0x80, 129: 0x7F, 130: 0, 131: 0x80})
    await b.run_through(20)
    result = writes(b)
    assert result == {1: 128, 2: 0x80007F80, 3: 0x0000FF00, 4: 0x80007F81, 5: 0}, result
    assert b.cycles == len(program) + 4 + 1, f"expected one true load-use stall and no x0 false stall, got {b.cycles} cycles"

    # Branch conditions, branch forwarding, taken flush, JAL/JALR and link values.
    program = [
        addi(1, 0, 1),                         # 0
        addi(2, 0, 1),                         # 4
        enc_b(0, 1, 2, 8),                    # 8 BEQ taken -> 16
        addi(3, 0, 99),                        # 12 wrong path
        enc_b(1, 1, 2, 8),                    # 16 BNE not taken
        enc_b(4, 1, 2, 8),                    # 20 BLT not taken
        enc_b(5, 1, 2, 8),                    # 24 BGE taken -> 32
        enc_s(2, 0, 1, 0),                    # 28 wrong-path store
        enc_b(6, 1, 2, 8),                    # 32 BLTU not taken
        enc_b(7, 1, 2, 8),                    # 36 BGEU taken -> 44
        addi(3, 0, 88),                        # 40 wrong path
        enc_j(4, 8),                           # 44 JAL -> 52, link x4=48
        addi(5, 0, 77),                        # 48 wrong path
        addi(6, 0, 9),                         # 52
        addi(7, 0, 65),                        # 56; JALR bit zero is cleared
        enc_i(0x67, 8, 0, 7, 0),              # 60 JALR -> 64, link x8=64
        addi(9, 0, 3),                         # 64
    ]
    await b.reset(program)
    await b.run_through(64)
    expected_pcs = [0, 4, 8, 16, 20, 24, 32, 36, 44, 52, 56, 60, 64]
    assert retire_pcs(b) == expected_pcs, retire_pcs(b)
    result = writes(b)
    assert result[4] == 48 and result[8] == 64 and result[9] == 3, result
    assert result.get(3) is None and result.get(5) is None, result
    assert b.mem[0:4] == b"\0\0\0\0", "wrong-path store changed memory"
    assert next(r for r in b.retirements if r["pc"] == 8)["next_pc"] == 16
    assert next(r for r in b.retirements if r["pc"] == 24)["next_pc"] == 32
    assert next(r for r in b.retirements if r["pc"] == 44)["next_pc"] == 52

    # Exercise both outcomes of all six branch predicates with signed/unsigned disagreement.
    program = [
        addi(1, 0, -1),
        addi(2, 0, 1),
        enc_b(0, 1, 2, 8),                    # BEQ false -> 12
        addi(3, 0, 3),
        enc_b(1, 1, 2, 8),                    # BNE true -> 24
        addi(4, 0, 88),                        # flushed
        enc_b(4, 1, 2, 8),                    # BLT true -> 32
        addi(5, 0, 88),                        # flushed
        enc_b(5, 1, 2, 8),                    # BGE false -> 36
        addi(6, 0, 6),
        enc_b(6, 1, 2, 8),                    # BLTU false -> 44
        addi(7, 0, 7),
        enc_b(7, 1, 2, 8),                    # BGEU true -> 56
        addi(8, 0, 88),                        # flushed
        addi(9, 0, 9),
    ]
    await b.reset(program)
    await b.run_through(56)
    expected_pcs = [0, 4, 8, 12, 16, 24, 32, 36, 40, 44, 48, 56]
    assert retire_pcs(b) == expected_pcs, retire_pcs(b)
    result = writes(b)
    assert result[3] == 3 and result[6] == 6 and result[7] == 7 and result[9] == 9, result
    assert 4 not in result and 5 not in result and 8 not in result, result
    assert {pc: next(r for r in b.retirements if r["pc"] == pc)["next_pc"]
            for pc in (8, 16, 24, 32, 40, 48)} == {
                8: 12, 16: 24, 24: 32, 32: 36, 40: 44, 48: 56,
            }

    # Negative B-immediate: loop backward twice, then fall through.
    program = [addi(1, 0, 2), addi(1, 1, -1), enc_b(1, 1, 0, -4), addi(2, 0, 7)]
    await b.reset(program)
    await b.run_through(12)
    assert retire_pcs(b) == [0, 4, 8, 4, 8, 12], retire_pcs(b)
    assert writes(b) == {1: 0, 2: 7}

    # Load-to-branch dependency stalls once; forwarding then resolves and flushes.
    program = [
        addi(1, 0, 128),
        load(2, 1, 0, 2),
        enc_b(0, 2, 0, 8),
        addi(3, 0, 88),                        # flushed
        addi(4, 0, 5),
    ]
    await b.reset(program, {128: 0, 129: 0, 130: 0, 131: 0})
    await b.run_through(16)
    assert retire_pcs(b) == [0, 4, 8, 16], retire_pcs(b)
    assert writes(b) == {1: 128, 2: 0, 4: 5}
    assert b.cycles == len(b.retirements) + 4 + 1 + 2, "load-use stall and EX redirect bubbles were not modeled"

    # Loads, stores, lane masks, sign/zero extension, and little-endian data.
    program = [
        addi(1, 0, 128),
        load(2, 1, 0, 0),                     # LB
        load(3, 1, 0, 4),                     # LBU
        load(4, 1, 0, 1),                     # LH
        load(5, 1, 2, 5),                     # LHU
        load(8, 1, 2, 1),                     # LH sign extension from bit 15
        load(6, 1, 0, 2),                     # LW
        addi(7, 0, -86),
        enc_s(0, 1, 7, 9),                    # SB at lane 1
        enc_s(1, 1, 7, 10),                   # SH at lanes 2,3
        enc_s(2, 1, 6, 36),                   # SW at address 164
        enc_s(0, 1, 7, -1),                   # SB with a negative S-immediate
    ]
    initial = {128: 0x80, 129: 0x7F, 130: 0x00, 131: 0x80}
    await b.reset(program, initial)
    await b.run_through(44)
    result = writes(b)
    assert {k: result[k] for k in (2, 3, 4, 5, 6, 8)} == {
        2: 0xFFFFFF80, 3: 0x80, 4: 0x7F80, 5: 0x8000, 6: 0x80007F80, 8: 0xFFFF8000,
    }, result
    stores = {r["pc"]: r for r in b.retirements if r["wmask"]}
    assert stores[32]["wmask"] == 0b0010 and stores[32]["wdata"] == 0x0000AA00
    assert stores[36]["wmask"] == 0b1100 and stores[36]["wdata"] == 0xFFAA0000
    assert stores[40]["wmask"] == 0b1111 and stores[40]["wdata"] == 0x80007F80
    assert stores[44]["wmask"] == 0b1000 and stores[44]["wdata"] == 0xAA000000
    assert b.mem[127] == 0xAA and b.mem[137] == 0xAA and b.mem[138:140] == bytes((0xAA, 0xFF))
    assert int.from_bytes(b.mem[164:168], "little") == 0x80007F80
    loads = {r["pc"]: r for r in b.retirements if r["rmask"]}
    assert [loads[pc]["rmask"] for pc in (4, 8, 12, 16, 20, 24)] == [1, 1, 3, 12, 12, 15]
    assert all(r["wdata"] == 0 for r in b.retirements if r["wmask"] == 0)

    # Unsupported word, misaligned data operations, and misaligned taken target are inert.
    program = [
        0xFFFFFFFF,
        addi(1, 0, 128),
        load(2, 1, 1, 2),                     # misaligned LW: no rd update
        enc_s(1, 1, 1, 1),                    # misaligned SH: no memory update
        enc_b(0, 0, 0, 2),                    # taken but 2-byte target: suppress transfer
        addi(3, 0, 7),                         # executes sequentially
    ]
    await b.reset(program, {128: 0x11, 129: 0x22, 130: 0x33, 131: 0x44})
    await b.run_through(20)
    result = writes(b)
    assert result == {1: 128, 3: 7}, result
    assert retire_pcs(b) == [0, 4, 8, 12, 16, 20]
    assert b.mem[128:132] == bytes((0x11, 0x22, 0x33, 0x44))
    assert all(r["rd"] == 0 and r["rd_value"] == 0 for r in b.retirements if r["pc"] in (0, 8, 12, 16))
