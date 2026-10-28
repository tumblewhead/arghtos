HOST_CC ?= gcc
CLANG ?= clang
PYTHON ?= python3
BUILD ?= build
M23_ARGHTOS_MSP ?= 104
M23_ARGHTOS_MSP_LTO ?= 88
M23_ARGHTOS_MSP_OMIT_FP ?= 80

HOST_CFLAGS := -std=c11 -Wall -Wextra -Werror -O2 -DARGHTOS_TEST \
	-Iinclude -Isrc -Iport/host
HOST_SRCS := src/arghtos_kernel.c port/host/arghtos_port.c tests/test_kernel.c

M23_COMMON := --target=arm-none-eabi -mcpu=cortex-m23 -mthumb -std=c11 \
	-ffreestanding -fno-builtin -fdata-sections -ffunction-sections \
	-fno-unwind-tables -fno-asynchronous-unwind-tables -fstack-usage -Oz \
	-Wall -Wextra -Werror -Iinclude -Isrc -Iport/cortex_m23



.PHONY: all portability-check host-test host-test-size-selector host-test-r3c host-test-r4u host-test-r5c host-test-r5d host-test-r5e host-sanitize m23-compile m23-footprint m23-footprint-r3c m23-footprint-r4y m23-footprint-r5a m23-footprint-r5b m23-footprint-r5c m23-footprint-r5d m23-footprint-r5e footprint-report clean



all: portability-check host-test m23-compile

$(BUILD):
	mkdir -p $(BUILD)

portability-check:
	$(PYTHON) tools/check_portable_source.py

host-test: | $(BUILD)
	$(HOST_CC) $(HOST_CFLAGS) $(HOST_SRCS) -o $(BUILD)/host-tests
	$(BUILD)/host-tests

host-test-size-selector: | $(BUILD)
	$(HOST_CC) $(HOST_CFLAGS) -DARGHTOS_SIZE_FIRST_SELECTOR=1 $(HOST_SRCS) \
		-o $(BUILD)/host-tests-size-selector
	$(BUILD)/host-tests-size-selector

host-test-r3c: | $(BUILD)
	$(HOST_CC) $(HOST_CFLAGS) -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=2 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		-DARGHTOS_SPECIALIZED_SERVICES=1 -DARGHTOS_MINIMAL_VALIDATION=1 \
		src/arghtos_kernel.c port/host/arghtos_port.c tests/test_static_profile.c \
		-o $(BUILD)/host-tests-r3c
	$(BUILD)/host-tests-r3c

host-test-r4u: | $(BUILD)
	$(HOST_CC) $(HOST_CFLAGS) -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=2 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		-DARGHTOS_SPECIALIZED_SERVICES=1 -DARGHTOS_MINIMAL_VALIDATION=1 \
		-DARGHTOS_RUNTIME_PROFILING=0 -DARGHTOS_SIZE_FIRST_SELECTOR=1 \
		-DARGHTOS_SIZE_FIRST_NO_DISPATCH_TIME=1 -DARGHTOS_SIZE_FIRST_NO_START_TIME=1 \
		-DARGHTOS_SPLIT_META=1 -DARGHTOS_ENTRY_NEXT_OVERLAY=1 \
		src/arghtos_kernel.c port/host/arghtos_port.c tests/test_static_profile.c \
		-o $(BUILD)/host-tests-r4u
	$(BUILD)/host-tests-r4u

host-test-r5c: | $(BUILD)
	$(HOST_CC) $(HOST_CFLAGS) -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=2 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		-DARGHTOS_SPECIALIZED_SERVICES=1 -DARGHTOS_MINIMAL_VALIDATION=1 \
		-DARGHTOS_RUNTIME_PROFILING=0 -DARGHTOS_SIZE_FIRST_SELECTOR=1 \
		-DARGHTOS_SIZE_FIRST_NO_DISPATCH_TIME=1 -DARGHTOS_SIZE_FIRST_NO_START_TIME=1 \
		-DARGHTOS_SPLIT_META=1 -DARGHTOS_ENTRY_NEXT_OVERLAY=1 \
		-DARGHTOS_R5C_COMPACT_WAKE_RESULT=1 \
		src/arghtos_kernel.c port/host/arghtos_port.c tests/test_static_profile.c \
		-o $(BUILD)/host-tests-r5c
	$(BUILD)/host-tests-r5c

