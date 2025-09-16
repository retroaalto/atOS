## Developing Your Own Programs

You can develop custom programs for **atOS** either directly inside the OS or externally using NASM or C. This section focuses on external development.

---

### Developing Outside atOS (Using C)



### Developing Outside atOS (Using NASM)

To include your NASM-based program in the ISO image, follow these steps:

1. Place your source file (e.g., `source/programs/path/to/program.asm`) inside the `SOURCE/PROGRAMS/` directory.
2. Add the full path to your program's `.asm` file in the `SOURCE/USER_PROGRAMS` list.
   This list controls which programs are compiled and included in the ISO during the build process.

This process ensures your custom program is assembled and embedded into the final ISO image.

---

### Helpful Include Files

The following `.inc` files contain useful definitions, constants, and macros for working with atOS features such as system calls and graphics. Each file is documented in a corresponding `README.md` located in its directory.

| Path                                                 | Description                                                                   |
| ---------------------------------------------------- | ----------------------------------------------------------------------------- |
| `SOURCE/KERNEL/32RTOSKRNL/DRIVERS/VIDEO/DEFINES.inc` | Contains video driver definitions, including color constants and video modes. |
| `SOURCE/KERNEL/32RTOSKRNL/SYSCALL/SYSCALL_MIN.inc`   | Provides system call constants and interface definitions for user programs.   |

> Refer to the `README.md` in each file's directory for usage examples and additional context.

> Take a look also inside `SOURCE\STD\README.md`!

---

### Testing the Keyboard Driver

1. Build and launch the OS with `make run` (QEMU is required).
2. Once the splash animation completes, type on the host keyboard. Characters should appear on the black console strip starting at `y = 60`.
3. Use `Enter` to move to the next line and `Backspace` to erase the most recent character. Typing past the right edge wraps to the next line.
4. If keys do not render, confirm that interrupts are enabled (`sti` in `kernel_after_gdt`) and that `IRQ_CLEAR_MASK(IRQ_KEYBOARD)` is invoked during boot via `KBD_INIT`.

---

### Using `kdprint`

`kdprint("message")` writes the provided string to the host terminal via the COM1 serial port. The default `make run` command already wires QEMU with `-serial stdio`, so you will see the output in the terminal that launched QEMU. Use standard `\n` line endings (carriage returns are injected automatically).
