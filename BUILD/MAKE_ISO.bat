:: MAKE_ISO.bat
:: Build an ISO image from the contents of the ISO directory.
:: Usage: MAKE_ISO.bat [options]
:: For help, use MAKE_ISO.bat /?
:: Outputs to root\%OUTPUT_ISO_DIR% -> See .\SCRIPTS\GLOBALS.bat for more information

@echo off
setlocal EnableDelayedExpansion

:: Set working directory
cd /d %~dp0
cd ..

call .\SCRIPTS\GLOBALS.bat

:: Ensure OUTPUT\ISO directory exists
IF NOT EXIST "%OUTPUT_ISO_DIR%" mkdir "%OUTPUT_ISO_DIR%"
ECHO ISO Name: %ISO_NAME%
ECHO IMG Name: %IMG_NAME%
ECHO ISO Output Dir: %OUTPUT_ISO_DIR%

:: Clean up any existing ISO files
IF EXIST "%OUTPUT_ISO_DIR%\%ISO_NAME%" del "%OUTPUT_ISO_DIR%\%ISO_NAME%"

:: Parse command-line arguments
SET "COMPILE_BOOTLOADER=0"
SET "COMPILE_KERNEL=0"
SET "COMPILE_ALL=0"
SET "SKIP_COMPILATION=0"
SET "RUN_ISO=0"
SET "HELP_MESSAGE=0"

IF "%~1"=="/?" GOTO :SHOW_HELP

FOR %%A IN (%*) DO (
    IF /I "%%A"=="/M:B" SET "COMPILE_BOOTLOADER=1"
    IF /I "%%A"=="/M:K" SET "COMPILE_KERNEL=1"
    IF /I "%%A"=="/M:A" SET "COMPILE_ALL=1"
    IF /I "%%A"=="/M:N" SET "SKIP_COMPILATION=1"
    IF /I "%%A"=="/R" SET "RUN_ISO=1"
    IF /I "%%A"=="/?" SET "HELP_MESSAGE=1"
    IF /I "%%A"=="/H" SET "HELP_MESSAGE=1"

    :: Handle combined modes like /M:BKAN
    IF /I "%%A"=="B" SET "COMPILE_BOOTLOADER=1"
    IF /I "%%A"=="K" SET "COMPILE_KERNEL=1"
    IF /I "%%A"=="A" SET "COMPILE_ALL=1"
    IF /I "%%A"=="N" SET "SKIP_COMPILATION=1"
)

:: Display help message
IF "%HELP_MESSAGE%"=="1" (
:SHOW_HELP
    ECHO Usage: MAKE_ISO.bat ^[/M:^[B^]^[K^]^[A^]^[B^]^] ^[/R^] ^[/? ^| /H^]
    ECHO   /M: Optional. Compile mode. Any combination of:
    ECHO       mode: Optional. The mode to use when creating the ISO image. Any combination of:
    ECHO             B - Compile bootloader
    ECHO             K - Compile kernel
    ECHO             A - Compile all files
    ECHO             N - Compile no files
    ECHO             Default: A. /M:A is thosen if no mode or flag is specified.
    ECHO       Example: /M:BK  ^(Compiles Bootloader and Kernel^)
    ECHO.
    ECHO   /R: Optional. Run the ISO after compilation.
    ECHO.
    ECHO   /^{H^|?^}: Optional. Display this help message.
    endlocal
    exit /b 0
)

:: If no mode is specified, use default
IF "%COMPILE_BOOTLOADER%%COMPILE_KERNEL%%COMPILE_ALL%%SKIP_COMPILATION%"=="0000" SET "COMPILE_ALL=1"

ECHO Compile Bootloader: %COMPILE_BOOTLOADER%
ECHO Compile Kernel: %COMPILE_KERNEL%
ECHO Compile All: %COMPILE_ALL%
ECHO Skip Compilation: %SKIP_COMPILATION%
ECHO Run ISO: %RUN_ISO%

:: If 'A' is selected, override and compile everything
IF "%COMPILE_ALL%"=="1" (
    SET "COMPILE_BOOTLOADER=1"
    SET "COMPILE_KERNEL=1"
)

:: If 'N' is selected, skip compilation entirely
IF "%SKIP_COMPILATION%"=="1" (
    SET "COMPILE_BOOTLOADER=0"
    SET "COMPILE_KERNEL=0"
    SET "COMPILE_ALL=0"
    echo Skipping compilation...
) ELSE (
    :: Compile based on selected options
    IF "%COMPILE_BOOTLOADER%"=="1" (
        echo Compiling bootloader...
        call .\BUILD\MAKE_BOOTLOADER.bat
    )

    IF "%COMPILE_KERNEL%"=="1" (
        echo Compiling kernel...
        call .\BUILD\MAKE_KERNEL.bat
    )
)

IF NOT EXIST "%INPUT_ISO_DIR%" mkdir "%INPUT_ISO_DIR%"
ECHO ISO Input Dir: %INPUT_ISO_DIR%

ECHO Copying files to ISO directory...
COPY /Y %SOURCE_DIR%\BASE.txt %INPUT_ISO_DIR%\BASE.txt
COPY /Y/B %OUTPUT_KERNEL_DIR%\KERNEL.BIN %INPUT_ISO_DIR%\KERNEL.BIN
COPY /Y/B %OUTPUT_BOOTLOADER_DIR%\BOOTLOADER.BIN %INPUT_ISO_DIR%\BOOTLOADER.BIN

:: Build ISO
echo Creating ISO...
set "ISO_ROOT=%INPUT_ISO_DIR%"
SET "ISO_ROOT=%ISO_ROOT:\=/%"

set "ISO_OUTPUT=%OUTPUT_ISO_DIR%\%ISO_NAME%"
IF EXIST "%ISO_OUTPUT%" del "%ISO_OUTPUT%"
set "ISO_OUTPUT=%ISO_OUTPUT:\=/%"
call %WSL_CMD% run genisoimage -o "%ISO_OUTPUT%" -r -J -b BOOTLOADER.BIN -no-emul-boot %ISO_ROOT% 
IF %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to create ISO image.
    endlocal
    exit /b 1
)


echo ISO image created successfully.

:: Run the ISO if requested
IF "%RUN_ISO%"=="1" (
    ECHO Running ISO...
    call .\SCRIPTS\RUN_ISO.bat
)

endlocal
exit /b 0
