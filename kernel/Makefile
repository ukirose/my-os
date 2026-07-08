QEMU = qemu-system-riscv32
CC = /opt/homebrew/opt/llvm/bin/clang

# ベアメタルカーネルとしてコンパイルし、標準ライブラリの自動リンクを防ぐ
CFLAGS = -target riscv32 -mabi=ilp32 -mcmodel=medany -std=c11 -ffreestanding -fno-common -nostdlib -fuse-ld=lld

BUILD_DIR = ../build
KERNEL_ELF = $(BUILD_DIR)/kernel.elf
KERNEL_MAP = $(BUILD_DIR)/kernel.map

OBJS = $(BUILD_DIR)/main.o $(BUILD_DIR)/sbi.o $(BUILD_DIR)/string.o $(BUILD_DIR)/kprintf.o

.PHONY: all run clean

all: $(KERNEL_ELF)

$(BUILD_DIR)/main.o: main.c kernel.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c main.c -o $@

$(BUILD_DIR)/sbi.o: sbi.c kernel.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c sbi.c -o $@

$(BUILD_DIR)/string.o: string.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c string.c -o $@

$(BUILD_DIR)/kprintf.o: kprintf.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c kprintf.c -o $@

$(KERNEL_ELF): $(OBJS) kernel.ld
	$(CC) $(CFLAGS) -T kernel.ld -Wl,-Map=$(KERNEL_MAP) $(OBJS) -o $@

run: $(KERNEL_ELF)
	$(QEMU) -machine virt -bios default -nographic -serial mon:stdio --no-reboot -kernel $(KERNEL_ELF)

clean:
	rm -f $(BUILD_DIR)/kernel.*
