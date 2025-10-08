# Paging

This directory contains the implementation of the paging mechanism in atOS-RT. Paging is a memory management scheme that eliminates the need for contiguous allocation of physical memory, thereby reducing fragmentation and allowing for more efficient use of memory.

## Overview

This is a simple 32-bit, single-level paging implementation with basic process isolation.
It provides ring0 paging for atOS-RT.

Kernel is linked to 0x100000 and identity-mapped there.

"User programs" aka processes are linked to 0x10000000 and
    but mapped to anywhere in physical memory

User programs are allocated into a predefined memory region.

This region allows user processes to have their own virtual address space,
    isolated from the kernel and other processes, but still having access to kernel and system memory
