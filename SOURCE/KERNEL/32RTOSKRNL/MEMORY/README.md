# Memory Map: x86 Protected Mode (512 MiB)

 - Recommended memory size: 512 MiB
 - Minimum: 128 Mib
 - Memory over ~112 MiB (`0x074FFFFF`) is for user space

```
| Memory size: 512 MiB
| Address range: 0x00000000 – 0x1FFFFFFF
```

| Start Address | End Address  | Label               | Size      | Notes                                                    |
| ------------- | ------------ | ------------------- | --------- | -------------------------------------------------------- |
| `0x00000000`  | `0x000003FF` | Real Mode IVT       | 1 KiB     | Interrupt Vector Table (used by BIOS)                    |
| `0x00000400`  | `0x000004FF` | BIOS Data Area      | 256 bytes | BIOS info like serial ports, disks, etc.                 |
| `0x00000500`  | `0x00000FFF` | Reserved (BIOS)     | \~2.5 KiB | BIOS scratchpad area                                     |
| `0x00001000`  | `0x00001FFF` | Bootloader          | 4 KiB     | Real mode 1st and 2nd stage bootloaders                  |
| `0x00002000`  | `0x00007FFF` | KRNL                | 24 KiB    | 32-bit kernel entry point                                |
| `0x00008000`  | `0x00008FFF` | E820 Table          | 4 KiB     | BIOS memory map                                          |
| `0x00009000`  | `0x00009100` | VBE Mode Info       | 256 bytes | VESA BIOS Extensions                                     |
| `0x00009100`  | `0x00009FFF` | VBE Controller Info | \~2 KiB   | Optional controller block                                |
| `0x000A0000`  | `0x000BFFFF` | Legacy Video Memory | 128 KiB   | VGA framebuffer (Mode 13h etc.)                          |
| `0x000C0000`  | `0x000FFFFF` | BIOS ROM + Stack    | \~256 KiB | BIOS area + protected mode stack                         |
| `0x00100000`  | `0x001FFFFF` | DATA                | 1 MiB     | Early kernel data, init heap                             |
| `0x00200000`  | `0x003FFFFF` | RTOSKRNL            | 2 MiB     | 32-bit RTOS Kernel binary                                |
| `0x00400000`  | `0x005FFFFF` | Kernel Heap         | 2 MiB     | Kernel dynamic allocations                               |
| `0x00600000`  | `0x00DFFFFF` | Program/Tmp         | 8 MiB     | Temporary programs and data                              |
| `0x00E00000`  | `0x00EFFFFF` | Paging Structures   | 1 MiB     | Page directory + tables                                  |
| `0x00F00000`  | `0x00FFFFFF` | Framebuffer         | 1 MiB     | VESA framebuffer (e.g., 1024x768x32)                     |
| `0x01000000`  | `0x010FFFFF` | ACPI/APIC           | 1 MiB     | ACPI (RSDP/XSDT), local APIC structures                  |
| `0x01100000`  | `0x074FFFFF` | Reserved            | 100 MiB   | Reserved for MMIO and future expansion                   |
| `0x07500000`  | `0x1FFFFFFF` | User Space          | \~395 MiB | Space for user applications, memory mappings, future use |
