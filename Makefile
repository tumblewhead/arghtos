HOST_CC ?= gcc
CLANG ?= clang
PYTHON ?= python3
BUILD ?= build
M23_ARGHTOS_MSP ?= 104
M23_ARGHTOS_MSP_LTO ?= 88

HOST_CFLAGS := -std=c11 -Wall -Wextra -Werror -O2 -DARGHTOS_TEST \
	-Iinclude -Isrc -Iport/host
HOST_SRCS := src/arghtos_kernel.c port/host/arghtos_port.c tests/test_kernel.c

M23_COMMON := --target=arm-none-eabi -mcpu=cortex-m23 -mthumb -std=c11 \
	-ffreestanding -fno-builtin -fdata-sections -ffunction-sections \
	-fno-unwind-tables -fno-asynchronous-unwind-tables -fstack-usage -Oz \
	-Wall -Wextra -Werror -Iinclude -Isrc -Iport/cortex_m23

.PHONY: all portability-check host-test host-test-size-selector host-test-size-crash-clear host-test-size-timer-sentinel host-test-size-start-timer host-test-size-service-ifchain host-test-size-fault-range host-test-size-time-two-pass host-test-size-wake-result-hoist host-test-compact host-test-static host-test-r2a host-test-r2b host-test-r2c host-test-r3a host-test-r3b host-test-r3c host-test-r4a host-test-r4b host-test-r4g host-test-r4h host-sanitize m23-compile m23-footprint m23-footprint-compact m23-footprint-static m23-footprint-r2a m23-footprint-r2b m23-footprint-r2c m23-footprint-r3a m23-footprint-r3b m23-footprint-r3c m23-footprint-r4a m23-footprint-r4b m23-footprint-r4c m23-footprint-r4d m23-footprint-r4e m23-footprint-r4f m23-footprint-r4g m23-footprint-r4h m23-footprint-r4i m23-footprint-r4j m23-footprint-r4k m23-footprint-r4l m23-footprint-r4m m23-footprint-r4n footprint-report clean

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

host-test-size-crash-clear: | $(BUILD)
	$(HOST_CC) $(HOST_CFLAGS) -DARGHTOS_SIZE_FIRST_CRASH_CLEAR=1 $(HOST_SRCS) \
		-o $(BUILD)/host-tests-size-crash-clear
	$(BUILD)/host-tests-size-crash-clear

host-test-size-timer-sentinel: | $(BUILD)
	$(HOST_CC) $(HOST_CFLAGS) -DARGHTOS_SIZE_FIRST_TIMER_SENTINEL=1 $(HOST_SRCS) \
		-o $(BUILD)/host-tests-size-timer-sentinel
	$(BUILD)/host-tests-size-timer-sentinel

host-test-size-start-timer: | $(BUILD)
	$(HOST_CC) $(HOST_CFLAGS) -DARGHTOS_SIZE_FIRST_START_TIMER=1 $(HOST_SRCS) \
		-o $(BUILD)/host-tests-size-start-timer
	$(BUILD)/host-tests-size-start-timer

host-test-size-service-ifchain: | $(BUILD)
	$(HOST_CC) $(HOST_CFLAGS) -DARGHTOS_SIZE_FIRST_SERVICE_IF_CHAIN=1 $(HOST_SRCS) \
		-o $(BUILD)/host-tests-size-service-ifchain
	$(BUILD)/host-tests-size-service-ifchain

host-test-size-fault-range: | $(BUILD)
	$(HOST_CC) $(HOST_CFLAGS) -DARGHTOS_SIZE_FIRST_FAULT_RANGE=1 $(HOST_SRCS) \
		-o $(BUILD)/host-tests-size-fault-range
	$(BUILD)/host-tests-size-fault-range

host-test-size-time-two-pass: | $(BUILD)
	$(HOST_CC) $(HOST_CFLAGS) -DARGHTOS_SIZE_FIRST_TIME_TWO_PASS=1 $(HOST_SRCS) \
		-o $(BUILD)/host-tests-size-time-two-pass
	$(BUILD)/host-tests-size-time-two-pass

host-test-size-wake-result-hoist: | $(BUILD)
	$(HOST_CC) $(HOST_CFLAGS) -DARGHTOS_SIZE_FIRST_WAKE_RESULT_HOIST=1 $(HOST_SRCS) \
		-o $(BUILD)/host-tests-size-wake-result-hoist
	$(BUILD)/host-tests-size-wake-result-hoist

host-test-compact: | $(BUILD)
	$(HOST_CC) $(HOST_CFLAGS) -DARGHTOS_TRUSTED_CONFIG=1 $(HOST_SRCS) \
		-o $(BUILD)/host-tests-compact
	$(BUILD)/host-tests-compact

host-test-static: | $(BUILD)
	$(HOST_CC) $(HOST_CFLAGS) -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=2 \
		src/arghtos_kernel.c port/host/arghtos_port.c tests/test_static_profile.c \
		-o $(BUILD)/host-tests-static
	$(BUILD)/host-tests-static

host-test-r2a: | $(BUILD)
	$(HOST_CC) $(HOST_CFLAGS) -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=2 \
		-DARGHTOS_COMPACT_RUNTIME=1 \
		src/arghtos_kernel.c port/host/arghtos_port.c tests/test_static_profile.c \
		-o $(BUILD)/host-tests-r2a
	$(BUILD)/host-tests-r2a

host-test-r2b: | $(BUILD)
	$(HOST_CC) $(HOST_CFLAGS) -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=2 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		src/arghtos_kernel.c port/host/arghtos_port.c tests/test_static_profile.c \
		-o $(BUILD)/host-tests-r2b
	$(BUILD)/host-tests-r2b

host-test-r2c: | $(BUILD)
	$(HOST_CC) $(HOST_CFLAGS) -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=2 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 \
		src/arghtos_kernel.c port/host/arghtos_port.c tests/test_static_profile.c \
		-o $(BUILD)/host-tests-r2c
	$(BUILD)/host-tests-r2c

host-test-r3a: | $(BUILD)
	$(HOST_CC) $(HOST_CFLAGS) -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=2 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		src/arghtos_kernel.c port/host/arghtos_port.c tests/test_static_profile.c \
		-o $(BUILD)/host-tests-r3a
	$(BUILD)/host-tests-r3a

host-test-r3b: | $(BUILD)
	$(HOST_CC) $(HOST_CFLAGS) -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=2 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		-DARGHTOS_SPECIALIZED_SERVICES=1 \
		src/arghtos_kernel.c port/host/arghtos_port.c tests/test_static_profile.c \
		-o $(BUILD)/host-tests-r3b
	$(BUILD)/host-tests-r3b

host-test-r3c: | $(BUILD)
	$(HOST_CC) $(HOST_CFLAGS) -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=2 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		-DARGHTOS_SPECIALIZED_SERVICES=1 -DARGHTOS_MINIMAL_VALIDATION=1 \
		src/arghtos_kernel.c port/host/arghtos_port.c tests/test_static_profile.c \
		-o $(BUILD)/host-tests-r3c
	$(BUILD)/host-tests-r3c

host-test-r4a: | $(BUILD)
	$(HOST_CC) $(HOST_CFLAGS) -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=2 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		-DARGHTOS_SPECIALIZED_SERVICES=1 -DARGHTOS_MINIMAL_VALIDATION=1 \
		-DARGHTOS_RUNTIME_PROFILING=0 \
		src/arghtos_kernel.c port/host/arghtos_port.c tests/test_static_profile.c \
		-o $(BUILD)/host-tests-r4a
	$(BUILD)/host-tests-r4a

host-test-r4b: | $(BUILD)
	$(HOST_CC) $(HOST_CFLAGS) -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=2 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		-DARGHTOS_SPECIALIZED_SERVICES=1 -DARGHTOS_MINIMAL_VALIDATION=1 \
		-DARGHTOS_RUNTIME_PROFILING=0 -DARGHTOS_SIZE_FIRST_SELECTOR=1 \
		src/arghtos_kernel.c port/host/arghtos_port.c tests/test_static_profile.c \
		-o $(BUILD)/host-tests-r4b
	$(BUILD)/host-tests-r4b

