# Makefile for building FAT and ISO9660 tools

.PHONY: all clean iso fat help

# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra

# Directories
FAT_DIR = FAT
ISO_DIR = ISO9660
OUT_DIR = ../OUTPUT/ISO
ISOFILE = atOS.iso
TOOL_ISO = ISO9660.out
TOOL_FAT = FAT16.out

# Targets
all: iso fat

# ISO9660 Tool
iso: $(ISO_DIR)/$(TOOL_ISO)
	@echo "Running ISO9660 tool on ISO image..."
	cp -f $(OUT_DIR)/$(ISOFILE) ./$(ISOFILE)
	./$(TOOL_ISO) $(ISOFILE) BASE.txt
	./$(TOOL_ISO) $(ISOFILE) "INNER/INSIDE_1.txt"
	./$(TOOL_ISO) $(ISOFILE) "INNER/INNER2/INSIDE_1.txt"

# FAT Tool (currently just a placeholder)
fat: $(FAT_DIR)/$(TOOL_FAT)
	@echo "Running FAT16 tool..."
	# Add logic here to run FAT16 if needed
	# ./$(TOOL_FAT) [args]

$(ISO_DIR)/$(TOOL_ISO): $(ISO_DIR)/ISO9660.c
	@echo "Building $(TOOL_ISO)..."
	$(CC) $(CFLAGS) -o $(TOOL_ISO) $(ISO_DIR)/ISO9660.c

$(FAT_DIR)/$(TOOL_FAT): $(FAT_DIR)/FAT16.c
	@echo "Building $(TOOL_FAT)..."
	$(CC) $(CFLAGS) -o $(TOOL_FAT) $(FAT_DIR)/FAT16.c

clean:
	rm -f $(ISO_DIR)/$(TOOL_ISO) $(FAT_DIR)/$(TOOL_FAT) *.iso *.obj *.exe

help:
	@echo "Available targets:"
	@echo "  all   - Build both ISO9660 and FAT16 tools"
	@echo "  iso   - Build and run ISO9660 tool"
	@echo "  fat   - Build and run FAT16 tool (placeholder)"
	@echo "  clean - Remove generated files"
