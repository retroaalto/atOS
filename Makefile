
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
KRNL_INCLUDES ?= \
	-I$(SOURCE_DIR)/ \
	-I$(SOURCE_KERNEL_DIR)/32RTOSKRNL/DRIVERS/VIDEO/ \
	-I$(SOURCE_KERNEL_DIR)/32RTOSKRNL/DRIVERS/DISK/ \
	-I$(SOURCE_KERNEL_DIR)/32RTOSKRNL/CPU/ \
	-I$(SOURCE_KERNEL_DIR)/32RTOSKRNL/CPU/GDT/ \
	-I$(SOURCE_KERNEL_DIR)/32RTOSKRNL/CPU/IDT/ \
	-I$(SOURCE_KERNEL_DIR)/32RTOSKRNL/CPU/ISR/ \
	-I$(SOURCE_KERNEL_DIR)/32RTOSKRNL/CPU/IRQ/ \
	-I$(SOURCE_KERNEL_DIR)/32RTOSKRNL/MEMORY/ \
	-I$(SOURCE_KERNEL_DIR)/32RTOSKRNL/DRIVERS/PIT/ \

RTOSKRNL_INCLUDES ?= \
	-I$(SOURCE_DIR)/ \
	-I$(SOURCE_KERNEL_DIR)/32RTOSKRNL/DRIVERS/ \
	-I$(SOURCE_KERNEL_DIR)/32RTOSKRNL/ \
	-I$(SOURCE_KERNEL_DIR)/32RTOSKRNL/DRIVERS/ \
	-I$(SOURCE_KERNEL_DIR)/32RTOSKRNL/CPU/ \
	-I$(SOURCE_KERNEL_DIR)/32RTOSKRNL/MEMORY/ \
	-I$(SOURCE_KERNEL_DIR)/32RTOSKRNL/DRIVERS/ \
	-I$(SOURCE_KERNEL_DIR)/32RTOSKRNL/FS/ \

CComp         ?= gcc
CompArgs 	  ?= -Wno-comment -Wno-comments -m32 -ffreestanding -fno-pic -fno-pie -nostdlib -O0 -Wall -Wextra -fno-stack-protector -fno-builtin -fno-inline
KRNLCompArgs  ?= $(CompArgs) $(KRNL_INCLUDES) -DKERNEL_ENTRY
RTOSKRNLCompArgs ?= $(CompArgs) $(RTOSKRNL_INCLUDES) -DRTOS_KERNEL -D__32RTOS__ -D__RTOS__

INPUT_ISO_DIR_SYSTEM ?= $(INPUT_ISO_DIR)/ATOS
INPUT_ISO_DIR_USER ?= $(INPUT_ISO_DIR)/USER
INPUT_ISO_DIR_PROGRAMS ?= $(INPUT_ISO_DIR)/PROGRAMS

.PHONY: all kernel bootloader iso clean run help

# Default target
all: iso

# Bootloader build (16-bit raw binary)
$(OUTPUT_BOOTLOADER_DIR)/BOOTLOADER.BIN: $(SOURCE_BOOTLOADER_DIR)/BOOTLOADER.asm
	@echo "Compiling bootloader (16-bit real mode)..."
	mkdir -p $(OUTPUT_BOOTLOADER_DIR)
	$(ASSEMBLER) -f bin -D__REAL_MODE__ -o $@ $<
	@echo "Bootloader compiled successfully."

bootloader: $(OUTPUT_BOOTLOADER_DIR)/BOOTLOADER.BIN

