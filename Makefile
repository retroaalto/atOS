# Makefile for atOS-RT

# Configurable paths
ASSEMBLER     ?= nasm
SOURCE_DIR    ?= SOURCE
SOURCE_KERNEL_DIR ?= $(SOURCE_DIR)/KERNEL
SOURCE_BOOTLOADER_DIR ?= $(SOURCE_DIR)/BOOTLOADER
OUTPUT_DIR    ?= OUTPUT
OUTPUT_KERNEL_DIR ?= $(OUTPUT_DIR)/KERNEL
OUTPUT_BOOTLOADER_DIR ?= $(OUTPUT_DIR)/BOOTLOADER
OUTPUT_ISO_DIR ?= $(OUTPUT_DIR)/ISO
INPUT_ISO_DIR  ?= ISO_DIR
ISO_NAME      ?= atOS.iso
IMG_NAME      ?= output.img

INPUT_ISO_DIR_SYSTEM ?= $(INPUT_ISO_DIR)/ATOS
INPUT_ISO_DIR_USER ?= $(INPUT_ISO_DIR)/USER
INPUT_ISO_DIR_PROGRAMS ?= $(INPUT_ISO_DIR)/PROGRAMS

HARD_DISK_SIZE ?= 128M
HARD_DISK_IMG ?= hdd.img
HARD_DISK_DIR ?= $(OUTPUT_DIR)/IMG

.PHONY: all kernel bootloader iso clean run debug help compiler shell

# Default target is to build the ISO
all: iso

# Bootloader build rule (16-bit x86 real mode)
$(OUTPUT_BOOTLOADER_DIR)/BOOTLOADER.BIN: $(SOURCE_BOOTLOADER_DIR)/BOOTLOADER.asm
	@echo "Compiling bootloader (16-bit real mode)..."
	mkdir -p $(OUTPUT_BOOTLOADER_DIR)
	$(ASSEMBLER) -f bin -D__REAL_MODE__ -o $@ $<
	@echo "Bootloader compiled successfully."

# Kernel build target
kernel: 
	@echo "Compiling KERNEL.BIN (16-bit real mode second stage)..."
	mkdir -p $(OUTPUT_KERNEL_DIR)
	$(ASSEMBLER) -f bin -D__REAL_PROTECTED_MODE__ $(SOURCE_KERNEL_DIR)/KERNEL_ENTRY.asm -o $(OUTPUT_KERNEL_DIR)/KERNEL.BIN
	@echo "KERNEL.BIN compiled successfully."
	@size=$$(stat -c%s "$(OUTPUT_KERNEL_DIR)/KERNEL.BIN"); \
	if [ $$size -gt 4095 ]; then \
		echo "\033[1;33mWARNING: KERNEL.BIN size is $$size bytes, which exceeds 4095 bytes!\033[0m"; \
	fi

	@echo "Compiling KRNL.BIN (32-bit protected mode entry)..."
	$(ASSEMBLER) -f bin -D__PROTECTED_MODE__ $(SOURCE_KERNEL_DIR)/KERNEL.asm -o $(OUTPUT_KERNEL_DIR)/KRNL.BIN
	@echo "KRNL.BIN compiled successfully."
	@size=$$(stat -c%s "$(OUTPUT_KERNEL_DIR)/KRNL.BIN"); \
	if [ $$size -gt 24000 ]; then \
		echo "\033[1;33mWARNING: KRNL.BIN size is $$size bytes, which exceeds 24000 bytes!\033[0m"; \
	fi

	@echo "Compiling 32RTOSKRNL.BIN (32-bit kernel)..."
	$(ASSEMBLER) -f bin -D__PROTECTED_MODE__ $(SOURCE_KERNEL_DIR)/RTOSKRNL.asm -o $(OUTPUT_KERNEL_DIR)/32RTOSKRNL.BIN
	@echo "32RTOSKRNL.BIN compiled successfully."

	@echo "All kernel components compiled successfully."

# ATOS shell (16-bit)
shell:
	@echo "Compiling ATOS shell (16-bit)..."
	mkdir -p $(OUTPUT_DIR)/SHELL
	$(ASSEMBLER) -f bin -D__PROTECTED_MODE__ $(SOURCE_DIR)/SHELL/SHELL.asm -o $(OUTPUT_DIR)/SHELL/ATSH.BIN
	@echo "ATOS shell compiled successfully."

