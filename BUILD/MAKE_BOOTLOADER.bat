:: MAKE_BOOTLOADER.bat
:: Compile the bootloader
:: Usage: MAKE_BOOTLOADER.bat
:: Requires: NASM installed and in PATH
:: Output: OUTPUT\BOOTLOADER\bootloader.bin

@echo off
setlocal

cd /d %~dp0
cd ..
call .\SCRIPTS\GLOBALS.bat

:: Ensure NASM is installed
where %ASSEMBLER% >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo ERROR: NASM is not installed or not in PATH.
    endlocal
    exit /b 1
)
ECHO NASM found: %ASSEMBLER%

:: Ensure OUTPUT\BOOTLOADER directory exists
if not exist "%OUTPUT_BOOTLOADER_DIR%" mkdir "%OUTPUT_BOOTLOADER_DIR%"
ECHO Output Directory: %OUTPUT_BOOTLOADER_DIR%

:: Compile bootloader
call %ASSEMBLER% %ASSEMBLER_FLAGS% "%OUTPUT_BOOTLOADER_DIR%\BOOTLOADER.bin" "%SOURCE_BOOTLOADER_DIR%\BOOTLOADER.asm"
if %ERRORLEVEL% neq 0 (
    echo ERROR: Failed to compile bootloader.
    endlocal
    exit /b 1
)
ECHO Bootloader compiled successfully.
endlocal