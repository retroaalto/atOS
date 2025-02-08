:: MAKE_KERNEL.bat
:: Compiles the kernel
:: Usage: MAKE_KERNEL.bat
@echo off

setlocal enabledelayedexpansion

cd /d %~dp0
cd ..

call .\SCRIPTS\GLOBALS.bat
ECHO Starting to compile kernel...

ECHO Source Dir: %SOURCE_DIR%
ECHO Output Dir: %OUTPUT_DIR%
ECHO Output Kernel Dir: %OUTPUT_KERNEL_DIR%

:: Ensure OUTPUT\KERNEL directory exists
IF NOT EXIST "%OUTPUT_KERNEL_DIR%" mkdir "%OUTPUT_KERNEL_DIR%"

:: Clean up any existing kernel files
IF EXIST "%OUTPUT_KERNEL_DIR%\KERNEL.BIN" del "%OUTPUT_KERNEL_DIR%\KERNEL.BIN"

:: Compile the kernel
echo Compiling the kernel...
%ASSEMBLER% -f bin %SOURCE_KERNEL_DIR%\KERNEL_ENTRY.asm -o %OUTPUT_KERNEL_DIR%\KERNEL.BIN

:: Check if the kernel was compiled successfully
IF %ERRORLEVEL% NEQ 0 (
    echo Kernel compilation failed.
    goto :END
)

echo Kernel compiled successfully.

:END
endlocal 