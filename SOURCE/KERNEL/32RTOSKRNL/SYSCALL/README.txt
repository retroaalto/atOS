# System Calls

System calls in **atOS-RT** are invoked using the `int 0x80` software interrupt. This interface allows user programs to safely call kernel routines for various system services.

SYSCALL_MIN.inc is a definition file, designed for non-kernel usage. It has 

---

## Calling Convention

All system calls follow a **stack-based** calling convention:

| Step         | Description                                                               |
| ------------ | ------------------------------------------------------------------------- |
| `push ...`   | Push syscall arguments to the stack in **reverse order** (last arg first) |
| `mov eax, #` | Load the system call number into `eax`                                    |
| `int 0x80`   | Trigger the interrupt to execute the system call                          |
| `eax`        | Return value from the syscall (if any) is placed in `eax`                 |

> **Important:** Arguments must be pushed in reverse order, just like calling a standard C-style function.

---

## System Call Table

The following system calls are currently implemented. This list will grow as more features are added.

| Syscall # | Name              | Purpose                     | Stack Arguments                            | Returns |
| --------- | ----------------- | --------------------------- | ------------------------------------------ | ------- |
| `0`       | `sys_putchar`     | Print a character to screen | `char ch`                                  | —       |
| `1`       | `sys_halt`        | Halt the CPU                | —                                          | —       |
| `2`       | `sys_draw_pixel`  | Draw a pixel on screen      | `int x`, `int y`, `int color (0xAARRGGBB)` | —       |
| `...`     | (future syscalls) | —                           | —                                          | —       |

---

## Example Usage

### Print a character:

```asm
push byte 'A'     ; character to print
mov eax, 0        ; syscall number: sys_putchar
int 0x80          ; make the system call
```

### Draw a red pixel at (100, 50):

```asm
push dword 0x00FF0000  ; color (red, colour macros can be found in SOURCE\KERNEL\DRIVERS\VIDEO\DEFINES.inc)
push dword 50          ; y coordinate
push dword 100         ; x coordinate
mov eax, 2             ; syscall number: sys_draw_pixel
int 0x80               ; make the system call
```
