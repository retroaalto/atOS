# atOS-RT - atOS Revised Technology

A custom 32-bit operating system written entirely in assembly language, designed without security constraints to allow deep low-level exploration and experimentation.

---

## Table of Contents

* [Overview](#overview)
* [Features](#features)
* [Running atOS-RT](#running-atos-rt)
* [Development](#development)
  * [Dependencies](#dependencies)
  * [Building](#building)
  * [Debugging](#debugging)
* [License](#license)

---

## Overview

**atOS-RT** is a 32-bit operating system built completely in **assembly language** from the ground up. This open-source project is crafted for enthusiasts and developers who want to dive deep into low-level system programming without the typical security restrictions of modern OSes. With atOS-RT, you have complete freedom to explore and modify every aspect of the system.

---

## Features

* **32-bit Architecture**: Targeted specifically for 32-bit x86 processors.
* **Pure Assembly**: Written entirely in assembly, providing unmatched control and insight into hardware interaction.
* **Custom Programming Language**: Includes a unique integrated language for OS-level application development.
* **No Security Constraints**: Designed for ultimate flexibility, allowing unrestricted operations—ideal for experimental and educational purposes.
* **Open Source**: Released under the MIT License, encouraging developers to explore, contribute, and learn how operating systems function at a low level.

---

## Running atOS-RT

Run the provided ISO image in a virtual machine environment. QEMU is recommended for simplicity, though other VMs may work.

### System Requirements

|     | Recommended | Minimum |
| --- | ----------- | ------- |
| RAM | 512 MiB     | 128 MiB |
| CPU | 1 Core      | 1 Core  |

### Installing QEMU

On Debian/Ubuntu-based systems:

```bash
sudo apt install qemu-system-x86
```

### Running the ISO

Navigate to the directory containing the ISO and run:

```bash
qemu-system-i386 -boot d -cdrom ./atOS-RT.iso -m 512
```

This boots the OS with 512MB of RAM, the recommended setting.

---

## Development

Interested in contributing or exploring atOS-RT’s internals? Here's how to get started.

### Dependencies

Install the essential tools:

```bash
sudo apt install qemu-system-x86 nasm make gcc genisoimage
```

| Tool            | Purpose                        |
| --------------- | ------------------------------ |
| qemu-system-x86 | Running the ISO                |
| make            | Build automation               |
| nasm            | Assembler                      |
| gcc             | Toolchain for utility programs |
| genisoimage     | ISO file generation            |

For advanced debugging (not recommended due to the OS's raw binary nature):

```bash
sudo apt install gdb bochs bochs-x
```

| Tool    | Purpose                |
| ------- | ---------------------- |
| gdb     | Debugger               |
| bochs   | x86 emulator/debugger  |
| bochs-x | GUI frontend for bochs |

### Building

The project uses `make` with convenient targets.

* **List all commands:**

  ```bash
  make help
  ```

* **Build and run the ISO:**

  ```bash
  make iso run
  ```

### Debugging

You can debug atOS-RT using GDB with QEMU:

1. **Build and start in debug mode:**

   ```bash
   make iso debug
   ```

2. **Launch GDB with config:**

   ```bash
   gdb -x ./.gdbinit
   ```

---

## License

This project is licensed under the **MIT License**. See the `LICENSE` file for full details.

---
