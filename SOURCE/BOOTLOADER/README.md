Bootloader overview

- `SOURCE/BOOTLOADER/BOOTLOADER.asm` is the 1st‑stage (real‑mode) bootloader. It boots from the El Torito no‑emulation image on the ISO, finds `KERNEL.BIN` in the ISO9660 root, loads it to memory, and jumps to it.
- `SOURCE/BOOTLOADER/DISK_VBR.asm` is a minimal VBR stub placeholder used for disk images; on the ISO it’s included as a data file for reference.

What stage‑1 does

- Runs in 16‑bit real mode at `0x7C00` and preserves `DL` (boot drive).
- Uses BIOS INT 13h Extensions (`AH=0x42` with a Disk Address Packet) to read device sectors.
- Treats the boot device as ISO9660 (CD‑ROM) with 2048‑byte sectors.
- Reads the Primary Volume Descriptor at LBA 16 and checks the `CD001` signature.
- Reads the root directory record from the PVD (offset 156) to get its LBA and size.
- Scans the root directory for the exact filename `KERNEL.BIN;1` (ISO9660 versioned name), skipping directory entries.
- When found, loads `KERNEL.BIN` to `KERNEL_LOAD_SEGMENT:KERNEL_LOAD_OFFSET` (currently `0x0000:0x2000`) and far‑jumps there.

Handoff to stage‑2

- Stage‑2 is `SOURCE/KERNEL/KERNEL_ENTRY.asm` (built as `KERNEL.BIN`).
- It remains in 16‑bit to perform BIOS work, enumerates memory (E820), sets up VESA (optional), prepares GDT/IDT, switches to 32‑bit protected mode, locates `KRNL.BIN;1` on the ISO, loads it, and jumps to the 32‑bit C kernel entry.

Building and running

- `make iso` builds the bootloader, stage‑2, and kernel binaries and assembles `OUTPUT/ISO/atOS.iso` with `BOOTLOADER.BIN` as the El Torito boot image.
- `make iso run` launches QEMU with the recommended devices and boots the ISO.

Implementation notes

- Sector size: 2048 bytes (CD‑ROM). LBA reads use INT 13h Extensions via a DAP.
- PVD location: LBA 16. Root directory record is parsed from PVD byte offset 156.
- Directory record fields used: length (byte 0), extent LBA (bytes 2..5 LE), data length (bytes 10..13 LE), flags (byte 25), name length (byte 32), name (byte 33..).
- Filenames must include the ISO9660 version suffix (`;1`). Only the root directory is scanned.
- Minimal console output: prints simple `diskERROR`/`isoERROR` messages via BIOS teletype on failure.

Limitations and TODOs (known from the current code)

- The sector count computation in stage‑1 is currently aligned to 512‑byte sectors; for ISO9660 it should align to 2048 bytes.
- Stage‑1 updates the kernel LBA but still uses the root directory’s stored length for the read; it should use the file record’s extent length for `KERNEL.BIN`.
- Only the first sector of the root directory is scanned; multi‑sector directories and subdirectories are not handled.
- Only standard ISO9660 is supported; no Joliet/Rock Ridge extensions.

Paths in this repository use forward slashes. Example: `SOURCE/KERNEL/KERNEL_ENTRY.asm`.
