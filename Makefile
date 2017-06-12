STU_ID = 151220122

BIN_DIR := bin
BOOT   := $(BIN_DIR)/boot.bin
KERNEL := $(BIN_DIR)/kernel.bin
TEST   := $(BIN_DIR)/test.bin
SEM	   := $(BIN_DIR)/sem.bin
GAME   := $(BIN_DIR)/game.bin
PROGRAM := $(BIN_DIR)/program.bin
FORMATTER := $(BIN_DIR)/formatter
COPY2MYFS := $(BIN_DIR)/copy2myfs
READ_MYFS := $(BIN_DIR)/read_myfs
IMAGE  := disk.bin



# Could be switched between GAME, TEST and SEM.
# TARGET := $(SEM) Now we have file system, so it is no longer needed.

CC      := gcc
LD      := ld
OBJCOPY := objcopy
DD      := dd
QEMU    := qemu-system-i386
GDB     := gdb

CFLAGS := -Wall -Werror -Wfatal-errors #开启所有警告, 视警告为错误, 第一个错误结束编译
CFLAGS += -MD #生成依赖文件
CFLAGS += -std=gnu11 -m32 -c #编译标准, 目标架构, 只编译
CFLAGS += -I . #头文件搜索目录
CFLAGS += -I ./lib/include
CFLAGS += -O0 #不开优化, 方便调试
CFLAGS += -fno-builtin #禁止内置函数
CFLAGS += -ggdb3 #GDB调试信息
CFLAGS += -fno-stack-protector

QEMU_OPTIONS := -serial stdio #以标准输入输为串口(COM1)
#QEMU_OPTIONS += -d int #输出中断信息
QEMU_OPTIONS += -monitor telnet:127.0.0.1:1111,server,nowait #telnet monitor

QEMU_DEBUG_OPTIONS := -S #启动不执行
QEMU_DEBUG_OPTIONS += -s #GDB调试服务器: 127.0.0.1:1234

GDB_OPTIONS := -ex "target remote 127.0.0.1:1234"
GDB_OPTIONS += -ex "symbol $(KERNEL)"


OBJ_DIR        := obj
LIB_DIR        := lib
BOOT_DIR       := boot
KERNEL_DIR     := kernel
TEST_DIR       := test
SEM_DIR		   := sem
GAME_DIR	   := game
TOOLS_DIR 	   := tools
OBJ_LIB_DIR    := $(OBJ_DIR)/$(LIB_DIR)
OBJ_BOOT_DIR   := $(OBJ_DIR)/$(BOOT_DIR)
OBJ_KERNEL_DIR := $(OBJ_DIR)/$(KERNEL_DIR)
OBJ_GAME_DIR   := $(OBJ_DIR)/$(GAME_DIR)
OBJ_TEST_DIR   := $(OBJ_DIR)/$(TEST_DIR)
OBJ_SEM_DIR   := $(OBJ_DIR)/$(SEM_DIR)

LD_SCRIPT := $(shell find $(KERNEL_DIR) -name "*.ld")
GAME_LD_SCRIPT	 := $(shell find $(GAME_DIR) -name "*.ld")
TEST_LD_SCRIPT	 := $(shell find $(TEST_DIR) -name "*.ld")
SEM_LD_SCRIPT	 := $(shell find $(SEM_DIR) -name "*.ld")

