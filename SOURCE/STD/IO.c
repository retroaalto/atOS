#include <STD/IO.h>
#include <STD/PROC_COM.h>
#include <CPU/SYSCALL/SYSCALL.h>
#include <DRIVERS/PS2/KEYBOARD.h> // For definitions
#include <STD/MEM.h>

void putc(U8 c);
void puts(const U8 *str);
void cls(void);

// PS2 Keyboard functions
void reset_keyboard(void) {
    SYSCALL(SYSCALL_PS2_KEYBOARD_RESET, 0, 0, 0, 0, 0);
}
void get_last_keypress(KEYPRESS *kp) {
    if(!kp) return;
    SYSCALL(SYSCALL_GET_LAST_KEY_PRESSED, (U32)kp, 0, 0, 0, 0);
}
KEYPRESS *get_current_keypress(void) {
    U32 ptr = SYSCALL(SYSCALL_GET_CURRENT_KEY_PRESSED, 0, 0, 0, 0, 0);
    // TODO: ptr is kernel addresses, convert to process address space if necessary
    KEYPRESS *retval = (KEYPRESS *)ptr;
    return retval;
}
void get_modifiers(MODIFIERS *mod) {
    
}
void free_keypress(KEYPRESS *kp) {
    if(!kp) return;
    SYSCALL(SYSCALL_KFREE, (U32)kp, 0, 0, 0, 0);
}