host-test-r4g: | $(BUILD)
	$(HOST_CC) $(HOST_CFLAGS) -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=2 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		-DARGHTOS_SPECIALIZED_SERVICES=1 -DARGHTOS_MINIMAL_VALIDATION=1 \
		-DARGHTOS_RUNTIME_PROFILING=0 -DARGHTOS_SIZE_FIRST_SELECTOR=1 \
		-DARGHTOS_SIZE_FIRST_NO_DISPATCH_TIME=1 \
		src/arghtos_kernel.c port/host/arghtos_port.c tests/test_static_profile.c \
		-o $(BUILD)/host-tests-r4g
	$(BUILD)/host-tests-r4g

host-test-r4h: | $(BUILD)
	$(HOST_CC) $(HOST_CFLAGS) -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=2 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		-DARGHTOS_SPECIALIZED_SERVICES=1 -DARGHTOS_MINIMAL_VALIDATION=1 \
		-DARGHTOS_RUNTIME_PROFILING=0 -DARGHTOS_SIZE_FIRST_SELECTOR=1 \
		-DARGHTOS_SIZE_FIRST_NO_DISPATCH_TIME=1 -DARGHTOS_SIZE_FIRST_NO_START_TIME=1 \
		src/arghtos_kernel.c port/host/arghtos_port.c tests/test_static_profile.c \
		-o $(BUILD)/host-tests-r4h
	$(BUILD)/host-tests-r4h

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


m23-footprint-compact: | $(BUILD)
	$(CLANG) $(M23_COMMON) -DARGHTOS_TRUSTED_CONFIG=1 -c src/arghtos_kernel.c \
		-o $(BUILD)/arghtos_kernel_m23_compact.o
	$(CLANG) $(M23_COMMON) -c port/cortex_m23/arghtos_port.c \
		-o $(BUILD)/arghtos_port_m23_compact.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb \
		-ffunction-sections -c port/cortex_m23/arghtos_context.S \
		-o $(BUILD)/arghtos_context_m23_compact.o
	$(CLANG) $(M23_COMMON) -c footprint/m23_board_stub.c \
		-o $(BUILD)/m23_board_stub_compact.o
	$(CLANG) $(M23_COMMON) -c footprint/m23_footprint.c \
		-o $(BUILD)/m23_footprint_compact.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb -nostdlib \
		-fuse-ld=lld -Wl,--gc-sections -Wl,-Map=$(BUILD)/m23-compact.map \
		-Wl,-T,footprint/m23.ld -o $(BUILD)/m23-compact.elf \
		$(BUILD)/arghtos_kernel_m23_compact.o \
		$(BUILD)/arghtos_port_m23_compact.o \
		$(BUILD)/arghtos_context_m23_compact.o \
		$(BUILD)/m23_board_stub_compact.o \
		$(BUILD)/m23_footprint_compact.o
	size -A $(BUILD)/m23-compact.elf
	$(PYTHON) tools/footprint_report.py --map $(BUILD)/m23-compact.map \
		--msp $(M23_ARGHTOS_MSP) --json \
		> $(BUILD)/footprint-compact.json
	cat $(BUILD)/footprint-compact.json


m23-footprint-static: | $(BUILD)
	$(CLANG) $(M23_COMMON) -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=24 \
		-c src/arghtos_kernel.c -o $(BUILD)/arghtos_kernel_m23_static.o
	$(CLANG) $(M23_COMMON) -c port/cortex_m23/arghtos_port.c \
		-o $(BUILD)/arghtos_port_m23_static.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb \
		-ffunction-sections -c port/cortex_m23/arghtos_context.S \
		-o $(BUILD)/arghtos_context_m23_static.o
	$(CLANG) $(M23_COMMON) -c footprint/m23_board_stub.c \
		-o $(BUILD)/m23_board_stub_static.o
	$(CLANG) $(M23_COMMON) -DARGHTOS_STATIC_CONFIG=1 \
		-DARGHTOS_STATIC_TASK_COUNT=24 \
		-c footprint/m23_footprint.c -o $(BUILD)/m23_footprint_static.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb -nostdlib \
		-fuse-ld=lld -Wl,--gc-sections -Wl,-Map=$(BUILD)/m23-static.map \
		-Wl,-T,footprint/m23.ld -o $(BUILD)/m23-static.elf \
		$(BUILD)/arghtos_kernel_m23_static.o \
		$(BUILD)/arghtos_port_m23_static.o \
		$(BUILD)/arghtos_context_m23_static.o \
		$(BUILD)/m23_board_stub_static.o \
		$(BUILD)/m23_footprint_static.o
	size -A $(BUILD)/m23-static.elf
	$(PYTHON) tools/footprint_report.py --map $(BUILD)/m23-static.map \
		--msp $(M23_ARGHTOS_MSP) --json \
		> $(BUILD)/footprint-static.json
	cat $(BUILD)/footprint-static.json


m23-footprint-r2a: | $(BUILD)
	$(CLANG) $(M23_COMMON) -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=24 \
		-DARGHTOS_COMPACT_RUNTIME=1 \
		-c src/arghtos_kernel.c -o $(BUILD)/arghtos_kernel_m23_r2a.o
	$(CLANG) $(M23_COMMON) -c port/cortex_m23/arghtos_port.c \
		-o $(BUILD)/arghtos_port_m23_r2a.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb \
		-ffunction-sections -c port/cortex_m23/arghtos_context.S \
		-o $(BUILD)/arghtos_context_m23_r2a.o
	$(CLANG) $(M23_COMMON) -c footprint/m23_board_stub.c \
		-o $(BUILD)/m23_board_stub_r2a.o
	$(CLANG) $(M23_COMMON) -DARGHTOS_STATIC_CONFIG=1 \
		-DARGHTOS_STATIC_TASK_COUNT=24 -DARGHTOS_COMPACT_RUNTIME=1 \
		-c footprint/m23_footprint.c -o $(BUILD)/m23_footprint_r2a.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb -nostdlib \
		-fuse-ld=lld -Wl,--gc-sections -Wl,-Map=$(BUILD)/m23-r2a.map \
		-Wl,-T,footprint/m23.ld -o $(BUILD)/m23-r2a.elf \
		$(BUILD)/arghtos_kernel_m23_r2a.o \
		$(BUILD)/arghtos_port_m23_r2a.o \
		$(BUILD)/arghtos_context_m23_r2a.o \
		$(BUILD)/m23_board_stub_r2a.o \
		$(BUILD)/m23_footprint_r2a.o
	size -A $(BUILD)/m23-r2a.elf
	$(PYTHON) tools/footprint_report.py --map $(BUILD)/m23-r2a.map \
		--msp $(M23_ARGHTOS_MSP) --json \
		> $(BUILD)/footprint-r2a.json
	cat $(BUILD)/footprint-r2a.json


m23-footprint-r2b: | $(BUILD)
	$(CLANG) $(M23_COMMON) -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=24 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-c src/arghtos_kernel.c -o $(BUILD)/arghtos_kernel_m23_r2b.o
	$(CLANG) $(M23_COMMON) -c port/cortex_m23/arghtos_port.c \
		-o $(BUILD)/arghtos_port_m23_r2b.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb \
		-ffunction-sections -c port/cortex_m23/arghtos_context.S \
		-o $(BUILD)/arghtos_context_m23_r2b.o
	$(CLANG) $(M23_COMMON) -c footprint/m23_board_stub.c \
		-o $(BUILD)/m23_board_stub_r2b.o
	$(CLANG) $(M23_COMMON) -DARGHTOS_STATIC_CONFIG=1 \
		-DARGHTOS_STATIC_TASK_COUNT=24 -DARGHTOS_COMPACT_RUNTIME=1 \
		-DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-c footprint/m23_footprint.c -o $(BUILD)/m23_footprint_r2b.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb -nostdlib \
		-fuse-ld=lld -Wl,--gc-sections -Wl,-Map=$(BUILD)/m23-r2b.map \
		-Wl,-T,footprint/m23.ld -o $(BUILD)/m23-r2b.elf \
		$(BUILD)/arghtos_kernel_m23_r2b.o \
		$(BUILD)/arghtos_port_m23_r2b.o \
		$(BUILD)/arghtos_context_m23_r2b.o \
		$(BUILD)/m23_board_stub_r2b.o \
		$(BUILD)/m23_footprint_r2b.o
	size -A $(BUILD)/m23-r2b.elf
	$(PYTHON) tools/footprint_report.py --map $(BUILD)/m23-r2b.map \
		--msp $(M23_ARGHTOS_MSP) --json \
		> $(BUILD)/footprint-r2b.json
	cat $(BUILD)/footprint-r2b.json


