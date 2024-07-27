cmake_minimum_required(VERSION 3.8)
project(atOS-RT LANGUAGES C ASM_NASM)

if(USEDEBUG STREQUAL "1")
  set(DEBUG 1)
endif()
message(STATUS "Debug status: ${DEBUG}")
message(STATUS "Status: ${CMAKE_SOURCE_DIR}")

set(BUILD_DIR ${CMAKE_SOURCE_DIR}/build)
set(ISO_DIR ${CMAKE_SOURCE_DIR}/iso)
set(FLOPPY_IMG ${CMAKE_SOURCE_DIR}/floppy.img)
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

# Assemble bootloader_1
add_custom_command(
  OUTPUT ${BUILD_DIR}/bootloader_1.bin
  COMMAND nasm -f bin -o ${BUILD_DIR}/bootloader_1.bin ${CMAKE_SOURCE_DIR}/source/bootloader_1.asm
  DEPENDS ${CMAKE_SOURCE_DIR}/source/bootloader_1.asm
  COMMENT "Assembling bootloader_1"
)

# Assemble kernel
add_custom_command(
  OUTPUT ${BUILD_DIR}/kernel.bin
  COMMAND nasm -f bin -o ${BUILD_DIR}/kernel.bin ${CMAKE_SOURCE_DIR}/source/kernel.asm
  DEPENDS ${CMAKE_SOURCE_DIR}/source/kernel.asm
  COMMENT "Assembling kernel"
)

add_custom_target(assemble ALL
  DEPENDS ${BUILD_DIR}/bootloader_1.bin ${BUILD_DIR}/kernel.bin
)

# Create floppy image
add_custom_command(
  OUTPUT ${FLOPPY_IMG}
  COMMAND dd if=/dev/zero of=${FLOPPY_IMG} bs=1024 count=1440
  COMMAND dd if=${BUILD_DIR}/bootloader_1.bin of=${FLOPPY_IMG} seek=0 count=1 conv=notrunc
  DEPENDS assemble
  COMMENT "Creating 1.44mb floppy and adding bootloader_1"
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
add_custom_target(run
  COMMAND qemu-system-x86_64 -cdrom ${ISO_IMG} -boot d
  DEPENDS iso
  COMMENT "Running QEMU"
)
