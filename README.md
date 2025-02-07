# atOS Revised technology

32-bit operating system

## Building & Requirements

### Windows w/ WSL

#### Requirements
 - Any WSL distribution, change it inside .\SCRIPTS\GLOBALS.bat. default: debian. 
 - WSL: genisoimage. For .ISO image creation
 - WSL: qemu-system. For running the .ISO image
 - Windows: NASM, GCC

#### Building

Build parts of the OS or ISO
```sh
:: Build bootloader
.\BUILD\MAKE_BOOTLOADER.bat

:: Build kernel
.\BUILD\MAKE_KERNEL.bat

:: Build ISO. Use /H or /? for help
.\BUILD\MAKE_ISO.bat [/M:[B][K][A][B]] [/R] [/H | /?]
```

Run the ISO
```sh
:: Use /? for help
.\SCRIPTS\RUN_ISO.bat
```