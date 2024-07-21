Build and run

    Windows, visual studio:
    C++ CMake tools for windows
    C++ v14.xx build tools for (processor)
    MSVC build tools

    Linux:
    cmake, ninja and somekind of C/C++ compiler

    NOTE: See .\<win|linux>\<batch|shell>\globals.<bat|sh>. Enter your paths there, or the executable programs if they are found in the path

    initialize new git repository:
    .\start.<bat|sh>

    build project:
    .\<win|linux>\build.<bat|sh>

    run project:
    .\<win|linux>\run.<bat|sh> [args...]

    build and run the project:
    .\build_n_run.<bat|sh> [args...]

    clear build:
    .\<win|linux>\<batch|shell>\del.<bat|sh>


Releasing:

    In .\release_building, run win.bat and then linux.sh
    You may have to modify the order yourself or the release build process

    Change project names in .nsi and check the whole .AppDir, it is named as "C_CPP_BASE"