# Kernel build (raw binaries only, no linker script)
# Kernel build target (raw binary, no linker script)
kernel: 
	@echo "Compiling KERNEL.BIN (16-bit real mode second stage)..."
	mkdir -p $(OUTPUT_KERNEL_DIR)
	rm -rf $(OUTPUT_KERNEL_DIR)
	mkdir -p $(OUTPUT_KERNEL_DIR)
	$(ASSEMBLER) -f bin -D__REAL_PROTECTED_MODE__ \
		$(SOURCE_KERNEL_DIR)/KERNEL_ENTRY.asm \
		-o $(OUTPUT_KERNEL_DIR)/KERNEL.BIN
	@echo "KERNEL.BIN compiled successfully."
	@size=$$(stat -c%s "$(OUTPUT_KERNEL_DIR)/KERNEL.BIN"); \
	if [ $$size -gt 4095 ]; then \
		echo "\033[1;33mWARNING: KERNEL.BIN size is $$size bytes, which exceeds 4095 bytes!\033[0m"; \
	fi

	@echo "Compiling KRNL.BIN (32-bit protected mode C kernel)..."
	mkdir -p $(OUTPUT_KERNEL_DIR)
	$(CComp) $(KRNLCompArgs) -c $(SOURCE_KERNEL_DIR)/KERNEL.c -o $(OUTPUT_KERNEL_DIR)/KERNEL.o -m32
	$(CComp) $(KRNLCompArgs) -c $(SOURCE_KERNEL_DIR)/32RTOSKRNL/DRIVERS/VIDEO/VESA.c -o $(OUTPUT_KERNEL_DIR)/VESA.o
	$(CComp) $(KRNLCompArgs) -c $(SOURCE_KERNEL_DIR)/32RTOSKRNL/DRIVERS/VIDEO/VBE.c -o $(OUTPUT_KERNEL_DIR)/VBE.o
	$(CComp) $(KRNLCompArgs) -c $(SOURCE_KERNEL_DIR)/32RTOSKRNL/CPU/GDT/GDT.c -o $(OUTPUT_KERNEL_DIR)/GDT.o
	$(CComp) $(KRNLCompArgs) -c $(SOURCE_KERNEL_DIR)/32RTOSKRNL/CPU/IDT/IDT.c -o $(OUTPUT_KERNEL_DIR)/IDT.o
	$(CComp) $(KRNLCompArgs) -c $(SOURCE_KERNEL_DIR)/32RTOSKRNL/CPU/ISR/ISR.c -o $(OUTPUT_KERNEL_DIR)/ISR.o
	$(CComp) $(KRNLCompArgs) -c $(SOURCE_KERNEL_DIR)/32RTOSKRNL/CPU/IRQ/IRQ.c -o $(OUTPUT_KERNEL_DIR)/IRQ.o
	$(CComp) $(KRNLCompArgs) -c $(SOURCE_KERNEL_DIR)/32RTOSKRNL/CPU/INTERRUPTS.c -o $(OUTPUT_KERNEL_DIR)/INTERRUPTS.o
	$(CComp) $(KRNLCompArgs) -c $(SOURCE_KERNEL_DIR)/32RTOSKRNL/DRIVERS/DISK/ATA_ATAPI.c -o $(OUTPUT_KERNEL_DIR)/ATA_ATAPI.o
	$(CComp) $(KRNLCompArgs) -c $(SOURCE_KERNEL_DIR)/32RTOSKRNL/DRIVERS/PIT/PIT.c -o $(OUTPUT_KERNEL_DIR)/PIT.o
	$(CComp) $(KRNLCompArgs) -c $(SOURCE_KERNEL_DIR)/32RTOSKRNL/CPU/PIC.c -o $(OUTPUT_KERNEL_DIR)/PIC.o

