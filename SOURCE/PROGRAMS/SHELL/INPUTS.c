#include <PROGRAMS/SHELL/SHELL.h>
#include <STD/STRING.h>
#include <STD/TYPEDEF.h>
#include <STD/IO.h>
#include <STD/MEM.h>

VOID HANDLE_KB(KEYPRESS *kp, MODIFIERS *mod) {
    if(!kp->pressed) return; // We only care about key presses, not releases
    
    switch (kp->keycode)
    {
    case KEY_ENTER:
        HANDLE_KEY_ENTER();
        PUT_SHELL_START();
        break;
    case KEY_BACKSPACE:
        HANDLE_BACKSPACE();
        break;
    case KEY_DELETE:
        HANDLE_KEY_DELETE();
        break;
    case KEY_ARROW_DOWN:
        HANDLE_KEY_ARROW_DOWN();
        break;
    case KEY_ARROW_UP:
        HANDLE_KEY_ARROW_UP();
        break;
    case KEY_ARROW_LEFT:
        HANDLE_KEY_ARROW_LEFT();
        break;
    case KEY_ARROW_RIGHT:
        HANDLE_KEY_ARROW_RIGHT();
        break;
    case KEY_INSERT:
        TOGGLE_INSERT_MODE();
        break;
    case KEY_C:
        if (mod->ctrl) {
            HANDLE_CTRL_C();
            break;
        }
    default:
        U8 c = keypress_to_char(kp->keycode);
        if (c) PUTC(c);
        break;
    }

}