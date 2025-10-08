#include <PROGRAMS/SHELL/SHELL.h>
#include <STD/STRING.h>
#include <STD/TYPEDEF.h>
#include <STD/IO.h>
#include <STD/PROC_COM.h>
#include <STD/ASM.h>
#include <STD/MEM.h>
#include <STD/GRAPHICS.h>
#include <CPU/SYSCALL/SYSCALL.h>
#include <DRIVERS/PS2/KEYBOARD.h>
#include <PROGRAMS/SHELL/VOUTPUT.h>

BOOLEAN draw_access_granted ATTRIB_DATA = FALSE;

U0 SHELL_LOOP(U0);
U0 INITIALIZE_SHELL();

U0 _start(U0) {
    // draw_access_granted = FALSE;
    INITIALIZE_SHELL();
    // INIT_SHELL_VOUTPUT();
    SHELL_LOOP();
}

U0 INITIALIZE_SHELL() {
    PROC_MESSAGE msg = CREATE_PROC_MSG(0, PROC_GET_FRAMEBUFFER, NULL, NULL, 0);
    SEND_MESSAGE(&msg);
    // msg = CREATE_PROC_MSG(0, PROC_GET_KEYBOARD_EVENTS, NULL, NULL, 0);
    // SEND_MESSAGE(&msg);
    msg = CREATE_PROC_MSG(0, PROC_MSG_SET_FOCUS, NULL, NULL, PROC_GETPID());
    SEND_MESSAGE(&msg);
    return;
}
U0 MSG_LOOP(U0) {
    U32 msg_count = MESSAGE_AMOUNT();
    while (msg_count > 0) {
        PROC_MESSAGE *msg = GET_MESSAGE();
        if (!msg) break;
        switch(msg->type) {
            case PROC_KEYBOARD_EVENTS_GRANTED:
                // Keyboard events granted
                // HLT;
                break;
            case PROC_FRAMEBUFFER_GRANTED:
                draw_access_granted = TRUE;
                break;
            case PROC_MSG_KEYBOARD:
                if (msg->data_provided && msg->data) {
                    KEYPRESS *kp = (KEYPRESS *)msg->data;
                    MODIFIERS *mods = (MODIFIERS *)((U8 *)msg->data + sizeof(KEYPRESS));
                    // Handle key press
                    U8 chars = keypress_to_char(kp->keycode);
                    if (chars) {
                        PUTS((char *)chars);
                        Free(chars);
                    }
                }
                break;
        }
    }
}
U0 SHELL_LOOP(U0) {
    OutputHandle out = GetOutputHandle();
    U32 i = 0;
    U32 j = 0;
    // while(!draw_access_granted) {
    //     MSG_LOOP();
    // }
    CLEAR_SCREEN_COLOUR(VBE_RED);
    while(1) {

        // if(i++ % 5 == 0)
            // CLEAR_SCREEN_COLOUR(VBE_RED);
        // else 
        // MSG_LOOP();
        // if(!draw_access_granted) continue;
        
        PUTS("atOShell v0.1\r\nType 'help' for a list of commands.\r\n> ");
        // DRAW_LINE(i, out->SHEIGHT - 1, out->SWIDTH - 1, out->SHEIGHT - 1, VBE_WHITE);
        // i += 10;
        // if (i >= out->SWIDTH) i = 0;
        // CLS();
    }
}