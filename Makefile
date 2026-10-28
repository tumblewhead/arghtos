HOST_CC ?= gcc
CLANG ?= clang
PYTHON ?= python3
BUILD ?= build

HOST_CFLAGS := -std=c11 -Wall -Wextra -Werror -O2 -DARGHTOS_TEST \
	-Iinclude -Isrc -Iport/host
HOST_SRCS := src/arghtos_kernel.c port/host/arghtos_port.c tests/test_kernel.c

M23_COMMON := --target=arm-none-eabi -mcpu=cortex-m23 -mthumb -std=c11 \
	-ffreestanding -fno-builtin -fdata-sections -ffunction-sections \
	-fno-unwind-tables -fno-asynchronous-unwind-tables -Oz \
	-Wall -Wextra -Werror -Iinclude -Isrc -Iport/cortex_m23

.PHONY: all host-test host-sanitize m23-compile m23-footprint footprint-report clean

all: host-test m23-compile

$(BUILD):
	mkdir -p $(BUILD)

host-test: | $(BUILD)
	$(HOST_CC) $(HOST_CFLAGS) $(HOST_SRCS) -o $(BUILD)/host-tests
	$(BUILD)/host-tests

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
	$(PYTHON) tools/footprint_report.py --map $(BUILD)/m23.map --json \
		> $(BUILD)/footprint.json
	cat $(BUILD)/footprint.json

footprint-report: m23-footprint

clean:
	rm -rf $(BUILD)
