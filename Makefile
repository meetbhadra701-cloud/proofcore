RTL := rtl/proofcore_pkg.sv \
	rtl/alu.sv rtl/imm_gen.sv rtl/regfile.sv rtl/decoder.sv \
	rtl/forwarding.sv rtl/hazard_unit.sv \
	rtl/pipeline/if_id.sv rtl/pipeline/id_ex.sv \
	rtl/pipeline/ex_mem.sv rtl/pipeline/mem_wb.sv rtl/core.sv
VERILATOR ?= $(shell command -v verilator 2>/dev/null || echo /tmp/proofcore-tools/root/usr/bin/verilator)
YOSYS ?= $(shell command -v yosys 2>/dev/null || echo /tmp/proofcore-tools/root/usr/bin/yosys)
VERILATOR_ROOT ?= $(shell if command -v verilator >/dev/null 2>&1; then verilator --getenv VERILATOR_ROOT; else echo /tmp/proofcore-tools/root/usr/share/verilator; fi)
export VERILATOR_ROOT

.PHONY: all lint lint-package test-units test-core test synth-check \
	verify-m1 reference-test diff-build diff-directed diff-random comparator-selftest \
	verify-m2 verify clean
all: verify

lint-package:
	$(VERILATOR) --lint-only --sv -Wall --top-module core $(RTL)

lint: lint-package

test-units test-core:
	python3 scripts/run_cocotb.py

test: test-units

synth-check:
	mkdir -p build
	PATH=$(dir $(YOSYS)):$$PATH $(YOSYS) -Q -p 'read_verilog -sv $(RTL); hierarchy -check -top core; proc; check; synth -top core; check; stat' > build/yosys.log 2>&1; rc=$$?; tail -n 45 build/yosys.log; exit $$rc

CXX ?= g++
CXXFLAGS_M2 := -std=c++20 -Wall -Wextra -Wpedantic -Werror -I$(CURDIR)/reference/include
CXXFLAGS_VERILATOR := $(CXXFLAGS_M2) -Wno-error=format
REFERENCE_SRC := reference/src/rv32i_decode.cpp reference/src/rv32i_model.cpp reference/src/memory.cpp
DIFF_SRC := sim/diff/main.cpp sim/diff/rtl_driver.cpp sim/diff/comparator.cpp \
	sim/diff/trace_writer.cpp sim/diff/case_file.cpp
DIFF_BIN := build/diff_obj/Vcore
M2_SEED ?= 12345
M2_PROGRAMS ?= 500

verify-m1: lint test synth-check

reference-test:
	mkdir -p build
	$(CXX) $(CXXFLAGS_M2) $(REFERENCE_SRC) reference/tests/reference_tests.cpp -o build/reference_tests
	build/reference_tests

diff-build:
	$(VERILATOR) --cc --exe --build --timing --top-module core --Mdir build/diff_obj \
		-CFLAGS "$(CXXFLAGS_VERILATOR)" $(RTL) $(REFERENCE_SRC) $(DIFF_SRC)

diff-directed: diff-build
	python3 tools/programgen/generate.py --m1-directed --out build/m2-directed
	$(DIFF_BIN) --case-dir build/m2-directed

diff-random: diff-build
	python3 tools/programgen/generate.py --count $(M2_PROGRAMS) --seed $(M2_SEED) --out build/m2-random
	$(DIFF_BIN) --case-dir build/m2-random

comparator-selftest: diff-build
	python3 tools/programgen/generate.py --selftest --seed $(M2_SEED) --out build/m2-selftest
	rm -rf build/m2-selftest-failures
	@if $(DIFF_BIN) --case-dir build/m2-selftest --selftest-corrupt-retire 2 --artifacts build/m2-selftest-failures; then \
		echo "ERROR: comparator self-test unexpectedly passed"; exit 1; \
	else code=$$?; test $$code -eq 1; fi
	test -f build/m2-selftest-failures/seed-$(M2_SEED)-cycle-*/failure.json
	echo "PASS: injected retirement corruption was detected and artifact was written"

verify-m2: reference-test diff-directed comparator-selftest diff-random

verify: verify-m1 verify-m2

clean:
	rm -rf build
