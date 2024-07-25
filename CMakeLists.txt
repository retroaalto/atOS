cmake_minimum_required(VERSION 3.8)
project(atOS-RT LANGUAGES C ASM_NASM)

if(USEDEBUG STREQUAL "1")
  set(DEBUG 1)
endif()
message(STATUS "Debug status: ${DEBUG}")
message(STATUS "Status: ${CMAKE_SOURCE_DIR}")

file(GLOB C_SOURCES "./source/kernel_1.c" "./source/kernel.h")

set(KERNEL_BINARY ${CMAKE_BINARY_DIR}/kernel.bin)

add_custom_command(
  OUTPUT ${KERNEL_BINARY}
  COMMAND ${CMAKE_C_COMPILER} -ffreestanding -fno-builtin -nostdlib -nostartfiles -T ${CMAKE_SOURCE_DIR}/linker.ld -o ${KERNEL_BINARY} ${C_SOURCES}
  DEPENDS ${C_SOURCES}
  COMMENT "Building the kernel as a raw binary..."
)

set(BOOTLOADER_BINARY ${CMAKE_BINARY_DIR}/ISO_FILE/BOOTLOADER.BIN)
add_custom_command(
  OUTPUT ${BOOTLOADER_BINARY}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/ISO_FILE
  COMMAND nasm -f bin ${CMAKE_SOURCE_DIR}/source/bootloader.asm -o ${BOOTLOADER_BINARY}
  DEPENDS ${CMAKE_SOURCE_DIR}/source/bootloader.asm
  COMMENT "Assembling the bootloader..."
)

add_custom_target(create_iso
  ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/ISO_FILE
  COMMAND ${CMAKE_COMMAND} -E copy ${KERNEL_BINARY} ${CMAKE_BINARY_DIR}/ISO_FILE/kernel.bin
  COMMAND ${CMAKE_COMMAND} -E copy ${BOOTLOADER_BINARY} ${BOOTLOADER_BINARY}
  COMMAND genisoimage -o ${CMAKE_BINARY_DIR}/ATOS.ISO -b BOOTLOADER.BIN -input-charset utf-8 -no-emul-boot -boot-load-size 4 -boot-info-table ${CMAKE_BINARY_DIR}/ISO_FILE
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/ISO_FILE
  DEPENDS ${KERNEL_BINARY} ${BOOTLOADER_BINARY}
  COMMENT "Creating the ISO image..."
)

