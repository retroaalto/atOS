#include <PROGRAMS/SHELL/SHELL.h>
#include <STD/STRING.h>
#include <STD/TYPEDEF.h>
#include <STD/ASM.h>
#include <CPU/SYSCALL/SYSCALL.h>
#include <DRIVERS/PS2/KEYBOARD.h>
#include <PROGRAMS/SHELL/VOUTPUT.h>

U0 SHELL_LOOP(U0);

U0 _start(U0) {
    INIT_SHELL_VOUTPUT();
    CLS();
    SHELL_LOOP();
}

U0 SHELL_LOOP(U0) {
    PUTS("atOS-RT Shell v0.1\nType 'help' for a list of commands.\n> ");
    while(1) {
        KEYPRESS *kp = get_current_keypress();
        if(kp) {
            // U8 *chars = KEYPRESS_TO_CHARS(kp);
            if(kp->pressed) {
                PUTS('b');
            }
            free_keypress(kp);
        }
    }
}