m23-footprint-r2c: | $(BUILD)
	$(CLANG) $(M23_COMMON) -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=24 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 \
		-c src/arghtos_kernel.c -o $(BUILD)/arghtos_kernel_m23_r2c.o
	$(CLANG) $(M23_COMMON) -c port/cortex_m23/arghtos_port.c \
		-o $(BUILD)/arghtos_port_m23_r2c.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb \
		-ffunction-sections -c port/cortex_m23/arghtos_context.S \
		-o $(BUILD)/arghtos_context_m23_r2c.o
	$(CLANG) $(M23_COMMON) -c footprint/m23_board_stub.c \
		-o $(BUILD)/m23_board_stub_r2c.o
	$(CLANG) $(M23_COMMON) -DARGHTOS_STATIC_CONFIG=1 \
		-DARGHTOS_STATIC_TASK_COUNT=24 -DARGHTOS_COMPACT_RUNTIME=1 \
		-DARGHTOS_COMPACT_DESCRIPTOR=1 -DARGHTOS_UNIFIED_SLOT=1 \
		-c footprint/m23_footprint.c -o $(BUILD)/m23_footprint_r2c.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb -nostdlib \
		-fuse-ld=lld -Wl,--gc-sections -Wl,-Map=$(BUILD)/m23-r2c.map \
		-Wl,-T,footprint/m23.ld -o $(BUILD)/m23-r2c.elf \
		$(BUILD)/arghtos_kernel_m23_r2c.o \
		$(BUILD)/arghtos_port_m23_r2c.o \
		$(BUILD)/arghtos_context_m23_r2c.o \
		$(BUILD)/m23_board_stub_r2c.o \
		$(BUILD)/m23_footprint_r2c.o
	size -A $(BUILD)/m23-r2c.elf
	$(PYTHON) tools/footprint_report.py --map $(BUILD)/m23-r2c.map \
		--msp $(M23_ARGHTOS_MSP) --json \
		> $(BUILD)/footprint-r2c.json
	cat $(BUILD)/footprint-r2c.json


m23-footprint-r3a: | $(BUILD)
	$(CLANG) $(M23_COMMON) -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=24 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		-c src/arghtos_kernel.c -o $(BUILD)/arghtos_kernel_m23_r3a.o
	$(CLANG) $(M23_COMMON) -c port/cortex_m23/arghtos_port.c \
		-o $(BUILD)/arghtos_port_m23_r3a.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb \
		-ffunction-sections -c port/cortex_m23/arghtos_context.S \
		-o $(BUILD)/arghtos_context_m23_r3a.o
	$(CLANG) $(M23_COMMON) -c footprint/m23_board_stub.c \
		-o $(BUILD)/m23_board_stub_r3a.o
	$(CLANG) $(M23_COMMON) -DARGHTOS_STATIC_CONFIG=1 \
		-DARGHTOS_STATIC_TASK_COUNT=24 -DARGHTOS_COMPACT_RUNTIME=1 \
		-DARGHTOS_COMPACT_DESCRIPTOR=1 -DARGHTOS_UNIFIED_SLOT=1 \
		-DARGHTOS_PRODUCTION_PROFILE=1 \
		-c footprint/m23_footprint.c -o $(BUILD)/m23_footprint_r3a.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb -nostdlib \
		-fuse-ld=lld -Wl,--gc-sections -Wl,-Map=$(BUILD)/m23-r3a.map \
		-Wl,-T,footprint/m23.ld -o $(BUILD)/m23-r3a.elf \
		$(BUILD)/arghtos_kernel_m23_r3a.o \
		$(BUILD)/arghtos_port_m23_r3a.o \
		$(BUILD)/arghtos_context_m23_r3a.o \
		$(BUILD)/m23_board_stub_r3a.o \
		$(BUILD)/m23_footprint_r3a.o
	size -A $(BUILD)/m23-r3a.elf
	$(PYTHON) tools/footprint_report.py --map $(BUILD)/m23-r3a.map \
		--msp $(M23_ARGHTOS_MSP) --json \
		> $(BUILD)/footprint-r3a.json
	cat $(BUILD)/footprint-r3a.json


m23-footprint-r3b: | $(BUILD)
	$(CLANG) $(M23_COMMON) -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=24 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		-DARGHTOS_SPECIALIZED_SERVICES=1 \
		-c src/arghtos_kernel.c -o $(BUILD)/arghtos_kernel_m23_r3b.o
	$(CLANG) $(M23_COMMON) -c port/cortex_m23/arghtos_port.c \
		-o $(BUILD)/arghtos_port_m23_r3b.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb \
		-ffunction-sections -c port/cortex_m23/arghtos_context.S \
		-o $(BUILD)/arghtos_context_m23_r3b.o
	$(CLANG) $(M23_COMMON) -c footprint/m23_board_stub.c \
		-o $(BUILD)/m23_board_stub_r3b.o
	$(CLANG) $(M23_COMMON) -DARGHTOS_STATIC_CONFIG=1 \
		-DARGHTOS_STATIC_TASK_COUNT=24 -DARGHTOS_COMPACT_RUNTIME=1 \
		-DARGHTOS_COMPACT_DESCRIPTOR=1 -DARGHTOS_UNIFIED_SLOT=1 \
		-DARGHTOS_PRODUCTION_PROFILE=1 -DARGHTOS_SPECIALIZED_SERVICES=1 \
		-c footprint/m23_footprint.c -o $(BUILD)/m23_footprint_r3b.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb -nostdlib \
		-fuse-ld=lld -Wl,--gc-sections -Wl,-Map=$(BUILD)/m23-r3b.map \
		-Wl,-T,footprint/m23.ld -o $(BUILD)/m23-r3b.elf \
		$(BUILD)/arghtos_kernel_m23_r3b.o \
		$(BUILD)/arghtos_port_m23_r3b.o \
		$(BUILD)/arghtos_context_m23_r3b.o \
		$(BUILD)/m23_board_stub_r3b.o \
		$(BUILD)/m23_footprint_r3b.o
	size -A $(BUILD)/m23-r3b.elf
	$(PYTHON) tools/footprint_report.py --map $(BUILD)/m23-r3b.map \
		--msp $(M23_ARGHTOS_MSP) --json \
		> $(BUILD)/footprint-r3b.json
	cat $(BUILD)/footprint-r3b.json


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

