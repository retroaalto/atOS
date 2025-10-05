| Start (inclusive) | End (exclusive) | Label                                | Size        |
| ----------------- | --------------- | ------------------------------------ | ----------- |
| `0x00000000`      | `0x00001000`    | Low Memory Reserved (IVT+BDA)        | 4 KiB       |
| `0x00001000`      | `0x00002000`    | Bootloader (stage1/loader area)      | 4 KiB       |
| `0x00002000`      | `0x00004000`    | Second stage / loader temp buffers   | 12 KiB      |
| `0x00006000`      | `0x00006384`    | E820 copy area                       | 4 KiB       |
| `0x00006400`      | `0x00007000`    | VESA/VBE structures (reserve)        | 4 KiB       |
| `0x00007000`      | `0x0000C000`    | Kernel entry / small gap page        | ~17 KiB     |
| `0x0009FC00`      | `0x00100000`    | Reserved memory                      | ~0.4 MiB    |
| `0x000E0000`      | `0x00100000`    | BIOS Reserved (option ROMs, ACPI)    | 128 KiB     |
| `0x00100000`      | `0x00550000`    | Main Kernel (image)                  | ~4.33 MiB   |
| `0x00550000`      | `0x00F33000`    | Kernel Heap / Kernel-managed RAM     | ~5 MiB     |
| `0x00F33000`      | `0x00F34000`    | Guard page below kernel stack        | 4 KiB       |
| `0x00F34000`      | `0x00F44000`    | Kernel stack                         | 64 KiB      |
| `0x00F44000`      | `0x00F45000`    | Guard page above kernel stack        | 4 KiB       |
| `0x00F45000`      | `0x013004F0`    | Framebuffer (VBE LFB)                | ~2.74 MiB   |
| `0x01344000`      | `0x10000000`    | Reserved (MMIO)                      | 16 MiB      |
| `0x10000000`      | `0x20000000`    | User Space area                      | ... MiB     |
| `0xFD000000`      | `0xFD180000`    | VBE framebuffer                      | ~1.5 MiB    |
