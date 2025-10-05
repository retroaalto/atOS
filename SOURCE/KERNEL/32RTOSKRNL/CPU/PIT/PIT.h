#ifndef PIT_H
#define PIT_H
#ifdef __RTOS__
#include "../../../STD/TYPEDEF.h"
#include <PROC/PROC.h> // for TrapFrame struct

#define PIT_VECTOR 0x20 // IRQ0
#define PIT_TICKS_HZ 100 // default 100 Hz
#define PIT_TICK_MS (1000 / PIT_TICKS_HZ) // in milliseconds

U0 PIT_INIT(U0);
U32 *PIT_GET_TICKS_PTR();
U32 *PIT_GET_HZ_PTR();
void pit_set_frequency(U32 freq);
U0 PIT_WAIT_MS(U32 ms);

void set_next_task_esp_val(U32 esp);
void set_next_task_cr3_val(U32 cr3);
U32 get_current_task_esp(void);
void set_next_task_pid(U32 pid);
void set_next_task_num_switches(U32 num_switches);

void update_next_cr3();

void isr_pit(void);

#endif // __RTOS__

#endif // PIT_H