## Developing Your Own Programs

You can develop custom programs for **atOS-RT** either directly inside the OS or externally using NASM. This section focuses on external development.

---

### Developing Outside atOS-RT (Using NASM)

To include your NASM-based program in the ISO image, follow these steps:

1. Place your source file (e.g., `source/programs/path/to/program.asm`) inside the `SOURCE/PROGRAMS/` directory.
2. Add the full path to your program's `.asm` file in the `SOURCE/USER_PROGRAMS` list.
   This list controls which programs are compiled and included in the ISO during the build process.

This process ensures your custom program is assembled and embedded into the final ISO image.

---

### Helpful Include Files

The following `.inc` files contain useful definitions, constants, and macros for working with atOS-RT features such as system calls and graphics. Each file is documented in a corresponding `README.md` located in its directory.

| Path                                                 | Description                                                                   |
| ---------------------------------------------------- | ----------------------------------------------------------------------------- |
| `SOURCE/KERNEL/32RTOSKRNL/DRIVERS/VIDEO/DEFINES.inc` | Contains video driver definitions, including color constants and video modes. |
| `SOURCE/KERNEL/32RTOSKRNL/SYSCALL/SYSCALL_MIN.inc`   | Provides system call constants and interface definitions for user programs.   |

> Refer to the `README.md` in each file's directory for usage examples and additional context.
