| Start Address | End Address  | Label                              | Size      | Notes|
| ------------- | ------------ | ---------------------------------- | --------- | ----------------------------- |
| `0x00000000`  | `0x00000FFF` | Low Memory Reserved                | 4 KiB     | Old IVT/BDA/Bios scratchpad. Safe to overwrite in protected mode. |
| `0x00001000`  | `0x00001FFF` | Bootloader                         | 4 KiB     | Still resides here temporarily.|
| `0x00008000`  | `0x00008001` | Drive letter                 | 1 B     | Drive letter.|
| `0x00008002`  | `0x000093FF` | Free / Early Data                  | 5 KiB     | Temporary usage.|
| `0x00009400` | `0x0000A3FF` | Stack               |    4 KiB | Early kernel stack                                     |
| `0x0000A400` | `0x0000FFFF` | MMIO / Temp Buffers | 23.6 KiB | Optional memory-mapped IO, temp buffers                |
| `0x00018000` | `0x00019FFF` | Free        |    8 KiB | Free |
| `0x00020000` | `0x00024000` | Kernel entry      | 16 KiB    | Kernel entry point + 4kb safe gap |
| `0x00428000` | `0x00467FFF` | Safety Gap        | 256 KB    | Reserved gap for safety        |
| `0x00468000` | `0x00667FFF` | Kernel Heap       | 2 MiB     | RTOS kernel heap               |
| `0x00844000` | `0x00DFFFFF` | Program / Temp    | 8 MiB     | User programs / temp storage   |
| `0x00E44000` | `0x00EFFFFF` | Paging Structures | 1 MiB     | Page directory + tables        |
| `0x00F44000` | `0x012004EF` | Framebuffer       | \~3 MiB   | VBE framebuffer 1024×768×32bpp |
| `0x01245000` | `0x012FFFFF` | ACPI / APIC       | 1 MiB     | ACPI tables / local APIC       |
| `0x01344000` | `0x076FFFFF` | Reserved          | 100 MiB   | MMIO / future expansion        |
| `0x07744000` | `0x1FFFFFFF` | User Space        | \~395 MiB | User applications              |