m23-footprint-r4a: | $(BUILD)
	$(CLANG) $(M23_COMMON) -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=24 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		-DARGHTOS_SPECIALIZED_SERVICES=1 -DARGHTOS_MINIMAL_VALIDATION=1 \
		-DARGHTOS_RUNTIME_PROFILING=0 \
		-c src/arghtos_kernel.c -o $(BUILD)/arghtos_kernel_m23_r4a.o
	$(CLANG) $(M23_COMMON) -c port/cortex_m23/arghtos_port.c \
		-o $(BUILD)/arghtos_port_m23_r4a.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb \
		-ffunction-sections -c port/cortex_m23/arghtos_context.S \
		-o $(BUILD)/arghtos_context_m23_r4a.o
	$(CLANG) $(M23_COMMON) -c footprint/m23_board_stub.c \
		-o $(BUILD)/m23_board_stub_r4a.o
	$(CLANG) $(M23_COMMON) -DARGHTOS_STATIC_CONFIG=1 \
		-DARGHTOS_STATIC_TASK_COUNT=24 -DARGHTOS_COMPACT_RUNTIME=1 \
		-DARGHTOS_COMPACT_DESCRIPTOR=1 -DARGHTOS_UNIFIED_SLOT=1 \
		-DARGHTOS_PRODUCTION_PROFILE=1 -DARGHTOS_SPECIALIZED_SERVICES=1 \
		-DARGHTOS_MINIMAL_VALIDATION=1 -DARGHTOS_RUNTIME_PROFILING=0 \
		-c footprint/m23_footprint.c -o $(BUILD)/m23_footprint_r4a.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb -nostdlib \
		-fuse-ld=lld -Wl,--gc-sections -Wl,-Map=$(BUILD)/m23-r4a.map \
		-Wl,-T,footprint/m23.ld -o $(BUILD)/m23-r4a.elf \
		$(BUILD)/arghtos_kernel_m23_r4a.o \
		$(BUILD)/arghtos_port_m23_r4a.o \
		$(BUILD)/arghtos_context_m23_r4a.o \
		$(BUILD)/m23_board_stub_r4a.o \
		$(BUILD)/m23_footprint_r4a.o
	size -A $(BUILD)/m23-r4a.elf
	$(PYTHON) tools/footprint_report.py --map $(BUILD)/m23-r4a.map \
		--msp $(M23_ARGHTOS_MSP) --json \
		> $(BUILD)/footprint-r4a.json
	cat $(BUILD)/footprint-r4a.json

m23-footprint-r4b: | $(BUILD)
	$(CLANG) $(M23_COMMON) -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=24 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		-DARGHTOS_SPECIALIZED_SERVICES=1 -DARGHTOS_MINIMAL_VALIDATION=1 \
		-DARGHTOS_RUNTIME_PROFILING=0 -DARGHTOS_SIZE_FIRST_SELECTOR=1 \
		-c src/arghtos_kernel.c -o $(BUILD)/arghtos_kernel_m23_r4b.o
	$(CLANG) $(M23_COMMON) -c port/cortex_m23/arghtos_port.c \
		-o $(BUILD)/arghtos_port_m23_r4b.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb \
		-ffunction-sections -c port/cortex_m23/arghtos_context.S \
		-o $(BUILD)/arghtos_context_m23_r4b.o
	$(CLANG) $(M23_COMMON) -c footprint/m23_board_stub.c \
		-o $(BUILD)/m23_board_stub_r4b.o
	$(CLANG) $(M23_COMMON) -DARGHTOS_STATIC_CONFIG=1 \
		-DARGHTOS_STATIC_TASK_COUNT=24 -DARGHTOS_COMPACT_RUNTIME=1 \
		-DARGHTOS_COMPACT_DESCRIPTOR=1 -DARGHTOS_UNIFIED_SLOT=1 \
		-DARGHTOS_PRODUCTION_PROFILE=1 -DARGHTOS_SPECIALIZED_SERVICES=1 \
		-DARGHTOS_MINIMAL_VALIDATION=1 -DARGHTOS_RUNTIME_PROFILING=0 \
		-DARGHTOS_SIZE_FIRST_SELECTOR=1 \
		-c footprint/m23_footprint.c -o $(BUILD)/m23_footprint_r4b.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb -nostdlib \
		-fuse-ld=lld -Wl,--gc-sections -Wl,-Map=$(BUILD)/m23-r4b.map \
		-Wl,-T,footprint/m23.ld -o $(BUILD)/m23-r4b.elf \
		$(BUILD)/arghtos_kernel_m23_r4b.o \
		$(BUILD)/arghtos_port_m23_r4b.o \
		$(BUILD)/arghtos_context_m23_r4b.o \
		$(BUILD)/m23_board_stub_r4b.o \
		$(BUILD)/m23_footprint_r4b.o
	size -A $(BUILD)/m23-r4b.elf
	$(PYTHON) tools/footprint_report.py --map $(BUILD)/m23-r4b.map \
		--msp $(M23_ARGHTOS_MSP) --json \
		> $(BUILD)/footprint-r4b.json
	cat $(BUILD)/footprint-r4b.json

m23-footprint-r4c: | $(BUILD)
	$(CLANG) $(M23_COMMON) -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=24 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		-DARGHTOS_SPECIALIZED_SERVICES=1 -DARGHTOS_MINIMAL_VALIDATION=1 \
		-DARGHTOS_RUNTIME_PROFILING=0 -DARGHTOS_SIZE_FIRST_SELECTOR=1 \
		-c src/arghtos_kernel.c -o $(BUILD)/arghtos_kernel_m23_r4c.o
	$(CLANG) $(M23_COMMON) -DARGHTOS_SIZE_FIRST_CRASH_COPY=1 \
		-c port/cortex_m23/arghtos_port.c -o $(BUILD)/arghtos_port_m23_r4c.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb \
		-ffunction-sections -c port/cortex_m23/arghtos_context.S \
		-o $(BUILD)/arghtos_context_m23_r4c.o
	$(CLANG) $(M23_COMMON) -c footprint/m23_board_stub.c \
		-o $(BUILD)/m23_board_stub_r4c.o
	$(CLANG) $(M23_COMMON) -DARGHTOS_STATIC_CONFIG=1 \
		-DARGHTOS_STATIC_TASK_COUNT=24 -DARGHTOS_COMPACT_RUNTIME=1 \
		-DARGHTOS_COMPACT_DESCRIPTOR=1 -DARGHTOS_UNIFIED_SLOT=1 \
		-DARGHTOS_PRODUCTION_PROFILE=1 -DARGHTOS_SPECIALIZED_SERVICES=1 \
		-DARGHTOS_MINIMAL_VALIDATION=1 -DARGHTOS_RUNTIME_PROFILING=0 \
		-DARGHTOS_SIZE_FIRST_SELECTOR=1 \
		-c footprint/m23_footprint.c -o $(BUILD)/m23_footprint_r4c.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb -nostdlib \
		-fuse-ld=lld -Wl,--gc-sections -Wl,-Map=$(BUILD)/m23-r4c.map \
		-Wl,-T,footprint/m23.ld -o $(BUILD)/m23-r4c.elf \
		$(BUILD)/arghtos_kernel_m23_r4c.o \
		$(BUILD)/arghtos_port_m23_r4c.o \
		$(BUILD)/arghtos_context_m23_r4c.o \
		$(BUILD)/m23_board_stub_r4c.o \
		$(BUILD)/m23_footprint_r4c.o
	size -A $(BUILD)/m23-r4c.elf
	$(PYTHON) tools/footprint_report.py --map $(BUILD)/m23-r4c.map \
		--msp $(M23_ARGHTOS_MSP) --json \
		> $(BUILD)/footprint-r4c.json
	cat $(BUILD)/footprint-r4c.json

