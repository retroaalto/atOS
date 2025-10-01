#ifndef PIT_H
#define PIT_H
#ifdef __RTOS__
#include "../../../STD/TYPEDEF.h"
#include <PROC/PROC.h> // for TrapFrame struct

#define PIT_VECTOR 0x20 // IRQ0


U0 PIT_INIT(U0);
U32 *PIT_GET_TICKS_PTR();
U32 *PIT_GET_HZ_PTR();
void pit_set_frequency(U32 freq);
U0 PIT_WAIT_MS(U32 ms);

void isr_pit(void);

#endif // __RTOS__

#endif // PIT_H