# Link all object files into KRNL.BIN
	
	$(CComp) -m32 -nostdlib -ffreestanding \
		-Wl,-T,$(SOURCE_KERNEL_DIR)/kernel.ld,-e,_start,--oformat=binary \
		-o $(OUTPUT_KERNEL_DIR)/KRNL.BIN \
		$(OUTPUT_KERNEL_DIR)/KERNEL.o \
		$(OUTPUT_KERNEL_DIR)/VESA.o \
		$(OUTPUT_KERNEL_DIR)/VBE.o \
		$(OUTPUT_KERNEL_DIR)/GDT.o \
		$(OUTPUT_KERNEL_DIR)/IDT.o \
		$(OUTPUT_KERNEL_DIR)/ISR.o \
		$(OUTPUT_KERNEL_DIR)/IRQ.o \
		$(OUTPUT_KERNEL_DIR)/INTERRUPTS.o \
		$(OUTPUT_KERNEL_DIR)/ATA_ATAPI.o \
		$(OUTPUT_KERNEL_DIR)/PIT.o \
		$(OUTPUT_KERNEL_DIR)/PIC.o \

	@echo "KRNL.BIN compiled successfully."
	@size=$$(stat -c%s "$(OUTPUT_KERNEL_DIR)/KRNL.BIN"); \
	if [ $$size -gt 16384 ]; then \
		echo "\033[1;33mWARNING: KRNL.BIN size is $$size bytes, which exceeds 16384 bytes!\033[0m"; \
	fi


	# Remove kernel entry object files
	rm -f $(OUTPUT_KERNEL_DIR)/*.o

	@echo "Compiling 32RTOSKRNL.BIN (32-bit asm kernel)..."
	$(CComp) $(RTOSKRNLCompArgs) -c $(SOURCE_KERNEL_DIR)/RTOSKRNL.c -o $(OUTPUT_KERNEL_DIR)/RTOSKRNL.o -m32
	$(CComp) $(RTOSKRNLCompArgs) -c $(SOURCE_KERNEL_DIR)/32RTOSKRNL/DRIVERS/VIDEO/VESA.c -o $(OUTPUT_KERNEL_DIR)/VESA.o
	$(CComp) $(RTOSKRNLCompArgs) -c $(SOURCE_KERNEL_DIR)/32RTOSKRNL/DRIVERS/VIDEO/VBE.c -o $(OUTPUT_KERNEL_DIR)/VBE.o
	$(CComp) $(RTOSKRNLCompArgs) -c $(SOURCE_KERNEL_DIR)/32RTOSKRNL/DRIVERS/VIDEO/VOUTPUT.c -o $(OUTPUT_KERNEL_DIR)/VOUTPUT.o
	$(CComp) $(RTOSKRNLCompArgs) -c $(SOURCE_KERNEL_DIR)/32RTOSKRNL/DRIVERS/PS2/KEYBOARD.c -o $(OUTPUT_KERNEL_DIR)/PS2_KEYBOARD.o
	$(CComp) $(RTOSKRNLCompArgs) -c $(SOURCE_KERNEL_DIR)/32RTOSKRNL/CPU/GDT/GDT.c -o $(OUTPUT_KERNEL_DIR)/GDT.o
	$(CComp) $(RTOSKRNLCompArgs) -c $(SOURCE_KERNEL_DIR)/32RTOSKRNL/CPU/IDT/IDT.c -o $(OUTPUT_KERNEL_DIR)/IDT.o
	$(CComp) $(RTOSKRNLCompArgs) -c $(SOURCE_KERNEL_DIR)/32RTOSKRNL/CPU/ISR/ISR.c -o $(OUTPUT_KERNEL_DIR)/ISR.o
	$(CComp) $(RTOSKRNLCompArgs) -c $(SOURCE_KERNEL_DIR)/32RTOSKRNL/CPU/IRQ/IRQ.c -o $(OUTPUT_KERNEL_DIR)/IRQ.o
	$(CComp) $(RTOSKRNLCompArgs) -c $(SOURCE_KERNEL_DIR)/32RTOSKRNL/CPU/INTERRUPTS.c -o $(OUTPUT_KERNEL_DIR)/INTERRUPTS.o
	$(CComp) $(RTOSKRNLCompArgs) -c $(SOURCE_KERNEL_DIR)/32RTOSKRNL/DRIVERS/DISK/ATA_ATAPI.c -o $(OUTPUT_KERNEL_DIR)/ATA_ATAPI.o
	$(CComp) $(RTOSKRNLCompArgs) -c $(SOURCE_KERNEL_DIR)/32RTOSKRNL/DRIVERS/PIT/PIT.c -o $(OUTPUT_KERNEL_DIR)/PIT.o
	$(CComp) $(RTOSKRNLCompArgs) -c $(SOURCE_KERNEL_DIR)/32RTOSKRNL/CPU/PIC.c -o $(OUTPUT_KERNEL_DIR)/PIC.o
	$(CComp) $(RTOSKRNLCompArgs) -c $(SOURCE_KERNEL_DIR)/32RTOSKRNL/MEMORY/E820.c -o $(OUTPUT_KERNEL_DIR)/E820.o
	$(CComp) $(RTOSKRNLCompArgs) -c $(SOURCE_DIR)/STD/MEM.c -o $(OUTPUT_KERNEL_DIR)/MEM.o
	$(CComp) $(RTOSKRNLCompArgs) -c $(SOURCE_DIR)/STD/STRING.c -o $(OUTPUT_KERNEL_DIR)/STRING.o

	$(CComp) -m32 -nostdlib -ffreestanding \
		-Wl,-T,$(SOURCE_KERNEL_DIR)/rtoskernel.ld,-e,_start,--oformat=binary \
		-o $(OUTPUT_KERNEL_DIR)/32RTOSKRNL.BIN \
		$(OUTPUT_KERNEL_DIR)/RTOSKRNL.o \
		$(OUTPUT_KERNEL_DIR)/VESA.o \
		$(OUTPUT_KERNEL_DIR)/VBE.o \
		$(OUTPUT_KERNEL_DIR)/PS2_KEYBOARD.o \
		$(OUTPUT_KERNEL_DIR)/VOUTPUT.o \
		$(OUTPUT_KERNEL_DIR)/GDT.o \
		$(OUTPUT_KERNEL_DIR)/IDT.o \
		$(OUTPUT_KERNEL_DIR)/ISR.o \
		$(OUTPUT_KERNEL_DIR)/IRQ.o \
		$(OUTPUT_KERNEL_DIR)/INTERRUPTS.o \
		$(OUTPUT_KERNEL_DIR)/PIT.o \
		$(OUTPUT_KERNEL_DIR)/PIC.o \
		$(OUTPUT_KERNEL_DIR)/E820.o \
		$(OUTPUT_KERNEL_DIR)/MEM.o \
		$(OUTPUT_KERNEL_DIR)/STRING.o \

	@echo "32RTOSKRNL.BIN compiled successfully."

	@echo "All kernel components compiled successfully."


# ISO build
iso: bootloader kernel
	@echo "Creating ISO directory structure..."
	mkdir -p $(INPUT_ISO_DIR)/INNER/INNER2
	mkdir -p $(INPUT_ISO_DIR_SYSTEM)
	mkdir -p $(INPUT_ISO_DIR_USER)
	mkdir -p $(INPUT_ISO_DIR_PROGRAMS)
	cp -f $(OUTPUT_BOOTLOADER_DIR)/BOOTLOADER.BIN $(INPUT_ISO_DIR)/BOOTLOADER.BIN
	cp -f $(SOURCE_DIR)/BASE.txt $(INPUT_ISO_DIR)/BASE.txt
	cp -f $(OUTPUT_KERNEL_DIR)/KERNEL.BIN $(INPUT_ISO_DIR)/KERNEL.BIN
	cp -f $(OUTPUT_KERNEL_DIR)/32RTOSKRNL.BIN $(INPUT_ISO_DIR_SYSTEM)/32RTOSKRNL.BIN
	cp -f $(OUTPUT_KERNEL_DIR)/KRNL.BIN $(INPUT_ISO_DIR)/KRNL.BIN

	@echo "Building ISO..."
	mkdir -p $(OUTPUT_ISO_DIR)
	genisoimage -o $(OUTPUT_ISO_DIR)/$(ISO_NAME) -r -J -b BOOTLOADER.BIN -no-emul-boot $(INPUT_ISO_DIR)
	@echo "ISO created at $(OUTPUT_ISO_DIR)/$(ISO_NAME)"

# Run ISO in QEMU
run: 
	@echo "Running ISO in QEMU..."
	qemu-img create -f raw hdd.img 256M
	qemu-system-i386 -vga std \
	-boot d \
	-cdrom $(OUTPUT_ISO_DIR)/$(ISO_NAME) \
	-m 512 \
	-drive file=hdd.img,format=raw,if=ide,index=0,media=disk \

runlh:
	@echo "Running ISO in QEMU..."
	qemu-img create -f raw hdd.img 256M
	qemu-system-i386 -vga std -boot d -cdrom \
	$(OUTPUT_ISO_DIR)/$(ISO_NAME) -m 512 \
	-drive file=hdd.img,format=raw,if=ide,index=0,media=disk \
	-monitor tcp:localhost:4444,server,nowait
	

# Clean
clean:
	@echo "Cleaning output directories..."
	rm -rf $(OUTPUT_DIR) $(INPUT_ISO_DIR)

# Help
help:
	@echo "Makefile usage:"
	@echo "  make kernel     - Compile kernel raw binaries"
	@echo "  make bootloader - Compile bootloader"
	@echo "  make iso        - Build bootable ISO (default)"
	@echo "  make run        - Run ISO in QEMU"
	@echo "  make clean      - Clean build artifacts"