m23-footprint-r4d: | $(BUILD)
	$(CLANG) $(M23_COMMON) -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=24 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		-DARGHTOS_SPECIALIZED_SERVICES=1 -DARGHTOS_MINIMAL_VALIDATION=1 \
		-DARGHTOS_RUNTIME_PROFILING=0 -DARGHTOS_SIZE_FIRST_SELECTOR=1 \
		-DARGHTOS_SIZE_FIRST_CRASH_CLEAR=1 \
		-c src/arghtos_kernel.c -o $(BUILD)/arghtos_kernel_m23_r4d.o
	$(CLANG) $(M23_COMMON) -c port/cortex_m23/arghtos_port.c \
		-o $(BUILD)/arghtos_port_m23_r4d.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb \
		-ffunction-sections -c port/cortex_m23/arghtos_context.S \
		-o $(BUILD)/arghtos_context_m23_r4d.o
	$(CLANG) $(M23_COMMON) -c footprint/m23_board_stub.c \
		-o $(BUILD)/m23_board_stub_r4d.o
	$(CLANG) $(M23_COMMON) -DARGHTOS_STATIC_CONFIG=1 \
		-DARGHTOS_STATIC_TASK_COUNT=24 -DARGHTOS_COMPACT_RUNTIME=1 \
		-DARGHTOS_COMPACT_DESCRIPTOR=1 -DARGHTOS_UNIFIED_SLOT=1 \
		-DARGHTOS_PRODUCTION_PROFILE=1 -DARGHTOS_SPECIALIZED_SERVICES=1 \
		-DARGHTOS_MINIMAL_VALIDATION=1 -DARGHTOS_RUNTIME_PROFILING=0 \
		-DARGHTOS_SIZE_FIRST_SELECTOR=1 -DARGHTOS_SIZE_FIRST_CRASH_CLEAR=1 \
		-c footprint/m23_footprint.c -o $(BUILD)/m23_footprint_r4d.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb -nostdlib \
		-fuse-ld=lld -Wl,--gc-sections -Wl,-Map=$(BUILD)/m23-r4d.map \
		-Wl,-T,footprint/m23.ld -o $(BUILD)/m23-r4d.elf \
		$(BUILD)/arghtos_kernel_m23_r4d.o \
		$(BUILD)/arghtos_port_m23_r4d.o \
		$(BUILD)/arghtos_context_m23_r4d.o \
		$(BUILD)/m23_board_stub_r4d.o \
		$(BUILD)/m23_footprint_r4d.o
	size -A $(BUILD)/m23-r4d.elf
	$(PYTHON) tools/footprint_report.py --map $(BUILD)/m23-r4d.map \
		--msp $(M23_ARGHTOS_MSP) --json \
		> $(BUILD)/footprint-r4d.json
	cat $(BUILD)/footprint-r4d.json

m23-footprint-r4e: | $(BUILD)
	$(CLANG) $(M23_COMMON) -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=24 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		-DARGHTOS_SPECIALIZED_SERVICES=1 -DARGHTOS_MINIMAL_VALIDATION=1 \
		-DARGHTOS_RUNTIME_PROFILING=0 -DARGHTOS_SIZE_FIRST_SELECTOR=1 \
		-DARGHTOS_SIZE_FIRST_TIMER_SENTINEL=1 \
		-c src/arghtos_kernel.c -o $(BUILD)/arghtos_kernel_m23_r4e.o
	$(CLANG) $(M23_COMMON) -c port/cortex_m23/arghtos_port.c \
		-o $(BUILD)/arghtos_port_m23_r4e.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb \
		-ffunction-sections -c port/cortex_m23/arghtos_context.S \
		-o $(BUILD)/arghtos_context_m23_r4e.o
	$(CLANG) $(M23_COMMON) -c footprint/m23_board_stub.c \
		-o $(BUILD)/m23_board_stub_r4e.o
	$(CLANG) $(M23_COMMON) -DARGHTOS_STATIC_CONFIG=1 \
		-DARGHTOS_STATIC_TASK_COUNT=24 -DARGHTOS_COMPACT_RUNTIME=1 \
		-DARGHTOS_COMPACT_DESCRIPTOR=1 -DARGHTOS_UNIFIED_SLOT=1 \
		-DARGHTOS_PRODUCTION_PROFILE=1 -DARGHTOS_SPECIALIZED_SERVICES=1 \
		-DARGHTOS_MINIMAL_VALIDATION=1 -DARGHTOS_RUNTIME_PROFILING=0 \
		-DARGHTOS_SIZE_FIRST_SELECTOR=1 -DARGHTOS_SIZE_FIRST_TIMER_SENTINEL=1 \
		-c footprint/m23_footprint.c -o $(BUILD)/m23_footprint_r4e.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb -nostdlib \
		-fuse-ld=lld -Wl,--gc-sections -Wl,-Map=$(BUILD)/m23-r4e.map \
		-Wl,-T,footprint/m23.ld -o $(BUILD)/m23-r4e.elf \
		$(BUILD)/arghtos_kernel_m23_r4e.o \
		$(BUILD)/arghtos_port_m23_r4e.o \
		$(BUILD)/arghtos_context_m23_r4e.o \
		$(BUILD)/m23_board_stub_r4e.o \
		$(BUILD)/m23_footprint_r4e.o
	size -A $(BUILD)/m23-r4e.elf
	$(PYTHON) tools/footprint_report.py --map $(BUILD)/m23-r4e.map \
		--msp $(M23_ARGHTOS_MSP) --json \
		> $(BUILD)/footprint-r4e.json
	cat $(BUILD)/footprint-r4e.json

m23-footprint-r4f: | $(BUILD)
	$(CLANG) $(M23_COMMON) -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=24 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		-DARGHTOS_SPECIALIZED_SERVICES=1 -DARGHTOS_MINIMAL_VALIDATION=1 \
		-DARGHTOS_RUNTIME_PROFILING=0 -DARGHTOS_SIZE_FIRST_SELECTOR=1 \
		-DARGHTOS_SIZE_FIRST_START_TIMER=1 \
		-c src/arghtos_kernel.c -o $(BUILD)/arghtos_kernel_m23_r4f.o
	$(CLANG) $(M23_COMMON) -c port/cortex_m23/arghtos_port.c \
		-o $(BUILD)/arghtos_port_m23_r4f.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb \
		-ffunction-sections -c port/cortex_m23/arghtos_context.S \
		-o $(BUILD)/arghtos_context_m23_r4f.o
	$(CLANG) $(M23_COMMON) -c footprint/m23_board_stub.c \
		-o $(BUILD)/m23_board_stub_r4f.o
	$(CLANG) $(M23_COMMON) -DARGHTOS_STATIC_CONFIG=1 \
		-DARGHTOS_STATIC_TASK_COUNT=24 -DARGHTOS_COMPACT_RUNTIME=1 \
		-DARGHTOS_COMPACT_DESCRIPTOR=1 -DARGHTOS_UNIFIED_SLOT=1 \
		-DARGHTOS_PRODUCTION_PROFILE=1 -DARGHTOS_SPECIALIZED_SERVICES=1 \
		-DARGHTOS_MINIMAL_VALIDATION=1 -DARGHTOS_RUNTIME_PROFILING=0 \
		-DARGHTOS_SIZE_FIRST_SELECTOR=1 -DARGHTOS_SIZE_FIRST_START_TIMER=1 \
		-c footprint/m23_footprint.c -o $(BUILD)/m23_footprint_r4f.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb -nostdlib \
		-fuse-ld=lld -Wl,--gc-sections -Wl,-Map=$(BUILD)/m23-r4f.map \
		-Wl,-T,footprint/m23.ld -o $(BUILD)/m23-r4f.elf \
		$(BUILD)/arghtos_kernel_m23_r4f.o \
		$(BUILD)/arghtos_port_m23_r4f.o \
		$(BUILD)/arghtos_context_m23_r4f.o \
		$(BUILD)/m23_board_stub_r4f.o \
		$(BUILD)/m23_footprint_r4f.o
	size -A $(BUILD)/m23-r4f.elf
	$(PYTHON) tools/footprint_report.py --map $(BUILD)/m23-r4f.map \
		--msp $(M23_ARGHTOS_MSP) --json \
		> $(BUILD)/footprint-r4f.json
	cat $(BUILD)/footprint-r4f.json

