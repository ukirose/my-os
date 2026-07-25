QEMU ?= qemu-system-riscv32
CC ?= clang
OBJDUMP ?= llvm-objdump
CFLAGS = -target riscv32 -mabi=ilp32 -mcmodel=medany -std=c11 -ffreestanding \
         -fno-common -nostdlib -fuse-ld=lld -MMD -MP

BUILD_DIR = ../build/kernel
KERNEL_ELF = $(BUILD_DIR)/kernel.elf
KERNEL_MAP = $(BUILD_DIR)/kernel.map

C_SRCS = $(wildcard *.c)
S_SRCS = $(wildcard *.S)
OBJS = $(addprefix $(BUILD_DIR)/, $(C_SRCS:.c=.o)) $(addprefix $(BUILD_DIR)/, $(S_SRCS:.S=.o))
DEPS = $(addprefix $(BUILD_DIR)/, $(C_SRCS:.c=.d))

.PHONY: all run clean debug dump

all: $(KERNEL_ELF)

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.S
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL_ELF): $(OBJS) kernel.ld
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -T kernel.ld -Wl,-Map=$(KERNEL_MAP) $(OBJS) -o $@

run: $(KERNEL_ELF)
	$(QEMU) -machine virt -bios default -nographic -serial mon:stdio --no-reboot -kernel $(KERNEL_ELF)

debug: $(KERNEL_ELF)
	$(QEMU) -machine virt -bios default -nographic -serial mon:stdio --no-reboot -kernel $(KERNEL_ELF) -d unimp,guest_errors,int,cpu_reset -D $(BUILD_DIR)/qemu.log

dump: $(KERNEL_ELF)
	$(OBJDUMP) -d $(KERNEL_ELF) > $(BUILD_DIR)/kernel.asm

clean:
	rm -f $(OBJS) $(DEPS) $(KERNEL_ELF) $(KERNEL_MAP)

-include $(DEPS)