host-test-r5d: | $(BUILD)
	$(HOST_CC) $(HOST_CFLAGS) -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=2 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		-DARGHTOS_SPECIALIZED_SERVICES=1 -DARGHTOS_MINIMAL_VALIDATION=1 \
		-DARGHTOS_RUNTIME_PROFILING=0 -DARGHTOS_SIZE_FIRST_SELECTOR=1 \
		-DARGHTOS_SIZE_FIRST_NO_DISPATCH_TIME=1 -DARGHTOS_SIZE_FIRST_NO_START_TIME=1 \
		-DARGHTOS_SPLIT_META=1 -DARGHTOS_ENTRY_NEXT_OVERLAY=1 \
		-DARGHTOS_R5D_COMPACT_SERVICE_OUTCOME=1 \
		src/arghtos_kernel.c port/host/arghtos_port.c tests/test_static_profile.c \
		-o $(BUILD)/host-tests-r5d
	$(BUILD)/host-tests-r5d

host-test-r5e: | $(BUILD)
	$(HOST_CC) $(HOST_CFLAGS) -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=2 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		-DARGHTOS_SPECIALIZED_SERVICES=1 -DARGHTOS_MINIMAL_VALIDATION=1 \
		-DARGHTOS_RUNTIME_PROFILING=0 -DARGHTOS_SIZE_FIRST_SELECTOR=1 \
		-DARGHTOS_SIZE_FIRST_NO_DISPATCH_TIME=1 -DARGHTOS_SIZE_FIRST_NO_START_TIME=1 \
		-DARGHTOS_SPLIT_META=1 -DARGHTOS_ENTRY_NEXT_OVERLAY=1 \
		-DARGHTOS_R5D_COMPACT_SERVICE_OUTCOME=1 -DARGHTOS_R5E_AGE_NEXT_OVERLAY=1 \
		src/arghtos_kernel.c port/host/arghtos_port.c tests/test_static_profile.c \
		-o $(BUILD)/host-tests-r5e
	$(BUILD)/host-tests-r5e

host-sanitize: | $(BUILD)
	$(HOST_CC) -std=c11 -Wall -Wextra -Werror -O1 -g \
		-fsanitize=address,undefined -fno-omit-frame-pointer \
		-DARGHTOS_TEST -Iinclude -Isrc -Iport/host $(HOST_SRCS) \
		-o $(BUILD)/host-tests-sanitize
	$(BUILD)/host-tests-sanitize

m23-compile: | $(BUILD)
	$(CLANG) $(M23_COMMON) -c src/arghtos_kernel.c -o $(BUILD)/arghtos_kernel_m23.o
	$(CLANG) $(M23_COMMON) -c port/cortex_m23/arghtos_port.c -o $(BUILD)/arghtos_port_m23.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb \
		-ffunction-sections -c port/cortex_m23/arghtos_context.S \
		-o $(BUILD)/arghtos_context_m23.o

m23-footprint: m23-compile
	$(CLANG) $(M23_COMMON) -c footprint/m23_board_stub.c -o $(BUILD)/m23_board_stub.o
	$(CLANG) $(M23_COMMON) -c footprint/m23_footprint.c -o $(BUILD)/m23_footprint.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb -nostdlib \
		-fuse-ld=lld -Wl,--gc-sections -Wl,-Map=$(BUILD)/m23.map \
		-Wl,-T,footprint/m23.ld -o $(BUILD)/m23.elf \
		$(BUILD)/arghtos_kernel_m23.o $(BUILD)/arghtos_port_m23.o \
		$(BUILD)/arghtos_context_m23.o $(BUILD)/m23_board_stub.o \
		$(BUILD)/m23_footprint.o
	size -A $(BUILD)/m23.elf
	$(PYTHON) tools/footprint_report.py --map $(BUILD)/m23.map \
		--msp $(M23_ARGHTOS_MSP) --json \
		> $(BUILD)/footprint.json
	cat $(BUILD)/footprint.json

