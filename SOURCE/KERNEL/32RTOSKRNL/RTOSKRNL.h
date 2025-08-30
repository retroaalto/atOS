#ifndef RTOSKRNL_H
#define RTOSKRNL_H
#ifndef RTOS_KERNEL
#define RTOS_KERNEL
#endif // RTOS_KERNEL
#include <STD/ASM.h>
#include <STD/ATOSMINDEF.h>
#include <STD/STRING.h>
#include <STD/MEM.h>

#include <DRIVERS/VIDEO/VBE.h>
#include <DRIVERS/VIDEO/VESA.h>
#include <DRIVERS/VIDEO/VOUTPUT.h>
#include <DRIVERS/PS2/KEYBOARD.h>

#include <RTOSKRNL/RTOSKRNL_INTERNAL.h>

#include <MEMORY/E820/E820.h>

#include <CPU/PIC.h>
#include <CPU/INTERRUPTS.h>
#include <CPU/GDT/GDT.h>
#include <CPU/IDT/IDT.h>
#include <CPU/ISR/ISR.h>
#include <CPU/IRQ/IRQ.h>
#include <DRIVERS/PIT/PIT.h>

__attribute__((noreturn))
U0 rtos_kernel(U0);

__attribute__((noreturn, section(".text")))
void _start(U0);

#endif // RTOSKRNL_H