cmake_minimum_required(VERSION 3.8)
project(atOS-RT LANGUAGES C ASM_NASM)

if(USEDEBUG STREQUAL "1")
  set(DEBUG 1)
endif()
message(STATUS "Debug status: ${DEBUG}")
message(STATUS "Status: ${CMAKE_SOURCE_DIR}")

message(STATUS "TEMPORARY SET_FS SET")
set(SET_FS "12") # TEMP

if(SET_FS STREQUAL "32")
  message(FATAL "FAT32 HAS NOT BEEN ADDED AS A PREPROCESSOR TO NASM FILE! EXITING...")
  set(FS "32")
elseif(SET_FS STREQUAL "16")
  set(FS "16")
else()
  set(FS "12")
endif()

message(STATUS "Using FAT${FS}")

set(BUILD_DIR ${CMAKE_SOURCE_DIR}/build)
set(ISO_DIR ${CMAKE_SOURCE_DIR}/iso)
set(DISK_IMG ${CMAKE_SOURCE_DIR}/atos_rt.img)
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

# Assemble bootloader
add_custom_command(
  OUTPUT ${BUILD_DIR}/bootloader_1_fat16-32.bin
  COMMAND nasm -f bin -o ${BUILD_DIR}/bootloader_1_fat16-32.bin ${CMAKE_SOURCE_DIR}/source/bootloader_1_fat16-32.asm
  COMMAND mkdir -p ${ISO_DIR}
  COMMAND cp ${BUILD_DIR}/bootloader_1_fat16-32.bin ${ISO_DIR}
  DEPENDS ${CMAKE_SOURCE_DIR}/source/bootloader_1_fat16-32.asm
  COMMENT "Assembling bootloader_1_fat16-32"
)

# Assemble kernel
add_custom_command(
  OUTPUT ${BUILD_DIR}/kernel.bin
  COMMAND nasm -f bin -o ${BUILD_DIR}/kernel.bin ${CMAKE_SOURCE_DIR}/source/kernel.asm
  COMMAND mkdir -p ${ISO_DIR}
  COMMAND cp ${BUILD_DIR}/kernel.bin ${ISO_DIR}
  DEPENDS ${CMAKE_SOURCE_DIR}/source/kernel.asm
  COMMENT "Assembling kernel"
)

add_custom_target(assemble ALL
  DEPENDS ${BUILD_DIR}/bootloader_1_fat16-32.bin ${BUILD_DIR}/kernel.bin
)

# Create and format the disk image
add_custom_command(
  OUTPUT ${DISK_IMG}
  # if(FS STREQUAL "12")
    COMMAND dd if=/dev/zero of=${DISK_IMG} bs=512 count=8192
  # elseif(FS STREQUAL "16")
    # COMMAND dd if=/dev/zero of=${DISK_IMG} bs=512 count=16384
  # else()
  #   COMMAND dd if=/dev/zero of=${DISK_IMG} bs=512 count=#TODO
  # endif()
  COMMAND mkfs.fat -v -v -F ${FS} ${DISK_IMG}
  COMMAND mcopy -i ${DISK_IMG} ${BUILD_DIR}/bootloader_1_fat16-32.bin ::/bootloader_1_fat16-32.bin
  COMMAND mcopy -i ${DISK_IMG} ${BUILD_DIR}/kernel.bin ::/kernel.bin
  DEPENDS ${BUILD_DIR}/bootloader_1_fat16-32.bin ${BUILD_DIR}/kernel.bin
  COMMENT "Creating and formatting disk image with FAT16 and copying bootloader and kernel"
)

add_custom_target(disk_image ALL
  DEPENDS ${DISK_IMG}
)

# Create ISO image
add_custom_command(
  OUTPUT ${ISO_IMG}
  COMMAND genisoimage -V 'ATOS' -input-charset iso8859-1 -o ${ISO_IMG} -b bootloader_1_fat16-32.bin -no-emul-boot -boot-load-size 4 -boot-info-table ${ISO_DIR}
  DEPENDS ${DISK_IMG}
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

# Compile FAT tool
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