m23-footprint-r3c: | $(BUILD)
	$(CLANG) $(M23_COMMON) -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=24 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		-DARGHTOS_SPECIALIZED_SERVICES=1 -DARGHTOS_MINIMAL_VALIDATION=1 \
		-c src/arghtos_kernel.c -o $(BUILD)/arghtos_kernel_m23_r3c.o
	$(CLANG) $(M23_COMMON) -c port/cortex_m23/arghtos_port.c \
		-o $(BUILD)/arghtos_port_m23_r3c.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb \
		-ffunction-sections -c port/cortex_m23/arghtos_context.S \
		-o $(BUILD)/arghtos_context_m23_r3c.o
	$(CLANG) $(M23_COMMON) -c footprint/m23_board_stub.c \
		-o $(BUILD)/m23_board_stub_r3c.o
	$(CLANG) $(M23_COMMON) -DARGHTOS_STATIC_CONFIG=1 \
		-DARGHTOS_STATIC_TASK_COUNT=24 -DARGHTOS_COMPACT_RUNTIME=1 \
		-DARGHTOS_COMPACT_DESCRIPTOR=1 -DARGHTOS_UNIFIED_SLOT=1 \
		-DARGHTOS_PRODUCTION_PROFILE=1 -DARGHTOS_SPECIALIZED_SERVICES=1 \
		-DARGHTOS_MINIMAL_VALIDATION=1 \
		-c footprint/m23_footprint.c -o $(BUILD)/m23_footprint_r3c.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb -nostdlib \
		-fuse-ld=lld -Wl,--gc-sections -Wl,-Map=$(BUILD)/m23-r3c.map \
		-Wl,-T,footprint/m23.ld -o $(BUILD)/m23-r3c.elf \
		$(BUILD)/arghtos_kernel_m23_r3c.o \
		$(BUILD)/arghtos_port_m23_r3c.o \
		$(BUILD)/arghtos_context_m23_r3c.o \
		$(BUILD)/m23_board_stub_r3c.o \
		$(BUILD)/m23_footprint_r3c.o
	size -A $(BUILD)/m23-r3c.elf
	$(PYTHON) tools/footprint_report.py --map $(BUILD)/m23-r3c.map \
		--msp $(M23_ARGHTOS_MSP) --json \
		> $(BUILD)/footprint-r3c.json
	cat $(BUILD)/footprint-r3c.json

