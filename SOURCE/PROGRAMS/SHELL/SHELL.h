#ifndef SHELL_H
#define SHELL_H
#include <STD/TYPEDEF.h>
#include <PROGRAMS/SHELL/VOUTPUT.h>

typedef struct {
    
    U32 focused_pid; // PID of the currently focused process. This tells kernel which
} SHELL_INSTANCE;

VOID SHELL_START(U0);

#endif // SHELL_H