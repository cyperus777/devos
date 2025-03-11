.DEFAULT_GOAL := all

# 默认配置
ARCH ?= x86
# os or rtos
MODE ?= os

# 输出目录配置
OUT_DIR := bin
BUILD_DIR := $(OUT_DIR)/$(ARCH)_$(MODE)
TARGET_DIR := $(OUT_DIR)/$(ARCH)_$(MODE)
# 处理包含空格的路径
MAKE := "$(MAKE)"

# 导入配置
include mk/arch/$(ARCH).mk

# 创建输出目录
directories:
	@echo "Directories to create: $(BUILD_DIR) $(TARGET_DIR)"
	mkdir -p $(OUT_DIR) $(BUILD_DIR) $(TARGET_DIR)
# 默认目标
all : $(TOOL_TARGETS) directories $(TARGET)
	@echo "Building for ARCH=$(ARCH) MODE=$(MODE)"

#使用架构中的RUN替换
run : $(RUN)

# 清理
clean:
	rm -rf $(OUT_DIR)

#在debug指令时引用debug.mk
ifeq ($(MAKECMDGOALS),debug)
    -include mk/debug.mk
endif

.PHONY: all clean directories debug
