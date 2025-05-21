# Makefile

# Include global configuration variables from a separate file if needed
include config.mk

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
INPUT_ISO_DIR_M ?= $(INPUT_ISO_DIR)/ATOS
ISO_NAME      ?= atOS.iso
IMG_NAME      ?= output.img

.PHONY: all kernel bootloader iso clean run debug help

# Default target is to build the ISO
all: iso

# Kernel build target
kernel: $(OUTPUT_KERNEL_DIR)/KERNEL.BIN $(OUTPUT_KERNEL_DIR)/KRNL.BIN $(OUTPUT_KERNEL_DIR)/32RTOSKRNL.BIN
	@echo "Compiling KERNEL.BIN (real mode second stage)..."
	mkdir -p $(OUTPUT_KERNEL_DIR)
	$(ASSEMBLER) -f bin $(SOURCE_KERNEL_DIR)/KERNEL_ENTRY.asm -o $(OUTPUT_KERNEL_DIR)/KERNEL.BIN
	@echo "KERNEL.BIN compiled successfully."
	@size=$$(stat -c%s "$(OUTPUT_KERNEL_DIR)/KERNEL.BIN"); \
	if [ $$size -gt 4095 ]; then \
		echo "\033[1;33mWARNING: KERNEL.BIN size is $$size bytes, which exceeds 4095 bytes!\033[0m"; \
	fi



	@echo "Compiling 32-bit kernel entry..."
	mkdir -p $(OUTPUT_KERNEL_DIR)
	$(ASSEMBLER) -f bin $(SOURCE_KERNEL_DIR)/KERNEL.asm -o $(OUTPUT_KERNEL_DIR)/KRNL.BIN
	@echo "32-bit kernel entry compiled successfully."

	@echo "Compiling kernel..."
	mkdir -p $(OUTPUT_KERNEL_DIR)
	$(ASSEMBLER) -f bin $(SOURCE_KERNEL_DIR)/RTOSKRNL.asm -o $(OUTPUT_KERNEL_DIR)/32RTOSKRNL.BIN
	@echo "Kernel compiled successfully."

	@echo "All kernel components compiled successfully."


# Bootloader build target
bootloader: $(OUTPUT_BOOTLOADER_DIR)/BOOTLOADER.BIN

# Build the ISO
iso: bootloader kernel
	@echo "Creating ISO directory structure..."
	mkdir -p $(INPUT_ISO_DIR)/INNER/INNER2
	mkdir -p $(INPUT_ISO_DIR_M)
	cp -f $(OUTPUT_BOOTLOADER_DIR)/BOOTLOADER.BIN $(INPUT_ISO_DIR)/BOOTLOADER.BIN
	cp -f $(SOURCE_DIR)/INSIDE_1.txt $(INPUT_ISO_DIR)/INNER/INSIDE_1.txt
	cp -f $(SOURCE_DIR)/BASE.txt $(INPUT_ISO_DIR)/INNER/INNER2/INSIDE_1.txt
	cp -f $(SOURCE_DIR)/BASE.txt $(INPUT_ISO_DIR)/BASE.txt
	cp -f $(OUTPUT_KERNEL_DIR)/KERNEL.BIN $(INPUT_ISO_DIR)/KERNEL.BIN
	cp -f $(OUTPUT_KERNEL_DIR)/KRNL.BIN $(INPUT_ISO_DIR)/KRNL.BIN
# 	NOTE: Add any additional files into $(INPUT_ISO_DIR_M)
	cp -f $(OUTPUT_KERNEL_DIR)/32RTOSKRNL.BIN $(INPUT_ISO_DIR_M)/32RTOSKRNL.BIN
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
	@echo "Running ISO..."
	qemu-system-i386 -boot d -cdrom $(OUTPUT_ISO_DIR)/$(ISO_NAME) -m 512 


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
