RTL := rtl/proofcore_pkg.sv \
	rtl/alu.sv rtl/imm_gen.sv rtl/regfile.sv rtl/decoder.sv \
	rtl/forwarding.sv rtl/hazard_unit.sv \
	rtl/pipeline/if_id.sv rtl/pipeline/id_ex.sv \
	rtl/pipeline/ex_mem.sv rtl/pipeline/mem_wb.sv rtl/core.sv
VERILATOR ?= $(shell command -v verilator 2>/dev/null || echo /tmp/proofcore-tools/root/usr/bin/verilator)
YOSYS ?= $(shell command -v yosys 2>/dev/null || echo /tmp/proofcore-tools/root/usr/bin/yosys)
VERILATOR_ROOT ?= $(shell if command -v verilator >/dev/null 2>&1; then verilator --getenv VERILATOR_ROOT; else echo /tmp/proofcore-tools/root/usr/share/verilator; fi)
export VERILATOR_ROOT

.PHONY: all lint lint-package test-units test-core test synth-check verify clean
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

verify: lint test synth-check

clean:
	rm -rf build
