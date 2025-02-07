:: RUN_ISO.bat
:: Boots and runs the ISO in the virtual machine

@echo off
setlocal EnableDelayedExpansion
cd /d %~dp0
cd ..
call .\SCRIPTS\GLOBALS.bat

if "%1"=="/?" (
    echo Usage: RUN_ISO.bat [/?] [NOTE]
    echo   Runs the ISO in the virtual machine.
    echo   If the ISO does not exist, it will be compiled first.
    echo.
    echo   /?
    echo     Displays this help message.
    echo.
    echo   NOTE
    echo     Any additional arguments provided will be passed for the MAKE_ISO.bat script.
    endlocal
    exit /b 0
)

if "%1" neq "" (
    ECHO Compiling ISO with additional arguments: %*
    call .\BUILD\MAKE_ISO.bat %*
    echo %ERRORLEVEL%
    if not exist "%OUTPUT_ISO_DIR%\%ISO_NAME%" (
        ECHO Compilation unsuccesfull. Exiting...
        endlocal
        exit /b 1
    )
)

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