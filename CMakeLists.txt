cmake_minimum_required(VERSION 3.8)
project(atOS-RT LANGUAGES C ASM_NASM)

if(USEDEBUG STREQUAL "1")
  set(DEBUG 1)
endif()
message(STATUS "Debug status: ${DEBUG}")
message(STATUS "Status: ${CMAKE_SOURCE_DIR}")

# Define the source files for the kernel
file(GLOB KERNEL_C_SOURCES "./source/kernel_1.c" "./source/kernel.h")
