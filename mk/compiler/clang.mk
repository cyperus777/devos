# 基础工具链
CC := clang
LD := ld.lld
AS := nasm
AR := llvm-ar
OBJCOPY := llvm-objcopy
OBJDUMP := llvm-objdump
SIZE := llvm-size

# 基础编译标志
CFLAGS = -m32 -c -fno-pic -ffreestanding
# 链接标志
LDFLAGS += -nostdlib

# 汇编标志 (NASM)
ASFLAGS += -f elf32

# 根据目标三元组添加标志
CFLAGS += --target=$(TARGET_TRIPLE) 