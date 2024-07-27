cmake_minimum_required(VERSION 3.8)
project(atOS-RT LANGUAGES C ASM_NASM)

if(USEDEBUG STREQUAL "1")
  set(DEBUG 1)
endif()
message(STATUS "Debug status: ${DEBUG}")
message(STATUS "Status: ${CMAKE_SOURCE_DIR}")

set(BUILD_DIR ${CMAKE_SOURCE_DIR}/build)
set(ISO_DIR ${CMAKE_SOURCE_DIR}/iso)
set(FLOPPY_IMG ${CMAKE_SOURCE_DIR}/atos_rt.img)
set(ISO_IMG ${CMAKE_SOURCE_DIR}/atos_rt.iso)

file(MAKE_DIRECTORY ${BUILD_DIR})
file(MAKE_DIRECTORY ${ISO_DIR})

file(GLOB KERNEL_C_SOURCES "${CMAKE_SOURCE_DIR}/source/kernel_1.c" "${CMAKE_SOURCE_DIR}/source/kernel.h")

# Remove old files
file(GLOB OLD_FILES "${CMAKE_SOURCE_DIR}/*.img" "${CMAKE_SOURCE_DIR}/*.iso" "${CMAKE_SOURCE_DIR}/*.bin")
foreach(FILE ${OLD_FILES})
  file(REMOVE ${FILE})
endforeach()
file(REMOVE_RECURSE ${BUILD_DIR})
file(REMOVE_RECURSE ${ISO_DIR})

# Assemble bootloader_1_fat32
add_custom_command(
  OUTPUT ${BUILD_DIR}/bootloader_1_fat32.bin
  COMMAND nasm -f bin -o ${BUILD_DIR}/bootloader_1_fat32.bin ${CMAKE_SOURCE_DIR}/source/bootloader_1_fat32.asm
  DEPENDS ${CMAKE_SOURCE_DIR}/source/bootloader_1_fat32.asm
  COMMENT "Assembling bootloader_1_fat32"
)

# Assemble kernel
add_custom_command(
  OUTPUT ${BUILD_DIR}/kernel.bin
  COMMAND nasm -f bin -o ${BUILD_DIR}/kernel.bin ${CMAKE_SOURCE_DIR}/source/kernel.asm
  DEPENDS ${CMAKE_SOURCE_DIR}/source/kernel.asm
  COMMENT "Assembling kernel"
)

add_custom_target(assemble ALL
  DEPENDS ${BUILD_DIR}/bootloader_1_fat32.bin ${BUILD_DIR}/kernel.bin
)

# Create floppy image
add_custom_command(
  OUTPUT ${FLOPPY_IMG}
  COMMAND dd if=/dev/zero of=${FLOPPY_IMG} bs=512 count=2880
  COMMAND mkfs.fat -F 32 -n "FAT32   " ${FLOPPY_IMG}
  COMMAND dd if=${BUILD_DIR}/bootloader_1_fat32.bin of=${FLOPPY_IMG} conv=notrunc
  COMMAND mcopy -i ${FLOPPY_IMG} ${BUILD_DIR}/kernel.bin "::/kernel.bin"
  DEPENDS assemble
  COMMENT "Creating 1.44mb floppy and adding bootloader_1_fat32"
)


add_custom_target(floppy ALL
  DEPENDS ${FLOPPY_IMG}
)

# Copy floppy image to ISO directory
add_custom_command(
  OUTPUT ${ISO_DIR}/floppy.img
  COMMAND ${CMAKE_COMMAND} -E copy ${FLOPPY_IMG} ${ISO_DIR}/floppy.img
  DEPENDS floppy
  COMMENT "Copying floppy image to ISO directory"
)

# Create ISO image
add_custom_command(
  OUTPUT ${ISO_IMG}
  COMMAND genisoimage -quiet -V 'ATOS' -input-charset iso8859-1 -o ${ISO_IMG} -b floppy.img -hide floppy.img ${ISO_DIR}
  DEPENDS ${ISO_DIR}/floppy.img
  COMMENT "Creating ISO image"
)

add_custom_target(iso ALL
  DEPENDS ${ISO_IMG}
)

# Run QEMU
add_custom_target(run_cdrom
  COMMAND qemu-system-x86_64 -cdrom ${ISO_IMG} -boot d
  DEPENDS iso
  COMMENT "Running QEMU"
)

add_custom_target(run_floppy
  COMMAND qemu-system-x86_64 -fda ${FLOPPY_IMG} -boot a
  DEPENDS floppy
  COMMENT "Running QEMU"
)


#~~~~~~~~~~~~~~~~
# For tools
#~~~~~~~~~~~~~~~~
add_custom_command(
  OUTPUT ${BUILD_DIR}/tools/fat
  COMMAND mkdir -p ${BUILD_DIR}/tools/
  COMMAND gcc -o ${BUILD_DIR}/tools/fat ${CMAKE_SOURCE_DIR}/tools/fat/fat.c
  DEPENDS ${CMAKE_SOURCE_DIR}/tools/fat/fat.c
  COMMENT "Compiling FAT tool"
)

add_custom_target(build_fat_tool ALL
  DEPENDS ${BUILD_DIR}/tools/fat
)