# Compiler (16-bit)
compiler:
	@echo "Compiling ATLC compiler (16-bit)..."
	mkdir -p $(OUTPUT_DIR)/ATLC
	$(ASSEMBLER) -f bin -D__PROTECTED_MODE__ $(SOURCE_DIR)/COMPILER/COMPILER.asm -o $(OUTPUT_DIR)/ATLC/ATLC.BIN
	@echo "ATLC compiler compiled successfully."



# Build the ISO
iso: bootloader kernel shell compiler
	@echo "Creating ISO directory structure..."
	mkdir -p $(INPUT_ISO_DIR)/INNER/INNER2
	mkdir -p $(INPUT_ISO_DIR_SYSTEM)
	mkdir -p $(INPUT_ISO_DIR_USER)
	mkdir -p $(INPUT_ISO_DIR_PROGRAMS)
	cp -f $(OUTPUT_BOOTLOADER_DIR)/BOOTLOADER.BIN $(INPUT_ISO_DIR)/BOOTLOADER.BIN
	cp -f $(SOURCE_DIR)/INSIDE_1.txt $(INPUT_ISO_DIR)/INNER/INSIDE_1.txt
	cp -f $(SOURCE_DIR)/BASE.txt $(INPUT_ISO_DIR)/INNER/INNER2/INSIDE_1.txt
	cp -f $(SOURCE_DIR)/BASE.txt $(INPUT_ISO_DIR)/BASE.txt
	cp -f $(OUTPUT_KERNEL_DIR)/KERNEL.BIN $(INPUT_ISO_DIR)/KERNEL.BIN
	cp -f $(OUTPUT_KERNEL_DIR)/KRNL.BIN $(INPUT_ISO_DIR)/KRNL.BIN
# 	NOTE: Add any additional files into $(INPUT_ISO_DIR_*)
	cp -f $(OUTPUT_KERNEL_DIR)/32RTOSKRNL.BIN $(INPUT_ISO_DIR_SYSTEM)/32RTOSKRNL.BIN
	
	# TODO: Copy SOURCE\PROGRAMS into $(INPUT_ISO_DIR_PROGRAMS)
	

	@echo "Building ISO..."
	mkdir -p $(OUTPUT_ISO_DIR)
	genisoimage -o $(OUTPUT_ISO_DIR)/$(ISO_NAME) -r -J -b BOOTLOADER.BIN -no-emul-boot $(INPUT_ISO_DIR)
	@echo "ISO created at $(OUTPUT_ISO_DIR)/$(ISO_NAME)"

# Debugging target (running with GDB for troubleshooting)
debug:
	@echo "Debugging ISO..."
	qemu-system-i386 -boot d -cdrom $(OUTPUT_ISO_DIR)/$(ISO_NAME) -m 512 -S -gdb tcp::1234 -d int,cpu -D qemu.log
	@echo "Connected GDB server, use 'gdb' to attach."

# Run the ISO in QEMU
run:
	@echo "Creating hard disk image..."
	mkdir -p $(HARD_DISK_DIR)
	qemu-img create -f raw $(HARD_DISK_DIR)/$(HARD_DISK_IMG) $(HARD_DISK_SIZE)
	@echo "Running ISO..."
	qemu-system-i386 \
		-boot d \
		-cdrom $(OUTPUT_ISO_DIR)/$(ISO_NAME) \
		-drive file=$(HARD_DISK_DIR)/$(HARD_DISK_IMG),format=raw,if=ide,index=0,media=disk \
		-m 512



# Bootloader build rule
$(OUTPUT_BOOTLOADER_DIR)/BOOTLOADER.BIN: $(SOURCE_BOOTLOADER_DIR)/BOOTLOADER.asm
	@echo "Compiling bootloader..."
	mkdir -p $(OUTPUT_BOOTLOADER_DIR)
	$(ASSEMBLER) -f bin -o $@ $<
	@echo "Bootloader compiled successfully."

# Clean up build artifacts
clean:
	@echo "Cleaning output directories..."
	rm -rf $(OUTPUT_DIR) $(INPUT_ISO_DIR)

# Help target to display usage information
help:
	@echo "Makefile usage:"
	@echo "  make kernel       - Compile the kernel"
	@echo "  make bootloader   - Compile the bootloader"
	@echo "  make iso          - Build the ISO (default)"
	@echo "  make run          - Run the ISO"
	@echo "  make debug        - Debug the ISO with QEMU and GDB"
	@echo "  make clean        - Clean all output files"
	@echo "  make compiler     - Compiles ATLC compiler"