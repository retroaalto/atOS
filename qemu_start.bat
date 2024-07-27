@echo off
set "QEMUPATH=C:\Program Files\qemu"
set "QEMU_VM=qemu-system-x86_64.exe"
set "ISOFILE=ATOS.ISO"

if not exist "%QEMUPATH%\%QEMU_VM%" (
    echo QEMU executable not found at %QEMUPATH%\%QEMU_VM%
    exit /b 1
)

if not exist "%ISOFILE%" (
    echo ISO file %ISOFILE% not found
    exit /b 1
)
"%QEMUPATH%\%QEMU_VM%" -cdrom %ISOFILE%