m23-footprint-r4g: | $(BUILD)
	$(CLANG) $(M23_COMMON) -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=24 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		-DARGHTOS_SPECIALIZED_SERVICES=1 -DARGHTOS_MINIMAL_VALIDATION=1 \
		-DARGHTOS_RUNTIME_PROFILING=0 -DARGHTOS_SIZE_FIRST_SELECTOR=1 \
		-DARGHTOS_SIZE_FIRST_NO_DISPATCH_TIME=1 \
		-c src/arghtos_kernel.c -o $(BUILD)/arghtos_kernel_m23_r4g.o
	$(CLANG) $(M23_COMMON) -c port/cortex_m23/arghtos_port.c \
		-o $(BUILD)/arghtos_port_m23_r4g.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb \
		-ffunction-sections -c port/cortex_m23/arghtos_context.S \
		-o $(BUILD)/arghtos_context_m23_r4g.o
	$(CLANG) $(M23_COMMON) -c footprint/m23_board_stub.c \
		-o $(BUILD)/m23_board_stub_r4g.o
	$(CLANG) $(M23_COMMON) -DARGHTOS_STATIC_CONFIG=1 \
		-DARGHTOS_STATIC_TASK_COUNT=24 -DARGHTOS_COMPACT_RUNTIME=1 \
		-DARGHTOS_COMPACT_DESCRIPTOR=1 -DARGHTOS_UNIFIED_SLOT=1 \
		-DARGHTOS_PRODUCTION_PROFILE=1 -DARGHTOS_SPECIALIZED_SERVICES=1 \
		-DARGHTOS_MINIMAL_VALIDATION=1 -DARGHTOS_RUNTIME_PROFILING=0 \
		-DARGHTOS_SIZE_FIRST_SELECTOR=1 -DARGHTOS_SIZE_FIRST_NO_DISPATCH_TIME=1 \
		-c footprint/m23_footprint.c -o $(BUILD)/m23_footprint_r4g.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb -nostdlib \
		-fuse-ld=lld -Wl,--gc-sections -Wl,-Map=$(BUILD)/m23-r4g.map \
		-Wl,-T,footprint/m23.ld -o $(BUILD)/m23-r4g.elf \
		$(BUILD)/arghtos_kernel_m23_r4g.o \
		$(BUILD)/arghtos_port_m23_r4g.o \
		$(BUILD)/arghtos_context_m23_r4g.o \
		$(BUILD)/m23_board_stub_r4g.o \
		$(BUILD)/m23_footprint_r4g.o
	size -A $(BUILD)/m23-r4g.elf
	$(PYTHON) tools/footprint_report.py --map $(BUILD)/m23-r4g.map \
		--msp $(M23_ARGHTOS_MSP) --json \
		> $(BUILD)/footprint-r4g.json
	cat $(BUILD)/footprint-r4g.json

m23-footprint-r4h: | $(BUILD)
	$(CLANG) $(M23_COMMON) -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=24 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		-DARGHTOS_SPECIALIZED_SERVICES=1 -DARGHTOS_MINIMAL_VALIDATION=1 \
		-DARGHTOS_RUNTIME_PROFILING=0 -DARGHTOS_SIZE_FIRST_SELECTOR=1 \
		-DARGHTOS_SIZE_FIRST_NO_DISPATCH_TIME=1 -DARGHTOS_SIZE_FIRST_NO_START_TIME=1 \
		-c src/arghtos_kernel.c -o $(BUILD)/arghtos_kernel_m23_r4h.o
	$(CLANG) $(M23_COMMON) -c port/cortex_m23/arghtos_port.c \
		-o $(BUILD)/arghtos_port_m23_r4h.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb \
		-ffunction-sections -c port/cortex_m23/arghtos_context.S \
		-o $(BUILD)/arghtos_context_m23_r4h.o
	$(CLANG) $(M23_COMMON) -c footprint/m23_board_stub.c \
		-o $(BUILD)/m23_board_stub_r4h.o
	$(CLANG) $(M23_COMMON) -DARGHTOS_STATIC_CONFIG=1 \
		-DARGHTOS_STATIC_TASK_COUNT=24 -DARGHTOS_COMPACT_RUNTIME=1 \
		-DARGHTOS_COMPACT_DESCRIPTOR=1 -DARGHTOS_UNIFIED_SLOT=1 \
		-DARGHTOS_PRODUCTION_PROFILE=1 -DARGHTOS_SPECIALIZED_SERVICES=1 \
		-DARGHTOS_MINIMAL_VALIDATION=1 -DARGHTOS_RUNTIME_PROFILING=0 \
		-DARGHTOS_SIZE_FIRST_SELECTOR=1 -DARGHTOS_SIZE_FIRST_NO_DISPATCH_TIME=1 \
		-DARGHTOS_SIZE_FIRST_NO_START_TIME=1 \
		-c footprint/m23_footprint.c -o $(BUILD)/m23_footprint_r4h.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb -nostdlib \
		-fuse-ld=lld -Wl,--gc-sections -Wl,-Map=$(BUILD)/m23-r4h.map \
		-Wl,-T,footprint/m23.ld -o $(BUILD)/m23-r4h.elf \
		$(BUILD)/arghtos_kernel_m23_r4h.o \
		$(BUILD)/arghtos_port_m23_r4h.o \
		$(BUILD)/arghtos_context_m23_r4h.o \
		$(BUILD)/m23_board_stub_r4h.o \
		$(BUILD)/m23_footprint_r4h.o
	size -A $(BUILD)/m23-r4h.elf
	$(PYTHON) tools/footprint_report.py --map $(BUILD)/m23-r4h.map \
		--msp $(M23_ARGHTOS_MSP) --json \
		> $(BUILD)/footprint-r4h.json
	cat $(BUILD)/footprint-r4h.json

m23-footprint-r4i: | $(BUILD)
	$(CLANG) $(M23_COMMON) -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=24 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		-DARGHTOS_SPECIALIZED_SERVICES=1 -DARGHTOS_MINIMAL_VALIDATION=1 \
		-DARGHTOS_RUNTIME_PROFILING=0 -DARGHTOS_SIZE_FIRST_SELECTOR=1 \
		-DARGHTOS_SIZE_FIRST_NO_DISPATCH_TIME=1 -DARGHTOS_SIZE_FIRST_NO_START_TIME=1 \
		-DARGHTOS_SIZE_FIRST_SERVICE_IF_CHAIN=1 \
		-c src/arghtos_kernel.c -o $(BUILD)/arghtos_kernel_m23_r4i.o
	$(CLANG) $(M23_COMMON) -c port/cortex_m23/arghtos_port.c \
		-o $(BUILD)/arghtos_port_m23_r4i.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb \
		-ffunction-sections -c port/cortex_m23/arghtos_context.S \
		-o $(BUILD)/arghtos_context_m23_r4i.o
	$(CLANG) $(M23_COMMON) -c footprint/m23_board_stub.c \
		-o $(BUILD)/m23_board_stub_r4i.o
	$(CLANG) $(M23_COMMON) -DARGHTOS_STATIC_CONFIG=1 \
		-DARGHTOS_STATIC_TASK_COUNT=24 -DARGHTOS_COMPACT_RUNTIME=1 \
		-DARGHTOS_COMPACT_DESCRIPTOR=1 -DARGHTOS_UNIFIED_SLOT=1 \
		-DARGHTOS_PRODUCTION_PROFILE=1 -DARGHTOS_SPECIALIZED_SERVICES=1 \
		-DARGHTOS_MINIMAL_VALIDATION=1 -DARGHTOS_RUNTIME_PROFILING=0 \
		-DARGHTOS_SIZE_FIRST_SELECTOR=1 -DARGHTOS_SIZE_FIRST_NO_DISPATCH_TIME=1 \
		-DARGHTOS_SIZE_FIRST_NO_START_TIME=1 -DARGHTOS_SIZE_FIRST_SERVICE_IF_CHAIN=1 \
		-c footprint/m23_footprint.c -o $(BUILD)/m23_footprint_r4i.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb -nostdlib \
		-fuse-ld=lld -Wl,--gc-sections -Wl,-Map=$(BUILD)/m23-r4i.map \
		-Wl,-T,footprint/m23.ld -o $(BUILD)/m23-r4i.elf \
		$(BUILD)/arghtos_kernel_m23_r4i.o \
		$(BUILD)/arghtos_port_m23_r4i.o \
		$(BUILD)/arghtos_context_m23_r4i.o \
		$(BUILD)/m23_board_stub_r4i.o \
		$(BUILD)/m23_footprint_r4i.o
	size -A $(BUILD)/m23-r4i.elf
	$(PYTHON) tools/footprint_report.py --map $(BUILD)/m23-r4i.map \
		--msp $(M23_ARGHTOS_MSP) --json \
		> $(BUILD)/footprint-r4i.json
	cat $(BUILD)/footprint-r4i.json

