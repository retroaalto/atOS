# `SOURCE/` Directory Overview

This directory contains all core components and user-defined extensions that make up the atOS operating system. Below is a breakdown of each subdirectory and its purpose:

```
SOURCE/
├── BASE.txt            # Test file
├── BOOTLOADER/         # 1st stage bootloader
├── COMPILER/           # Custom compiler and language tooling sources
├── INSIDE_1.txt        # Test file
├── KERNEL/             # 32-bit kernel source code (protected mode) and 2nd stage bootloader
├── FS/                 # File system. File reading and writing
├── PROGRAMS/           # User programs to be included in the final ISO
├── README.md           # This file - provides an overview of the source layout
├── SHELL/              # atOS shell source code
├── STD/                # Standard library functions or reusable code
└── USER_PROGRAMS       # File list of programs to be compiled and included. See DOCS\DEVELOPING.md
```

* For more details on each component, see the README files located within each respective folder.

