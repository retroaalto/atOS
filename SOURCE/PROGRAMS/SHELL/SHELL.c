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

static BOOLEAN draw_access_granted ATTRIB_DATA = FALSE;
static BOOLEAN keyboard_access_granted ATTRIB_DATA = FALSE;

U0 SHELL_LOOP(U0);
U0 INITIALIZE_SHELL();

U0 _start(U0) {
    draw_access_granted = FALSE;
    keyboard_access_granted = FALSE;

    INITIALIZE_SHELL();
    INIT_SHELL_VOUTPUT();
    SHELL_LOOP();
}

U0 INITIALIZE_SHELL() {
    PROC_MESSAGE msg;

    msg = CREATE_PROC_MSG(KERNEL_PID, PROC_GET_FRAMEBUFFER, NULL, 0);
    SEND_MESSAGE(&msg); // Will be read first
    
    msg = CREATE_PROC_MSG(KERNEL_PID, PROC_MSG_SET_FOCUS, NULL, PROC_GETPID());
    SEND_MESSAGE(&msg);

    msg = CREATE_PROC_MSG(0, PROC_GET_KEYBOARD_EVENTS, NULL, 0);
    SEND_MESSAGE(&msg);
    
    // STRING panicmsg = MAlloc(64);
    // MEMSET(panicmsg, 'A', 64);
    // msg = CREATE_PROC_MSG(KERNEL_PID, PROC_MSG_PANIC, panicmsg, 0);
    // SEND_MESSAGE(&msg);

}
U0 MSG_LOOP(U0) {
    U32 msg_count = MESSAGE_AMOUNT();
    for(U32 i = 0; i <= msg_count; i++) {
        PROC_MESSAGE *msg = GET_MESSAGE(); // Gets last message. Marked as read
        if (!msg) break;
        switch(msg->type) {
            case PROC_KEYBOARD_EVENTS_GRANTED:
                // Keyboard events granted
                keyboard_access_granted = TRUE;
                break;
            case PROC_FRAMEBUFFER_GRANTED:
                draw_access_granted = TRUE;
                CLEAR_SCREEN_COLOUR(VBE_AQUA);
                break;
            case PROC_MSG_KEYBOARD:
                if (msg->data_provided && msg->data) {
                    KEYPRESS *kp = (KEYPRESS *)msg->data;
                    MODIFIERS *mods = (MODIFIERS *)((U8 *)msg->data + sizeof(KEYPRESS));
                    // Handle key press
                    U8 c = keypress_to_char(kp->keycode);
                    if(kp->pressed && c) PUTC(c);
                }
                break;
        }
        FREE_MESSAGE(msg);
    }
}
U0 SHELL_LOOP(U0) {
    OutputHandle out = GetOutputHandle();
    U32 i = 0;
    U32 j = 0;
    U32 pass = 0;
    
    while(1) {
        MSG_LOOP();
        if(draw_access_granted) break;
    }
    // CLEAR_SCREEN_COLOUR(VBE_RED);
    CLS();

    
    PUTS("atOShell v0.1\r\nType 'help' for a list of commands.\r\n> ");
    while(1) {

        MSG_LOOP();
        // if(!draw_access_granted) continue;
        
        // i += 10;
        // if (i >= out->SWIDTH) i = 0;
        // DRAW_LINE(300, i, j, i, pass++ % 2 == 0 ? VBE_BLUE : VBE_DARK_GREEN);
        // i++;
        // if(i >= 1024) i = 0;
        // j++;
        // if(j >= 768) j = 0;
        // if(pass >= 100) {
        //     pass = 0;
        //     pass = (pass % 2 == 0) ? 1 : -1;
        // }
    }
}