m23-footprint-r4y: | $(BUILD)
	$(CLANG) $(M23_COMMON) -flto -fomit-frame-pointer -fno-jump-tables -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=24 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		-DARGHTOS_SPECIALIZED_SERVICES=1 -DARGHTOS_MINIMAL_VALIDATION=1 \
		-DARGHTOS_RUNTIME_PROFILING=0 -DARGHTOS_SIZE_FIRST_SELECTOR=1 \
		-DARGHTOS_SIZE_FIRST_NO_DISPATCH_TIME=1 -DARGHTOS_SIZE_FIRST_NO_START_TIME=1 \
		-DARGHTOS_SPLIT_META=1 -DARGHTOS_ENTRY_NEXT_OVERLAY=1 \
		-c src/arghtos_kernel.c -o $(BUILD)/arghtos_kernel_m23_r4y.o
	$(CLANG) $(M23_COMMON) -flto -fomit-frame-pointer -fno-jump-tables \
		-DARGHTOS_ENTRY_NEXT_OVERLAY=1 -c port/cortex_m23/arghtos_port.c \
		-o $(BUILD)/arghtos_port_m23_r4y.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb \
		-DARGHTOS_COMPACT_PENDSV=1 -DARGHTOS_COMPACT_SVC_RETURN=1 -ffunction-sections -c port/cortex_m23/arghtos_context.S \
		-o $(BUILD)/arghtos_context_m23_r4y.o
	$(CLANG) $(M23_COMMON) -flto -fomit-frame-pointer -fno-jump-tables -c footprint/m23_board_stub.c \
		-o $(BUILD)/m23_board_stub_r4y.o
	$(CLANG) $(M23_COMMON) -flto -fomit-frame-pointer -fno-jump-tables \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=24 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		-DARGHTOS_SPECIALIZED_SERVICES=1 -DARGHTOS_MINIMAL_VALIDATION=1 \
		-DARGHTOS_RUNTIME_PROFILING=0 -DARGHTOS_SIZE_FIRST_SELECTOR=1 \
		-DARGHTOS_SIZE_FIRST_NO_DISPATCH_TIME=1 -DARGHTOS_SIZE_FIRST_NO_START_TIME=1 \
		-DARGHTOS_SPLIT_META=1 -DARGHTOS_ENTRY_NEXT_OVERLAY=1 \
		-c footprint/m23_footprint.c -o $(BUILD)/m23_footprint_r4y.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb -nostdlib -flto -fomit-frame-pointer -fno-jump-tables \
		-fuse-ld=lld -Wl,--gc-sections -Wl,-Map=$(BUILD)/m23-r4y.map \
		-Wl,-T,footprint/m23_min_no_exidx.ld -o $(BUILD)/m23-r4y.elf \
		$(BUILD)/arghtos_kernel_m23_r4y.o \
		$(BUILD)/arghtos_port_m23_r4y.o \
		$(BUILD)/arghtos_context_m23_r4y.o \
		$(BUILD)/m23_board_stub_r4y.o \
		$(BUILD)/m23_footprint_r4y.o
	size -A $(BUILD)/m23-r4y.elf
	$(PYTHON) tools/footprint_report.py --map $(BUILD)/m23-r4y.map \
		--msp $(M23_ARGHTOS_MSP_OMIT_FP) --json \
		> $(BUILD)/footprint-r4y.json
	cat $(BUILD)/footprint-r4y.json

m23-footprint-r5a: | $(BUILD)
	$(CLANG) $(M23_COMMON) -flto -fomit-frame-pointer -fno-jump-tables -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=24 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		-DARGHTOS_SPECIALIZED_SERVICES=1 -DARGHTOS_MINIMAL_VALIDATION=1 \
		-DARGHTOS_RUNTIME_PROFILING=0 -DARGHTOS_SIZE_FIRST_SELECTOR=1 \
		-DARGHTOS_SIZE_FIRST_NO_DISPATCH_TIME=1 -DARGHTOS_SIZE_FIRST_NO_START_TIME=1 \
		-DARGHTOS_SPLIT_META=1 -DARGHTOS_ENTRY_NEXT_OVERLAY=1 \
		-c src/arghtos_kernel.c -o $(BUILD)/arghtos_kernel_m23_r5a.o
	$(CLANG) $(M23_COMMON) -flto -fomit-frame-pointer -fno-jump-tables \
		-DARGHTOS_ENTRY_NEXT_OVERLAY=1 -c port/cortex_m23/arghtos_port.c \
		-o $(BUILD)/arghtos_port_m23_r5a.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb \
		-DARGHTOS_COMPACT_PENDSV=1 -DARGHTOS_COMPACT_SVC_RETURN=1 -DARGHTOS_R5A_COMPACT_PENDSV_TAIL=1 -ffunction-sections -c port/cortex_m23/arghtos_context.S \
		-o $(BUILD)/arghtos_context_m23_r5a.o
	$(CLANG) $(M23_COMMON) -flto -fomit-frame-pointer -fno-jump-tables -c footprint/m23_board_stub.c \
		-o $(BUILD)/m23_board_stub_r5a.o
	$(CLANG) $(M23_COMMON) -flto -fomit-frame-pointer -fno-jump-tables \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=24 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		-DARGHTOS_SPECIALIZED_SERVICES=1 -DARGHTOS_MINIMAL_VALIDATION=1 \
		-DARGHTOS_RUNTIME_PROFILING=0 -DARGHTOS_SIZE_FIRST_SELECTOR=1 \
		-DARGHTOS_SIZE_FIRST_NO_DISPATCH_TIME=1 -DARGHTOS_SIZE_FIRST_NO_START_TIME=1 \
		-DARGHTOS_SPLIT_META=1 -DARGHTOS_ENTRY_NEXT_OVERLAY=1 \
		-c footprint/m23_footprint.c -o $(BUILD)/m23_footprint_r5a.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb -nostdlib -flto -fomit-frame-pointer -fno-jump-tables \
		-fuse-ld=lld -Wl,--gc-sections -Wl,-Map=$(BUILD)/m23-r5a.map \
		-Wl,-T,footprint/m23_min_no_exidx.ld -o $(BUILD)/m23-r5a.elf \
		$(BUILD)/arghtos_kernel_m23_r5a.o \
		$(BUILD)/arghtos_port_m23_r5a.o \
		$(BUILD)/arghtos_context_m23_r5a.o \
		$(BUILD)/m23_board_stub_r5a.o \
		$(BUILD)/m23_footprint_r5a.o
	size -A $(BUILD)/m23-r5a.elf
	$(PYTHON) tools/footprint_report.py --map $(BUILD)/m23-r5a.map \
		--msp $(M23_ARGHTOS_MSP_OMIT_FP) --json \
		> $(BUILD)/footprint-r5a.json
	cat $(BUILD)/footprint-r5a.json

