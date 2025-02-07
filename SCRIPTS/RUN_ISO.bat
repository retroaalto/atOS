:: RUN_ISO.bat
:: Boots and runs the ISO in DISM
:: Usage: RUN_ISO.bat
:: Requires: WSL installed and in PATH with qemu x86_64 installed

@echo off
setlocal EnableDelayedExpansion
cd /d %~dp0
cd ..
call .\SCRIPTS\GLOBALS.bat

:: Ensure the ISO exists
if not exist "%OUTPUT_ISO_DIR%\%ISO_NAME%" (
    echo ERROR: %ISO_NAME% not found. Please compile the ISO first.
    set /P "ANSWER=Would you like to compile the ISO now? (Y/N) "
    if /I "%ANSWER%"=="Y" (
        call .\BUILD\MAKE_ISO.bat
        goto :RUN_ISO
    )
    endlocal
    exit /b 1
)

:RUN_ISO
:: Run the ISO
echo Running %ISO_NAME% with %VIRTUAL_MACHINE%...

call %WSL_CMD% run %VIRTUAL_MACHINE% %VIRTUAL_MACHINE_ARGS% 

endlocal DisableDelayedExpansion