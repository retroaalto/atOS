import sys, os

path_to_header      = "./source/main.h"
header_search_for   = "#define VERSION         "

path_to_nsi         = "./release_building/installer.nsi"
nsi_search_for      = "!define VERSION "

path_to_bat         ="./release_building/win.bat"
bat_search_for      ="set \"_VERSION_="

if len(sys.argv) < 2:
    print("Usage: updateversion.py <version>")
    sys.exit(1)

# get version
version = sys.argv[1]
version2 = version
version = '"' + version + '"'

for path, search_for in [
                        # (path_to_header, header_search_for), 
                        (path_to_nsi, nsi_search_for),
                        (path_to_bat, bat_search_for)
                        ]:
    with open(path, "r") as f:
        lines = f.readlines()

    for i, line in enumerate(lines):
        if line.startswith(search_for):
            if(line.startswith(bat_search_for)):
                lines[i] = bat_search_for + version2 + "\"\n"
            else:
                lines[i] = search_for + version + "\n"
            break

    with open(path, "w") as f:
        f.writelines(lines)