m23-footprint-r5b: | $(BUILD)
	$(CLANG) $(M23_COMMON) -flto -fomit-frame-pointer -fno-jump-tables -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=24 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		-DARGHTOS_SPECIALIZED_SERVICES=1 -DARGHTOS_MINIMAL_VALIDATION=1 \
		-DARGHTOS_RUNTIME_PROFILING=0 -DARGHTOS_SIZE_FIRST_SELECTOR=1 \
		-DARGHTOS_SIZE_FIRST_NO_DISPATCH_TIME=1 -DARGHTOS_SIZE_FIRST_NO_START_TIME=1 \
		-DARGHTOS_SPLIT_META=1 -DARGHTOS_ENTRY_NEXT_OVERLAY=1 \
		-c src/arghtos_kernel.c -o $(BUILD)/arghtos_kernel_m23_r5b.o
	$(CLANG) $(M23_COMMON) -flto -fomit-frame-pointer -fno-jump-tables \
		-DARGHTOS_ENTRY_NEXT_OVERLAY=1 -c port/cortex_m23/arghtos_port.c \
		-o $(BUILD)/arghtos_port_m23_r5b.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb \
		-DARGHTOS_COMPACT_PENDSV=1 -DARGHTOS_COMPACT_SVC_RETURN=1 -DARGHTOS_R5A_COMPACT_PENDSV_TAIL=1 -DARGHTOS_R5B_COMPACT_EXC_CHECKS=1 -ffunction-sections -c port/cortex_m23/arghtos_context.S \
		-o $(BUILD)/arghtos_context_m23_r5b.o
	$(CLANG) $(M23_COMMON) -flto -fomit-frame-pointer -fno-jump-tables -c footprint/m23_board_stub.c \
		-o $(BUILD)/m23_board_stub_r5b.o
	$(CLANG) $(M23_COMMON) -flto -fomit-frame-pointer -fno-jump-tables \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=24 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		-DARGHTOS_SPECIALIZED_SERVICES=1 -DARGHTOS_MINIMAL_VALIDATION=1 \
		-DARGHTOS_RUNTIME_PROFILING=0 -DARGHTOS_SIZE_FIRST_SELECTOR=1 \
		-DARGHTOS_SIZE_FIRST_NO_DISPATCH_TIME=1 -DARGHTOS_SIZE_FIRST_NO_START_TIME=1 \
		-DARGHTOS_SPLIT_META=1 -DARGHTOS_ENTRY_NEXT_OVERLAY=1 \
		-c footprint/m23_footprint.c -o $(BUILD)/m23_footprint_r5b.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb -nostdlib -flto -fomit-frame-pointer -fno-jump-tables \
		-fuse-ld=lld -Wl,--gc-sections -Wl,-Map=$(BUILD)/m23-r5b.map \
		-Wl,-T,footprint/m23_min_no_exidx.ld -o $(BUILD)/m23-r5b.elf \
		$(BUILD)/arghtos_kernel_m23_r5b.o \
		$(BUILD)/arghtos_port_m23_r5b.o \
		$(BUILD)/arghtos_context_m23_r5b.o \
		$(BUILD)/m23_board_stub_r5b.o \
		$(BUILD)/m23_footprint_r5b.o
	size -A $(BUILD)/m23-r5b.elf
	$(PYTHON) tools/footprint_report.py --map $(BUILD)/m23-r5b.map \
		--msp $(M23_ARGHTOS_MSP_OMIT_FP) --json \
		> $(BUILD)/footprint-r5b.json
	cat $(BUILD)/footprint-r5b.json

