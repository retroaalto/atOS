@echo off
setlocal
cd /d %~dp0
del *.iso > nul 2>&1
del *.obj > nul 2>&1
del *.exe > nul 2>&1
call d:\build_tools_x64\devcmd.bat > nul 2>&1
call gcc ISO9660.c -I. -Wall -Wextra -o ISO9660
del *.obj > nul 2>&1
Copy /Y/B ..\..\OUTPUT\ISO\atOS.iso .\atOS.iso
call ISO9660 atOS.iso BASE.txt
echo.
call ISO9660 atOS.iso "INNER/INSIDE_1.txt"
echo.
call ISO9660 atOS.iso "INNER/INNER2/INSIDE_1.txt"
endlocal