m23-footprint-r4j: | $(BUILD)
	$(CLANG) $(M23_COMMON) -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=24 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		-DARGHTOS_SPECIALIZED_SERVICES=1 -DARGHTOS_MINIMAL_VALIDATION=1 \
		-DARGHTOS_RUNTIME_PROFILING=0 -DARGHTOS_SIZE_FIRST_SELECTOR=1 \
		-DARGHTOS_SIZE_FIRST_NO_DISPATCH_TIME=1 -DARGHTOS_SIZE_FIRST_NO_START_TIME=1 \
		-DARGHTOS_SIZE_FIRST_FAULT_RANGE=1 \
		-c src/arghtos_kernel.c -o $(BUILD)/arghtos_kernel_m23_r4j.o
	$(CLANG) $(M23_COMMON) -c port/cortex_m23/arghtos_port.c \
		-o $(BUILD)/arghtos_port_m23_r4j.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb \
		-ffunction-sections -c port/cortex_m23/arghtos_context.S \
		-o $(BUILD)/arghtos_context_m23_r4j.o
	$(CLANG) $(M23_COMMON) -c footprint/m23_board_stub.c \
		-o $(BUILD)/m23_board_stub_r4j.o
	$(CLANG) $(M23_COMMON) -DARGHTOS_STATIC_CONFIG=1 \
		-DARGHTOS_STATIC_TASK_COUNT=24 -DARGHTOS_COMPACT_RUNTIME=1 \
		-DARGHTOS_COMPACT_DESCRIPTOR=1 -DARGHTOS_UNIFIED_SLOT=1 \
		-DARGHTOS_PRODUCTION_PROFILE=1 -DARGHTOS_SPECIALIZED_SERVICES=1 \
		-DARGHTOS_MINIMAL_VALIDATION=1 -DARGHTOS_RUNTIME_PROFILING=0 \
		-DARGHTOS_SIZE_FIRST_SELECTOR=1 -DARGHTOS_SIZE_FIRST_NO_DISPATCH_TIME=1 \
		-DARGHTOS_SIZE_FIRST_NO_START_TIME=1 -DARGHTOS_SIZE_FIRST_FAULT_RANGE=1 \
		-c footprint/m23_footprint.c -o $(BUILD)/m23_footprint_r4j.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb -nostdlib \
		-fuse-ld=lld -Wl,--gc-sections -Wl,-Map=$(BUILD)/m23-r4j.map \
		-Wl,-T,footprint/m23.ld -o $(BUILD)/m23-r4j.elf \
		$(BUILD)/arghtos_kernel_m23_r4j.o \
		$(BUILD)/arghtos_port_m23_r4j.o \
		$(BUILD)/arghtos_context_m23_r4j.o \
		$(BUILD)/m23_board_stub_r4j.o \
		$(BUILD)/m23_footprint_r4j.o
	size -A $(BUILD)/m23-r4j.elf
	$(PYTHON) tools/footprint_report.py --map $(BUILD)/m23-r4j.map \
		--msp $(M23_ARGHTOS_MSP) --json \
		> $(BUILD)/footprint-r4j.json
	cat $(BUILD)/footprint-r4j.json

m23-footprint-r4k: | $(BUILD)
	$(CLANG) $(M23_COMMON) -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=24 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		-DARGHTOS_SPECIALIZED_SERVICES=1 -DARGHTOS_MINIMAL_VALIDATION=1 \
		-DARGHTOS_RUNTIME_PROFILING=0 -DARGHTOS_SIZE_FIRST_SELECTOR=1 \
		-DARGHTOS_SIZE_FIRST_NO_DISPATCH_TIME=1 -DARGHTOS_SIZE_FIRST_NO_START_TIME=1 \
		-DARGHTOS_SIZE_FIRST_TIME_TWO_PASS=1 \
		-c src/arghtos_kernel.c -o $(BUILD)/arghtos_kernel_m23_r4k.o
	$(CLANG) $(M23_COMMON) -c port/cortex_m23/arghtos_port.c \
		-o $(BUILD)/arghtos_port_m23_r4k.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb \
		-ffunction-sections -c port/cortex_m23/arghtos_context.S \
		-o $(BUILD)/arghtos_context_m23_r4k.o
	$(CLANG) $(M23_COMMON) -c footprint/m23_board_stub.c \
		-o $(BUILD)/m23_board_stub_r4k.o
	$(CLANG) $(M23_COMMON) -DARGHTOS_STATIC_CONFIG=1 \
		-DARGHTOS_STATIC_TASK_COUNT=24 -DARGHTOS_COMPACT_RUNTIME=1 \
		-DARGHTOS_COMPACT_DESCRIPTOR=1 -DARGHTOS_UNIFIED_SLOT=1 \
		-DARGHTOS_PRODUCTION_PROFILE=1 -DARGHTOS_SPECIALIZED_SERVICES=1 \
		-DARGHTOS_MINIMAL_VALIDATION=1 -DARGHTOS_RUNTIME_PROFILING=0 \
		-DARGHTOS_SIZE_FIRST_SELECTOR=1 -DARGHTOS_SIZE_FIRST_NO_DISPATCH_TIME=1 \
		-DARGHTOS_SIZE_FIRST_NO_START_TIME=1 -DARGHTOS_SIZE_FIRST_TIME_TWO_PASS=1 \
		-c footprint/m23_footprint.c -o $(BUILD)/m23_footprint_r4k.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb -nostdlib \
		-fuse-ld=lld -Wl,--gc-sections -Wl,-Map=$(BUILD)/m23-r4k.map \
		-Wl,-T,footprint/m23.ld -o $(BUILD)/m23-r4k.elf \
		$(BUILD)/arghtos_kernel_m23_r4k.o \
		$(BUILD)/arghtos_port_m23_r4k.o \
		$(BUILD)/arghtos_context_m23_r4k.o \
		$(BUILD)/m23_board_stub_r4k.o \
		$(BUILD)/m23_footprint_r4k.o
	size -A $(BUILD)/m23-r4k.elf
	$(PYTHON) tools/footprint_report.py --map $(BUILD)/m23-r4k.map \
		--msp $(M23_ARGHTOS_MSP) --json \
		> $(BUILD)/footprint-r4k.json
	cat $(BUILD)/footprint-r4k.json

m23-footprint-r4l: | $(BUILD)
	$(CLANG) $(M23_COMMON) -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=24 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		-DARGHTOS_SPECIALIZED_SERVICES=1 -DARGHTOS_MINIMAL_VALIDATION=1 \
		-DARGHTOS_RUNTIME_PROFILING=0 -DARGHTOS_SIZE_FIRST_SELECTOR=1 \
		-DARGHTOS_SIZE_FIRST_NO_DISPATCH_TIME=1 -DARGHTOS_SIZE_FIRST_NO_START_TIME=1 \
		-DARGHTOS_SIZE_FIRST_WAKE_RESULT_HOIST=1 \
		-c src/arghtos_kernel.c -o $(BUILD)/arghtos_kernel_m23_r4l.o
	$(CLANG) $(M23_COMMON) -c port/cortex_m23/arghtos_port.c \
		-o $(BUILD)/arghtos_port_m23_r4l.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb \
		-ffunction-sections -c port/cortex_m23/arghtos_context.S \
		-o $(BUILD)/arghtos_context_m23_r4l.o
	$(CLANG) $(M23_COMMON) -c footprint/m23_board_stub.c \
		-o $(BUILD)/m23_board_stub_r4l.o
	$(CLANG) $(M23_COMMON) -DARGHTOS_STATIC_CONFIG=1 \
		-DARGHTOS_STATIC_TASK_COUNT=24 -DARGHTOS_COMPACT_RUNTIME=1 \
		-DARGHTOS_COMPACT_DESCRIPTOR=1 -DARGHTOS_UNIFIED_SLOT=1 \
		-DARGHTOS_PRODUCTION_PROFILE=1 -DARGHTOS_SPECIALIZED_SERVICES=1 \
		-DARGHTOS_MINIMAL_VALIDATION=1 -DARGHTOS_RUNTIME_PROFILING=0 \
		-DARGHTOS_SIZE_FIRST_SELECTOR=1 -DARGHTOS_SIZE_FIRST_NO_DISPATCH_TIME=1 \
		-DARGHTOS_SIZE_FIRST_NO_START_TIME=1 -DARGHTOS_SIZE_FIRST_WAKE_RESULT_HOIST=1 \
		-c footprint/m23_footprint.c -o $(BUILD)/m23_footprint_r4l.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb -nostdlib \
		-fuse-ld=lld -Wl,--gc-sections -Wl,-Map=$(BUILD)/m23-r4l.map \
		-Wl,-T,footprint/m23.ld -o $(BUILD)/m23-r4l.elf \
		$(BUILD)/arghtos_kernel_m23_r4l.o \
		$(BUILD)/arghtos_port_m23_r4l.o \
		$(BUILD)/arghtos_context_m23_r4l.o \
		$(BUILD)/m23_board_stub_r4l.o \
		$(BUILD)/m23_footprint_r4l.o
	size -A $(BUILD)/m23-r4l.elf
	$(PYTHON) tools/footprint_report.py --map $(BUILD)/m23-r4l.map \
		--msp $(M23_ARGHTOS_MSP) --json \
		> $(BUILD)/footprint-r4l.json
	cat $(BUILD)/footprint-r4l.json

