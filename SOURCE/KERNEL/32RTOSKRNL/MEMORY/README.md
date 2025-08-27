| Start Address | End Address  | Label                              | Size      | Notes                                                             |
| ------------- | ------------ | ---------------------------------- | --------- | ----------------------------------------------------------------- |
| `0x00000000`  | `0x00000FFF` | Low Memory Reserved                | 4 KiB     | Old IVT/BDA/Bios scratchpad. Safe to overwrite in protected mode. |
| `0x00001000`  | `0x00001FFF` | Bootloader                         | 4 KiB     | Still resides here temporarily.                                   |
| `0x00002000`  | `0x00007FFF` | Temporary Kernel Stub              | 24 KiB    | Early 32-bit kernel entry, small stub.                            |
| `0x00008000`  | `0x00008FFF` | Free / Early Data                  | 4 KiB     | Temporary usage.                                                  |
| `0x00009000`  | `0x000093FF` | GDT + IDT                          | 1 KiB     | 256 B GDT + 2 KB IDT. Completely in low memory. !NOT USED!                  |
| `0x00009400`  | `0x00009FFF` | Stack                              | 1.5 KiB   | Early kernel stack. Can increase if needed.                       |
| `0x0000A000`  | `0x0000FFFF` | Free / Temp Buffers                | 24 KiB    | Can be used for early allocations.                                |
| `0x00010000`  | `0x0001FFFF` | Optional extra low memory mappings | 64 KiB    | Optional, for MMIO or temporary data.                             |
| `0x00020000`  | `0x005FFFFF` | Kernel                             | 3.5 MiB   | Main 32-bit kernel binary, fully loaded here.                     |
| `0x00600000`  | `0x007FFFFF` | Kernel Heap                        | 2 MiB     | Dynamic allocations.                                              |
| `0x00800000`  | `0x00DFFFFF` | Program / Temp                     | 8 MiB     | User programs / temporary storage.                                |
| `0x00E00000`  | `0x00EFFFFF` | Paging Structures                  | 1 MiB     | Page directory + tables.                                          |
| `0x00F00000`  | `0x012004EF` | Framebuffer                        | \~3 MiB   | VESA framebuffer 1024×768×32bpp.                                  |
| `0x01201000`  | `0x012FFFFF` | ACPI / APIC                        | 1 MiB     | ACPI tables, local APIC.                                          |
| `0x01300000`  | `0x076FFFFF` | Reserved                           | 100 MiB   | MMIO / future expansion.                                          |
| `0x07700000`  | `0x1FFFFFFF` | User Space                         | \~395 MiB | User applications.                                                |