LIB_C := $(wildcard $(LIB_DIR)/*.c)
LIB_O := $(LIB_C:%.c=$(OBJ_DIR)/%.o)

BOOT_S := $(wildcard $(BOOT_DIR)/*.S)
BOOT_C := $(wildcard $(BOOT_DIR)/*.c)
BOOT_O := $(BOOT_S:%.S=$(OBJ_DIR)/%.o)
BOOT_O += $(BOOT_C:%.c=$(OBJ_DIR)/%.o)

KERNEL_C := $(shell find $(KERNEL_DIR) -name "*.c")
KERNEL_S := $(shell find $(KERNEL_DIR) -name "*.S")
KERNEL_O := $(KERNEL_C:%.c=$(OBJ_DIR)/%.o)
KERNEL_O += $(KERNEL_S:%.S=$(OBJ_DIR)/%.o)

GAME_C := $(shell find $(GAME_DIR) -name "*.c")
GAME_O := $(GAME_C:%.c=$(OBJ_DIR)/%.o)
GAME_DAT := $(shell find $(GAME_DIR) -name "*.dat")

SEM_C := $(shell find $(SEM_DIR) -name "*.c")
SEM_O := $(SEM_C:%.c=$(OBJ_DIR)/%.o)

TEST_C := $(shell find $(TEST_DIR) -name "*.c")
TEST_O := $(TEST_C:%.c=$(OBJ_DIR)/%.o)

FORMATTER_C := $(TOOLS_DIR)/formatter.c
COPY2MYFS_C := $(TOOLS_DIR)/copy2myfs.c
READ_MYFS_C := $(TOOLS_DIR)/read_myfs.c

$(IMAGE): $(BOOT) $(PROGRAM) $(FORMATTER) $(COPY2MYFS) $(READ_MYFS)
	@mkdir -p $(BIN_DIR)
#	@$(DD) if=/dev/zero of=$(IMAGE) count=10000         > /dev/null # 准备磁盘文件	total size: 5000 KB
#	@$(DD) if=$(BOOT) of=$(IMAGE) conv=notrunc          > /dev/null # 填充 boot loader
#	@$(DD) if=$(PROGRAM) of=$(IMAGE) seek=128 conv=notrunc > /dev/null # 填充 kernel, 跨过 mbr?  boot!!!
	@cd ./bin; ./formatter; ./copy2myfs
	@mv ./bin/disk.bin $(IMAGE)


$(FORMATTER): $(FORMATTER_C)
	@mkdir -p $(BIN_DIR)
	@gcc $< -o $@
	@echo cc $< -o $@

$(COPY2MYFS): $(COPY2MYFS_C)
	@mkdir -p $(BIN_DIR)
	@gcc $< -o $@
	@echo cc $< -o $@
	
$(READ_MYFS): $(READ_MYFS_C)
	@mkdir -p $(BIN_DIR)
	@gcc $< -o $@
	@echo cc $< -o $@

$(BOOT): $(BOOT_O)
	@mkdir -p $(BIN_DIR)
	@echo ld -o $@
	@$(LD) -e start -Ttext=0x7C00 -m elf_i386 -nostdlib -o $@.out $^
	@echo objcopy $@.out $@
	@$(OBJCOPY) --strip-all --only-section=.text --output-target=binary $@.out $@
	@rm $@.out
	perl ./boot/genboot.pl $@


$(OBJ_BOOT_DIR)/%.o: $(BOOT_DIR)/%.[cS]
	@mkdir -p $(OBJ_BOOT_DIR)
	@echo cc $< -o $@
	@$(CC) $(CFLAGS) -Os -I ./boot/include $< -o $@
	

$(PROGRAM): $(KERNEL) $(GAME) $(TEST) $(SEM)
	@mkdir -p $(BIN_DIR)


$(KERNEL): $(LD_SCRIPT)
$(KERNEL): $(KERNEL_O) $(LIB_O)
	@mkdir -p $(BIN_DIR)
	@echo ld -o $@
	@$(LD) -m elf_i386 -T $(LD_SCRIPT) -nostdlib -o $@ $^ $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)
#	perl ./kernel/genkernel.pl

$(OBJ_LIB_DIR)/%.o : $(LIB_DIR)/%.c
	@mkdir -p $(OBJ_LIB_DIR)
	@echo cc $< -o $@
	@$(CC) $(CFLAGS) $< -o $@

$(OBJ_KERNEL_DIR)/%.o: $(KERNEL_DIR)/%.[cS]
	@mkdir -p $(OBJ_DIR)/$(dir $<)
	@echo cc $< -o $@
	@$(CC) $(CFLAGS) -I ./kernel/include $< -o $@
	
	
$(GAME): $(GAME_LD_SCRIPT)
$(GAME): $(GAME_O) $(LIB_O)
	@mkdir -p $(BIN_DIR)
	@echo ld -o $@
	@$(LD) -m elf_i386 -T $(GAME_LD_SCRIPT) -nostdlib -o $@ $^ $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)
	@cp -r $(GAME_DAT) $< $(BIN_DIR)		#copy game data to bin directory

$(OBJ_GAME_DIR)/%.o: $(GAME_DIR)/%.c
	@mkdir -p $(OBJ_DIR)/$(dir $<)
	@echo cc $< -o $@
	@$(CC) $(CFLAGS) -I ./game/include $< -o $@
	
	
$(TEST): $(TEST_LD_SCRIPT)
$(TEST): $(TEST_O) $(LIB_O)
	@mkdir -p $(BIN_DIR)
	@echo ld -o $@
	@$(LD) -m elf_i386 -T $(TEST_LD_SCRIPT) -nostdlib -o $@ $^ $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)

$(OBJ_TEST_DIR)/%.o: $(TEST_DIR)/%.c
	@mkdir -p $(OBJ_DIR)/$(dir $<)
	@echo cc $< -o $@
	@$(CC) $(CFLAGS) -I ./test/include $< -o $@
	
	
$(SEM): $(SEM_LD_SCRIPT)
$(SEM): $(SEM_O) $(LIB_O)
	@mkdir -p $(BIN_DIR)
	@echo ld -o $@
	@$(LD) -m elf_i386 -T $(SEM_LD_SCRIPT) -nostdlib -o $@ $^ $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)

$(OBJ_SEM_DIR)/%.o: $(SEM_DIR)/%.c
	@mkdir -p $(OBJ_DIR)/$(dir $<)
	@echo cc $< -o $@
	@$(CC) $(CFLAGS) -I ./sem/include $< -o $@

	

DEPS := $(shell find -name "*.d")
-include $(DEPS)

.PHONY: qemu debug gdb clean submit

qemu: $(IMAGE)
	$(QEMU) $(QEMU_OPTIONS) $(IMAGE)

# Faster, but not suitable for debugging
qemu-kvm: $(IMAGE)
	$(QEMU) $(QEMU_OPTIONS) --enable-kvm $(IMAGE)

debug: $(IMAGE)
	$(QEMU) $(QEMU_DEBUG_OPTIONS) $(QEMU_OPTIONS) $(IMAGE)

gdb:
	$(GDB) $(GDB_OPTIONS)

clean:
	@rm -rf $(OBJ_DIR) 2> /dev/null
	@rm -rf $(IMAGE)   2> /dev/null
	@rm -rf $(BIN_DIR) 2> /dev/null


submit: clean
	cd .. && tar cvj $(shell pwd | grep -o '[^/]*$$') > $(STU_ID).tar.bz2