m23-footprint-r5d: | $(BUILD)
	$(CLANG) $(M23_COMMON) -flto -fomit-frame-pointer -fno-jump-tables -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=24 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		-DARGHTOS_SPECIALIZED_SERVICES=1 -DARGHTOS_MINIMAL_VALIDATION=1 \
		-DARGHTOS_RUNTIME_PROFILING=0 -DARGHTOS_SIZE_FIRST_SELECTOR=1 \
		-DARGHTOS_SIZE_FIRST_NO_DISPATCH_TIME=1 -DARGHTOS_SIZE_FIRST_NO_START_TIME=1 \
		-DARGHTOS_SPLIT_META=1 -DARGHTOS_ENTRY_NEXT_OVERLAY=1 \
		-DARGHTOS_R5D_COMPACT_SERVICE_OUTCOME=1 \
		-c src/arghtos_kernel.c -o $(BUILD)/arghtos_kernel_m23_r5d.o
	$(CLANG) $(M23_COMMON) -flto -fomit-frame-pointer -fno-jump-tables \
		-DARGHTOS_ENTRY_NEXT_OVERLAY=1 -c port/cortex_m23/arghtos_port.c \
		-o $(BUILD)/arghtos_port_m23_r5d.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb \
		-DARGHTOS_COMPACT_PENDSV=1 -DARGHTOS_COMPACT_SVC_RETURN=1 -DARGHTOS_R5A_COMPACT_PENDSV_TAIL=1 -DARGHTOS_R5B_COMPACT_EXC_CHECKS=1 -ffunction-sections -c port/cortex_m23/arghtos_context.S \
		-o $(BUILD)/arghtos_context_m23_r5d.o
	$(CLANG) $(M23_COMMON) -flto -fomit-frame-pointer -fno-jump-tables -c footprint/m23_board_stub.c \
		-o $(BUILD)/m23_board_stub_r5d.o
	$(CLANG) $(M23_COMMON) -flto -fomit-frame-pointer -fno-jump-tables \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=24 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		-DARGHTOS_SPECIALIZED_SERVICES=1 -DARGHTOS_MINIMAL_VALIDATION=1 \
		-DARGHTOS_RUNTIME_PROFILING=0 -DARGHTOS_SIZE_FIRST_SELECTOR=1 \
		-DARGHTOS_SIZE_FIRST_NO_DISPATCH_TIME=1 -DARGHTOS_SIZE_FIRST_NO_START_TIME=1 \
		-DARGHTOS_SPLIT_META=1 -DARGHTOS_ENTRY_NEXT_OVERLAY=1 \
		-c footprint/m23_footprint.c -o $(BUILD)/m23_footprint_r5d.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb -nostdlib -flto -fomit-frame-pointer -fno-jump-tables \
		-fuse-ld=lld -Wl,--gc-sections -Wl,-Map=$(BUILD)/m23-r5d.map \
		-Wl,-T,footprint/m23_min_no_exidx.ld -o $(BUILD)/m23-r5d.elf \
		$(BUILD)/arghtos_kernel_m23_r5d.o \
		$(BUILD)/arghtos_port_m23_r5d.o \
		$(BUILD)/arghtos_context_m23_r5d.o \
		$(BUILD)/m23_board_stub_r5d.o \
		$(BUILD)/m23_footprint_r5d.o
	size -A $(BUILD)/m23-r5d.elf
	$(PYTHON) tools/footprint_report.py --map $(BUILD)/m23-r5d.map \
		--msp $(M23_ARGHTOS_MSP_OMIT_FP) --json \
		> $(BUILD)/footprint-r5d.json
	cat $(BUILD)/footprint-r5d.json

