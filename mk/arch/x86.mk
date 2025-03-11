# 选择编译器
include mk/compiler/clang.mk

# 目录配置
ARCH_DIR := arch/x86

INCLUDE_DIR := include $(ARCH_DIR)/include/

HEADERS = $(wildcard kernel/*.h drivers/*.h)
C_SOURCES = $(wildcard kernel/*.c kernel/core/*.c drivers/*.c drivers/modules/*.c $(ARCH_DIR)/hal/*.c)
ASM_SOURCES = $(wildcard $(ARCH_DIR)/hal/*.asm)

C_OBJECTS = $(patsubst %.c, $(TARGET_DIR)/%.o, $(C_SOURCES))
ASM_OBJECTS = $(patsubst %.asm, $(TARGET_DIR)/%.o, $(ASM_SOURCES))
OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS)

TARGET_TRIPLE := i386-unknown-none

ASFLAGS += -i $(ARCH_DIR)/boot/
CFLAGS += -fno-strict-aliasing -fno-stack-protector -fno-optimize-sibling-calls -O3 -mno-sse

# 通用编译标志
CFLAGS += -Wall -fno-builtin
CFLAGS += $(DEFINES)

CFLAGS += $(foreach dir,$(INCLUDE_DIR),-I$(dir))

override TARGET := $(TARGET_DIR)/kernel.img
override RUN := qemu

# Boot
$(TARGET_DIR)/bootblock.bin: $(ARCH_DIR)/boot/boot.asm | directories
	nasm -i $(ARCH_DIR)/boot/ $(ARCH_DIR)/boot/boot.asm -o $@

# Loader
$(TARGET_DIR)/loaderkernel.bin: $(ARCH_DIR)/boot/loader.asm $(ARCH_DIR)/boot/loadermain.c | $(TOOL_TARGETS) directories
	$(AS) $(ASFLAGS) $(ARCH_DIR)/boot/loader.asm -o $(BUILD_DIR)/loader.o
	$(CC) $(CFLAGS) -Iinclude -Iarch/x86/include -fno-builtin -fno-stack-protector -nostdinc -Os $(ARCH_DIR)/boot/loadermain.c -o $(BUILD_DIR)/loadermain.o
	$(LD) $(LDFLAGS) -N -e _start -Ttext=0x1000 --no-dynamic-linker $(BUILD_DIR)/loader.o $(BUILD_DIR)/loadermain.o -o $(BUILD_DIR)/loaderkernel.o
	$(OBJDUMP) -S $(BUILD_DIR)/loaderkernel.o > $(BUILD_DIR)/loaderkernel.asm
	$(OBJCOPY) -S -O binary -j .text $(BUILD_DIR)/loaderkernel.o $@

# Kernel
$(TARGET_DIR)/kernel.bin: $(TARGET_DIR)/$(ARCH_DIR)/boot/entry.o ${OBJECTS} | directories
	$(LD) $(LDFLAGS) -Ttext=0x100000 --no-dynamic-linker $^ -o $@
	$(OBJDUMP) -S $@ > $(BUILD_DIR)/kernel.asm

$(TARGET_DIR)/%.o: %.c ${HEADERS}
	mkdir -p $(@D)
	${CC} ${CFLAGS} $< -o $@

$(TARGET_DIR)/%.o: %.asm
	mkdir -p $(@D)
	$(AS) $(ASFLAGS) $< -o $@
# 镜像生成规则
$(TARGET_DIR)/kernel.img: $(TARGET_DIR)/kernel.bin $(TARGET_DIR)/loaderkernel.bin $(TARGET_DIR)/bootblock.bin
	dd of=$@ if=/dev/zero seek=0 count=100 bs=512
	dd of=$@ if=$(TARGET_DIR)/bootblock.bin seek=0 count=1 bs=512
	dd of=$@ if=$(TARGET_DIR)/loaderkernel.bin seek=1 count=4 bs=512
	dd of=$@ if=$(TARGET_DIR)/kernel.bin seek=5 bs=512
	@echo "Generated: $@"

qemu : $(TARGET)
	DISPLAY=:0 qemu-system-i386 -m 32M -drive file=$(TARGET_DIR)/kernel.img,format=raw -net nic -net tap,ifname=tap0,id=net0 \
		-object filter-dump,id=dump0,netdev=net0,file=qemu-dump.pcap \
		-monitor stdio
