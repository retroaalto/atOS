Kernel stages and files

- `KERNEL_ENTRY.asm` — 16-bit stage-2 loader. Max binary size: 4095 bytes.
  - Uses BIOS to enumerate memory (E820), optional VESA/VBE setup, prepares GDT/IDT,
    switches to 32-bit protected mode, locates and loads `KRNL.BIN;1`, then jumps to it.
  - Includes from `SOURCE/KERNEL/16-BIT-BIOS/KERNEL_ENTRY_DATA.inc` and `SOURCE/KERNEL/16-BIT-BIOS/BIOS.inc`.

- `KERNEL.c` — 32-bit kernel entry (built as `KRNL.BIN`).
  - Minimal protected-mode entry that re-initializes GDT/IDT/IRQs, verifies VESA/VBE,
    and loads the main kernel binary from the ISO using ATAPI reads to keep size small.
  - Looks for `ATOS/32RTOSKR.BIN` (ISO9660 8.3 form of `32RTOSKRNL.BIN`) and jumps to it at `MEM_RTOSKRNL_BASE`.

- `RTOSKRNL.c` — main 32-bit RTOS kernel (built as `32RTOSKRNL.BIN`).
  - Initializes subsystems: ATAPI, E820, pageframe, kheap, paging, PS/2 keyboard, PCI,
    ATA PIO, optional RTL8139, filesystem (FAT), then launches the shell and enters the kernel loop.

Directory layout

- `SOURCE/KERNEL/16-BIT-BIOS` — 16-bit BIOS helpers used only by `KERNEL_ENTRY.asm`.
- `SOURCE/KERNEL/32RTOSKRNL` — protected-mode kernel subsystems (CPU, MEMORY, FS, DRIVERS, RTOSKRNL, etc.).

Binary naming (as used on the ISO)

- `BOOTLOADER.BIN` — El Torito stage-1 (from `SOURCE/BOOTLOADER`).
- `KERNEL.BIN` — stage-2 real-mode/pm switcher (`KERNEL_ENTRY.asm`).
- `KRNL.BIN` — 32-bit entry (`KERNEL.c`).
- `ATOS/32RTOSKRNL.BIN` — main kernel image (`RTOSKRNL.c`). Note: `KRNL.BIN` searches for `ATOS/32RTOSKR.BIN` to match ISO9660 8.3 filenames.

Build and size constraints

- `KERNEL.BIN` must be ≤ 4095 bytes (enforced in the `Makefile`).
- Binaries and ISO layout are produced via `make iso`; see the top-level `Makefile` for exact link scripts (`kernel.ld`, `rtoskernel.ld`).

Legacy notes (updated)

- Older references to `KERNEL.asm`/`RTOSKRNL.asm` have been replaced by `KERNEL.c` and `RTOSKRNL.c` respectively.