m23-footprint-r5e: | $(BUILD)
	$(CLANG) $(M23_COMMON) -flto -fomit-frame-pointer -fno-jump-tables -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=24 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		-DARGHTOS_SPECIALIZED_SERVICES=1 -DARGHTOS_MINIMAL_VALIDATION=1 \
		-DARGHTOS_RUNTIME_PROFILING=0 -DARGHTOS_SIZE_FIRST_SELECTOR=1 \
		-DARGHTOS_SIZE_FIRST_NO_DISPATCH_TIME=1 -DARGHTOS_SIZE_FIRST_NO_START_TIME=1 \
		-DARGHTOS_SPLIT_META=1 -DARGHTOS_ENTRY_NEXT_OVERLAY=1 \
		-DARGHTOS_R5D_COMPACT_SERVICE_OUTCOME=1 -DARGHTOS_R5E_AGE_NEXT_OVERLAY=1 \
		-c src/arghtos_kernel.c -o $(BUILD)/arghtos_kernel_m23_r5e.o
	$(CLANG) $(M23_COMMON) -flto -fomit-frame-pointer -fno-jump-tables \
		-DARGHTOS_ENTRY_NEXT_OVERLAY=1 -c port/cortex_m23/arghtos_port.c \
		-o $(BUILD)/arghtos_port_m23_r5e.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb \
		-DARGHTOS_COMPACT_PENDSV=1 -DARGHTOS_COMPACT_SVC_RETURN=1 -DARGHTOS_R5A_COMPACT_PENDSV_TAIL=1 -DARGHTOS_R5B_COMPACT_EXC_CHECKS=1 -ffunction-sections -c port/cortex_m23/arghtos_context.S \
		-o $(BUILD)/arghtos_context_m23_r5e.o
	$(CLANG) $(M23_COMMON) -flto -fomit-frame-pointer -fno-jump-tables -c footprint/m23_board_stub.c \
		-o $(BUILD)/m23_board_stub_r5e.o
	$(CLANG) $(M23_COMMON) -flto -fomit-frame-pointer -fno-jump-tables \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=24 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		-DARGHTOS_SPECIALIZED_SERVICES=1 -DARGHTOS_MINIMAL_VALIDATION=1 \
		-DARGHTOS_RUNTIME_PROFILING=0 -DARGHTOS_SIZE_FIRST_SELECTOR=1 \
		-DARGHTOS_SIZE_FIRST_NO_DISPATCH_TIME=1 -DARGHTOS_SIZE_FIRST_NO_START_TIME=1 \
		-DARGHTOS_SPLIT_META=1 -DARGHTOS_ENTRY_NEXT_OVERLAY=1 \
		-DARGHTOS_R5E_AGE_NEXT_OVERLAY=1 \
		-c footprint/m23_footprint.c -o $(BUILD)/m23_footprint_r5e.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb -nostdlib -flto -fomit-frame-pointer -fno-jump-tables \
		-fuse-ld=lld -Wl,--gc-sections -Wl,-Map=$(BUILD)/m23-r5e.map \
		-Wl,-T,footprint/m23_min_no_exidx.ld -o $(BUILD)/m23-r5e.elf \
		$(BUILD)/arghtos_kernel_m23_r5e.o \
		$(BUILD)/arghtos_port_m23_r5e.o \
		$(BUILD)/arghtos_context_m23_r5e.o \
		$(BUILD)/m23_board_stub_r5e.o \
		$(BUILD)/m23_footprint_r5e.o
	size -A $(BUILD)/m23-r5e.elf
	$(PYTHON) tools/footprint_report.py --map $(BUILD)/m23-r5e.map \
		--msp $(M23_ARGHTOS_MSP_OMIT_FP) --json \
		> $(BUILD)/footprint-r5e.json
	cat $(BUILD)/footprint-r5e.json

