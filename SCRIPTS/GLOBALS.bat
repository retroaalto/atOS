:: GLOBALS.bat
:: Global variables and settings for the build system
:: Usage: call GLOBALS.bat
:: Requires: None
:: NOTE: This script should be called by all other build scripts
::
:: If comment includes _WSL_, the variable is used by the WSL scripts
:: If comment includes _WIN_, the variable is used by the Windows scripts
:: If comment includes _ALL_, the variable is used by both

@echo off

:: %%%%%%%%%%%%%%%%%%%%%%%%%%%
:: General settings
:: %%%%%%%%%%%%%%%%%%%%%%%%%%%

:: ISO image output name. _ALL_
SET "ISO_NAME=atOS.iso"
SET "IMG_NAME=boot.img"




:: %%%%%%%%%%%%%%%%%%%%%%%%%%%
:: WSL settings
:: %%%%%%%%%%%%%%%%%%%%%%%%%%%

:: WSL_CMD: Linux distribution to use. _WSL_
SET "WSL_CMD=debian"




:: %%%%%%%%%%%%%%%%%%%%%%%%%%%
:: Directories
:: %%%%%%%%%%%%%%%%%%%%%%%%%%%

:: SCRIPT_DIR: The directory where the script is located. _ALL_
SET "SCRIPT_DIR=%~dp0"

:: ROOT_DIR: The root directory of the project. _ALL_
SET "ROOT_DIR=%SCRIPT_DIR%.."

:: SOURCE_DIR: The directory where source files are stored. _ALL_
SET "SOURCE_DIR=%ROOT_DIR%\SOURCE"

:: OUTPUT_DIR: The directory where the build output is stored. _ALL_
SET "OUTPUT_DIR=OUTPUT"
SET "OUTPUT_ISO_DIR=%OUTPUT_DIR%\ISO"
SET "OUTPUT_BOOTLOADER_DIR=%OUTPUT_DIR%\BOOTLOADER"
SET "OUTPUT_KERNEL_DIR=%OUTPUT_DIR%\KERNEL"

:: INPUT_DIR: The directory where files are stored for the ISO compilation. _ALL_
SET "INPUT_DIR=INPUT"
set "INPUT_ISO_DIR=%INPUT_DIR%\ISO"

:: SOURCE_DIR: The directory where source files are stored. _ALL_
SET "SOURCE_DIR=SOURCE"
SET "SOURCE_BOOTLOADER_DIR=%SOURCE_DIR%\BOOTLOADER"
SET "SOURCE_KERNEL_DIR=%SOURCE_DIR%\KERNEL"



:: %%%%%%%%%%%%%%%%%%%%%%%%%%%
:: QEMU settings
:: %%%%%%%%%%%%%%%%%%%%%%%%%%%

:: VIRTUAL_MACHINE: The virtual machine to use. x86 or x86_64 is recommended. _WSL_
SET "VIRTUAL_MACHINE=qemu-system-i386"
:: VIRTUAL_MACHINE_ARGS: Arguments to pass to the virtual machine. _WSL_
SET "VM_PATHIFIED=%OUTPUT_ISO_DIR%\%ISO_NAME%"
SET "VM_PATHIFIED=%VM_PATHIFIED:\=/%"
SET "VIRTUAL_MACHINE_ARGS=-boot d -cdrom %VM_PATHIFIED% -m 512 -s




:: %%%%%%%%%%%%%%%%%%%%%%%%%%%
:: Compiler, assembler, and linker settings 
:: %%%%%%%%%%%%%%%%%%%%%%%%%%%

:: NASM: The NASM assembler. _WIN_
SET "ASSEMBLER=nasm"
:: NASM_FLAGS: Flags to pass to NASM. _WIN_
:: -f bin: Output format is binary
:: -o: Output file
:: Sources are placed after the flags
SET "ASSEMBLER_FLAGS=-f bin -o"

:: C_COMPILER: The C compiler. _WIN_
SET "C_COMPILER="
:: C_COMPILER_FLAGS: Flags to pass to the C compiler. _WIN_
SET "C_COMPILER_FLAGS="

:: CXX_COMPILER: The C++ compiler. _WIN_
SET "CXX_COMPILER="
:: CXX_COMPILER_FLAGS: Flags to pass to the C++ compiler. _WIN_
SET "CXX_COMPILER_FLAGS="

:: LINKER: The linker. _WIN_
SET "LINKER="
:: LINKER_FLAGS: Flags to pass to the linker. _WIN_
SET "LINKER_FLAGS="
