CUR_DIR       := $(shell pwd)
ProgramName   ?= TEST1
Sources       ?= $(CUR_DIR)/TEST1.c \
				 $(CUR_DIR)/../../KERNEL/32RTOSKRNL/DRIVERS/VIDEO/VBE.c \
				 $(CUR_DIR)/../../KERNEL/32RTOSKRNL/DRIVERS/VIDEO/VESA.c \
                 $(CUR_DIR)/TEST1_2.c \

#                  $(CUR_DIR)/../../STD/STRING.c \
#                  $(CUR_DIR)/../../STD/MATH.c \

SOURCE_KERNEL_DIR ?= $(CUR_DIR)/../../KERNEL
SOURCE_DIR    ?= $(CUR_DIR)/SOURCE
OUTPUT_DIR    ?= $(CUR_DIR)/../../../OUTPUT/PROGRAMS
CComp         ?= gcc
CompArgs      ?= -DPROCBIN_VADDR=0x10000000 -D__USER__ -D__PROCESS__ -D__UP__ -Wno-comment -Wno-comments -m32 -ffreestanding -fno-pic -fno-pie -nostdlib -O0 -Wall -Wextra -fno-stack-protector -fno-builtin -fno-inline
Includes      ?= \
	-I$(CUR_DIR)/ \
	-I$(CUR_DIR)/../../ \
	-I$(SOURCE_KERNEL_DIR)/32RTOSKRNL/DRIVERS/ \
	-I$(SOURCE_KERNEL_DIR)/32RTOSKRNL/ \
	-I$(SOURCE_KERNEL_DIR)/32RTOSKRNL/CPU/ \
	-I$(SOURCE_KERNEL_DIR)/32RTOSKRNL/MEMORY/ \
	-I$(SOURCE_KERNEL_DIR)/32RTOSKRNL/FS/ \
	-I$(SOURCE_KERNEL_DIR)/32RTOSKRNL/RTOSKRNL/

# Map sources to objects in OUTPUT_DIR
OBJS := $(patsubst %.c,$(OUTPUT_DIR)/%.o,$(notdir $(Sources)))

.PHONY: all clean

all: $(OUTPUT_DIR) $(OUTPUT_DIR)/$(ProgramName).BIN

# Create output directory if missing
$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

# Compile individual .c files into objects
$(OUTPUT_DIR)/%.o: $(CUR_DIR)/%.c | $(OUTPUT_DIR)
	$(CComp) $(CompArgs) $(Includes) -c $< -o $@

# Link the program
$(OUTPUT_DIR)/$(ProgramName).BIN: $(OBJS)
	$(CComp) -m32 -nostdlib -ffreestanding \
		-Wl,-T,$(CUR_DIR)/../USER_PROGRAMS.ld,-e,_start,--oformat=binary \
		-o $@ $^

	@echo "Program $(ProgramName).BIN built successfully."

clean:
	rm -f $(OUTPUT_DIR)/*.BIN
	rm -f $(OUTPUT_DIR)/*.o