m23-footprint-r5c: | $(BUILD)
	$(CLANG) $(M23_COMMON) -flto -fomit-frame-pointer -fno-jump-tables -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=24 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		-DARGHTOS_SPECIALIZED_SERVICES=1 -DARGHTOS_MINIMAL_VALIDATION=1 \
		-DARGHTOS_RUNTIME_PROFILING=0 -DARGHTOS_SIZE_FIRST_SELECTOR=1 \
		-DARGHTOS_SIZE_FIRST_NO_DISPATCH_TIME=1 -DARGHTOS_SIZE_FIRST_NO_START_TIME=1 \
		-DARGHTOS_SPLIT_META=1 -DARGHTOS_ENTRY_NEXT_OVERLAY=1 \
		-DARGHTOS_R5C_COMPACT_WAKE_RESULT=1 \
		-c src/arghtos_kernel.c -o $(BUILD)/arghtos_kernel_m23_r5c.o
	$(CLANG) $(M23_COMMON) -flto -fomit-frame-pointer -fno-jump-tables \
		-DARGHTOS_ENTRY_NEXT_OVERLAY=1 -c port/cortex_m23/arghtos_port.c \
		-o $(BUILD)/arghtos_port_m23_r5c.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb \
		-DARGHTOS_COMPACT_PENDSV=1 -DARGHTOS_COMPACT_SVC_RETURN=1 -DARGHTOS_R5A_COMPACT_PENDSV_TAIL=1 -DARGHTOS_R5B_COMPACT_EXC_CHECKS=1 -ffunction-sections -c port/cortex_m23/arghtos_context.S \
		-o $(BUILD)/arghtos_context_m23_r5c.o
	$(CLANG) $(M23_COMMON) -flto -fomit-frame-pointer -fno-jump-tables -c footprint/m23_board_stub.c \
		-o $(BUILD)/m23_board_stub_r5c.o
	$(CLANG) $(M23_COMMON) -flto -fomit-frame-pointer -fno-jump-tables \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=24 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		-DARGHTOS_SPECIALIZED_SERVICES=1 -DARGHTOS_MINIMAL_VALIDATION=1 \
		-DARGHTOS_RUNTIME_PROFILING=0 -DARGHTOS_SIZE_FIRST_SELECTOR=1 \
		-DARGHTOS_SIZE_FIRST_NO_DISPATCH_TIME=1 -DARGHTOS_SIZE_FIRST_NO_START_TIME=1 \
		-DARGHTOS_SPLIT_META=1 -DARGHTOS_ENTRY_NEXT_OVERLAY=1 \
		-c footprint/m23_footprint.c -o $(BUILD)/m23_footprint_r5c.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb -nostdlib -flto -fomit-frame-pointer -fno-jump-tables \
		-fuse-ld=lld -Wl,--gc-sections -Wl,-Map=$(BUILD)/m23-r5c.map \
		-Wl,-T,footprint/m23_min_no_exidx.ld -o $(BUILD)/m23-r5c.elf \
		$(BUILD)/arghtos_kernel_m23_r5c.o \
		$(BUILD)/arghtos_port_m23_r5c.o \
		$(BUILD)/arghtos_context_m23_r5c.o \
		$(BUILD)/m23_board_stub_r5c.o \
		$(BUILD)/m23_footprint_r5c.o
	size -A $(BUILD)/m23-r5c.elf
	$(PYTHON) tools/footprint_report.py --map $(BUILD)/m23-r5c.map \
		--msp $(M23_ARGHTOS_MSP_OMIT_FP) --json \
		> $(BUILD)/footprint-r5c.json
	cat $(BUILD)/footprint-r5c.json

footprint-report: m23-footprint

clean:
	rm -rf $(BUILD)
