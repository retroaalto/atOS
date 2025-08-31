| Start (inclusive) | End (exclusive) | Label  | Size |
| ----------------- | --------------- | -------------------------------------- | --------- |
| `0x00000000`      | `0x00001000`    | Low Memory Reserved (IVT+BDA)          | 4 KiB|
| `0x00001000`      | `0x00002000`    | Bootloader (stage1/loader area)        | 4 KiB|
| `0x00002000`      | `0x00004000`    | Second stage / loader temp buffers     | 12 KiB            |
| `0x00006000`      | `0x00006384`    | E820 copy area    | 4 KiB|
| `0x00006400`      | `0x00007000`    | VESA/VBE structures (reserve)
| `0x00007000`      | `0x0000C000`    | Kernel entry / small gap page         | \~17 KiB|
| `0x0009FC00`      | `0x00100000`    | Reserved memory | \~0.4MB |
| `0x000E0000`      | `0x00100000`    | BIOS Reserved (option ROMs, ACPI area) | 128 KiB           |


| `0x00100000`      | `0x00550000`    | Main Kernel (image)                    | \~4.33 MiB        |
| `0x00550000`      | `0x00F44000`    | Kernel Heap / Kernel-managed RAM       | \~10 MiB (page aligned start)|
| `0x00F44000`      | `0x013004F0`    | Framebuffer (VBE LFB)                  | \~2.74 MiB        |
| `0x01344000`      | `0x02344000`    | Reserved (MMIO)                        | 16 MiB            |
| `0x02344000`      | `0x03D09000`    | User Space area                        | \~27 MiB          |
