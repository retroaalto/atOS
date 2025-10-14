#ifndef SHELL_H
#define SHELL_H
#include <STD/TYPEDEF.h>
#include <PROGRAMS/SHELL/VOUTPUT.h>
#include <DRIVERS/PS2/KEYBOARD.h>
#include <FAT/FAT.h>

typedef enum {
    STATE_CMD_INTERFACE, // A focused PROC is running 
    STATE_EDIT_LINE, // Editing current command line
} SHELL_STATES;

typedef struct {
    U8 *PATH;
} ENV_VARS;

typedef struct {
    U32 focused_pid; // PID of the currently focused process.
    U32 previously_focused_pid;
    OutputHandle cursor;
    SHELL_STATES state;
    U8 path[FAT_MAX_PATH];
    ENV_VARS VARS;
} SHELL_INSTANCE;

SHELL_INSTANCE *GET_SHNDL(VOID);
U0 SWITCH_PROC_MODE(VOID);


U0 SWITCH_CMD_INTERFACE_MODE(VOID);
VOID HANDLE_KB_CMDI(KEYPRESS *kp, MODIFIERS *mod);
VOID CMD_INTERFACE_LOOP();
U0 EDIT_LINE_LOOP();
U0 EDIT_LINE_MSG_LOOP();
VOID HANDLE_KB_EDIT_LINE(KEYPRESS *kp, MODIFIERS *mod);

#endif // SHELL_H