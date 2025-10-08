/**
 * PROC_COM.h
 *
 * Process communication utilities for programs and processes.
 *
 * NOT usable in kernel code.
 * 
 * Search for PROC_COM for process communication utilities.
 * Search for SHELLCOM for shell communication utilities.
 * Search for KRNLCOM for kernel communication utilities.
 */

#ifndef PROC_COM_H
#define PROC_COM_H

#include <STD/TYPEDEF.h>
#include <PROC/PROC.h>
#include <DRIVERS/PS2/KEYBOARD.h> // for KEYPRESS and MODIFIERS structs

/**
 * PROC_COM
 */
U32 PROC_GETPID(U0); // Get your own PID
U8 *PROC_GETNAME(U0); // Get your own name
U32 PROC_GETPPID(U0); // Get your parent's PID, -1 if none
U8 *PROC_GETPARENTNAME(U0); // Get your parent's name, NULL if no name

U32 PROC_GETPID_BY_NAME(U8 *name); // Get PID of process by name, returns -1 if not found

TCB *GET_CURRENT_TCB(void); // Get your own TCB, NULL on error
TCB *GET_MASTER_TCB(void); // Get the master TCB, should never be NULL
TCB *GET_TCB_BY_PID(U32 pid); // Get TCB of process by PID, NULL if not found. Free with FREE_TCB
TCB *GET_PARENT_TCB(void); // Get your parent's TCB, NULL if no
void FREE_TCB(TCB *tcb); // Free a TCB received via GET_TCB_BY_PID

#define CREATE_PROC_MSG(receiver, msg_type, data_ptr, msg_ptr, signal_val) \
    (PROC_MESSAGE){ \
        .sender_pid = PROC_GETPID(), \
        .receiver_pid = receiver, \
        .type = msg_type, \
        .data_provided = (data_ptr != NULL), \
        .data = data_ptr, \
        .msg_provided = (msg_ptr != NULL), \
        .signal = signal_val, \
        .timestamp = 0, /* to be filled by kernel */ \
        .read = FALSE, \
    }

/**
 * KRNLCOM
 */

// Send a message to another process or the kernel, Create struct with CREATE_PROC_MSG
VOID SEND_MESSAGE(PROC_MESSAGE *msg);

// Get amount of messages in your message queue
U32 MESSAGE_AMOUNT();

// Get the next message from your message queue, or NULL if none
PROC_MESSAGE *GET_MESSAGE();

// Free a message received via GET_MESSAGE
VOID FREE_MESSAGE(PROC_MESSAGE *msg);

/**
 * SHELLCOM
 */
typedef enum {
    SHELL_CMD_NONE = 0,
    SHELL_CMD_CLEAR_SCREEN = 1,
    SHELL_CMD_SET_TITLE = 2,
    SHELL_CMD_SET_FG = 3,
    SHELL_CMD_SET_BG = 4,
    SHELL_CMD_RESET_COLORS = 5,
    SHELL_CMD_SET_CURSOR_POS = 6,
    SHELL_CMD_GET_CURSOR_POS = 7,
    SHELL_CMD_GET_SCREEN_SIZE = 8,
    SHELL_CMD_ENABLE_CURSOR = 9,
    SHELL_CMD_DISABLE_CURSOR = 10,
    SHELL_CMD_PUTS = 11,
    SHELL_CMD_PUTC = 12,
} SHELL_COMMAND_TYPE;
#endif //PROC_COM_H