m23-footprint-r4m: | $(BUILD)
	$(CLANG) $(M23_COMMON) -flto -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=24 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		-DARGHTOS_SPECIALIZED_SERVICES=1 -DARGHTOS_MINIMAL_VALIDATION=1 \
		-DARGHTOS_RUNTIME_PROFILING=0 -DARGHTOS_SIZE_FIRST_SELECTOR=1 \
		-DARGHTOS_SIZE_FIRST_NO_DISPATCH_TIME=1 -DARGHTOS_SIZE_FIRST_NO_START_TIME=1 \
		-c src/arghtos_kernel.c -o $(BUILD)/arghtos_kernel_m23_r4m.o
	$(CLANG) $(M23_COMMON) -flto -c port/cortex_m23/arghtos_port.c \
		-o $(BUILD)/arghtos_port_m23_r4m.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb \
		-ffunction-sections -c port/cortex_m23/arghtos_context.S \
		-o $(BUILD)/arghtos_context_m23_r4m.o
	$(CLANG) $(M23_COMMON) -flto -c footprint/m23_board_stub.c \
		-o $(BUILD)/m23_board_stub_r4m.o
	$(CLANG) $(M23_COMMON) -flto -DARGHTOS_STATIC_CONFIG=1 \
		-DARGHTOS_STATIC_TASK_COUNT=24 -DARGHTOS_COMPACT_RUNTIME=1 \
		-DARGHTOS_COMPACT_DESCRIPTOR=1 -DARGHTOS_UNIFIED_SLOT=1 \
		-DARGHTOS_PRODUCTION_PROFILE=1 -DARGHTOS_SPECIALIZED_SERVICES=1 \
		-DARGHTOS_MINIMAL_VALIDATION=1 -DARGHTOS_RUNTIME_PROFILING=0 \
		-DARGHTOS_SIZE_FIRST_SELECTOR=1 -DARGHTOS_SIZE_FIRST_NO_DISPATCH_TIME=1 \
		-DARGHTOS_SIZE_FIRST_NO_START_TIME=1 \
		-c footprint/m23_footprint.c -o $(BUILD)/m23_footprint_r4m.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb -nostdlib -flto \
		-fuse-ld=lld -Wl,--gc-sections -Wl,-Map=$(BUILD)/m23-r4m.map \
		-Wl,-T,footprint/m23.ld -o $(BUILD)/m23-r4m.elf \
		$(BUILD)/arghtos_kernel_m23_r4m.o \
		$(BUILD)/arghtos_port_m23_r4m.o \
		$(BUILD)/arghtos_context_m23_r4m.o \
		$(BUILD)/m23_board_stub_r4m.o \
		$(BUILD)/m23_footprint_r4m.o
	size -A $(BUILD)/m23-r4m.elf
	$(PYTHON) tools/footprint_report.py --map $(BUILD)/m23-r4m.map \
		--msp $(M23_ARGHTOS_MSP_LTO) --json \
		> $(BUILD)/footprint-r4m.json
	cat $(BUILD)/footprint-r4m.json

m23-footprint-r4n: | $(BUILD)
	$(CLANG) $(M23_COMMON) -flto -fomit-frame-pointer -DARGHTOS_TRUSTED_CONFIG=1 \
		-DARGHTOS_STATIC_CONFIG=1 -DARGHTOS_STATIC_TASK_COUNT=24 \
		-DARGHTOS_COMPACT_RUNTIME=1 -DARGHTOS_COMPACT_DESCRIPTOR=1 \
		-DARGHTOS_UNIFIED_SLOT=1 -DARGHTOS_PRODUCTION_PROFILE=1 \
		-DARGHTOS_SPECIALIZED_SERVICES=1 -DARGHTOS_MINIMAL_VALIDATION=1 \
		-DARGHTOS_RUNTIME_PROFILING=0 -DARGHTOS_SIZE_FIRST_SELECTOR=1 \
		-DARGHTOS_SIZE_FIRST_NO_DISPATCH_TIME=1 -DARGHTOS_SIZE_FIRST_NO_START_TIME=1 \
		-c src/arghtos_kernel.c -o $(BUILD)/arghtos_kernel_m23_r4n.o
	$(CLANG) $(M23_COMMON) -flto -fomit-frame-pointer -c port/cortex_m23/arghtos_port.c \
		-o $(BUILD)/arghtos_port_m23_r4n.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb \
		-ffunction-sections -c port/cortex_m23/arghtos_context.S \
		-o $(BUILD)/arghtos_context_m23_r4n.o
	$(CLANG) $(M23_COMMON) -flto -fomit-frame-pointer -c footprint/m23_board_stub.c \
		-o $(BUILD)/m23_board_stub_r4n.o
	$(CLANG) $(M23_COMMON) -flto -fomit-frame-pointer -DARGHTOS_STATIC_CONFIG=1 \
		-DARGHTOS_STATIC_TASK_COUNT=24 -DARGHTOS_COMPACT_RUNTIME=1 \
		-DARGHTOS_COMPACT_DESCRIPTOR=1 -DARGHTOS_UNIFIED_SLOT=1 \
		-DARGHTOS_PRODUCTION_PROFILE=1 -DARGHTOS_SPECIALIZED_SERVICES=1 \
		-DARGHTOS_MINIMAL_VALIDATION=1 -DARGHTOS_RUNTIME_PROFILING=0 \
		-DARGHTOS_SIZE_FIRST_SELECTOR=1 -DARGHTOS_SIZE_FIRST_NO_DISPATCH_TIME=1 \
		-DARGHTOS_SIZE_FIRST_NO_START_TIME=1 \
		-c footprint/m23_footprint.c -o $(BUILD)/m23_footprint_r4n.o
	$(CLANG) --target=arm-none-eabi -mcpu=cortex-m23 -mthumb -nostdlib -flto -fomit-frame-pointer \
		-fuse-ld=lld -Wl,--gc-sections -Wl,-Map=$(BUILD)/m23-r4n.map \
		-Wl,-T,footprint/m23.ld -o $(BUILD)/m23-r4n.elf \
		$(BUILD)/arghtos_kernel_m23_r4n.o \
		$(BUILD)/arghtos_port_m23_r4n.o \
		$(BUILD)/arghtos_context_m23_r4n.o \
		$(BUILD)/m23_board_stub_r4n.o \
		$(BUILD)/m23_footprint_r4n.o
	size -A $(BUILD)/m23-r4n.elf
	$(PYTHON) tools/footprint_report.py --map $(BUILD)/m23-r4n.map \
		--msp $(M23_ARGHTOS_MSP_LTO) --json \
		> $(BUILD)/footprint-r4n.json
	cat $(BUILD)/footprint-r4n.json

footprint-report: m23-footprint

clean:
	rm -rf $(BUILD)
