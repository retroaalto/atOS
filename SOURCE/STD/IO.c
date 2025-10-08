#include <STD/IO.h>
#include <STD/PROC_COM.h>
#include <CPU/SYSCALL/SYSCALL.h>
#include <DRIVERS/PS2/KEYBOARD.h> // For definitions
#include <STD/MEM.h>

void putc(U8 c);
void puts(const U8 *str);
void cls(void);

// // PS2 Keyboard functions
// void reset_keyboard(void) {
//     SYSCALL(SYSCALL_PS2_KEYBOARD_RESET, 0, 0, 0, 0, 0);
// }
// void get_last_keypress(KEYPRESS *kp) {
//     KEYPRESS *k = (KEYPRESS *)SYSCALL(SYSCALL_GET_LAST_KEY_PRESSED, 0, 0, 0, 0, 0);
//     if (!k) {
//         kp->keycode = 0;
//         kp->pressed = FALSE;
//         return;
//     }
//     MEMCPY(kp, k, sizeof(KEYPRESS));
//     Free(k);
// }
// KEYPRESS *get_current_keypress(void) {
//     return (KEYPRESS *)SYSCALL(SYSCALL_GET_CURRENT_KEY_PRESSED, 0, 0, 0, 0, 0);
// }
// void get_modifiers(MODIFIERS *mod) {
//     MODIFIERS *m = (MODIFIERS *)SYSCALL(SYSCALL_GET_KEYBOARD_MODIFIERS, 0, 0, 0, 0, 0);
//     if (!m) {
//         MEMZERO(mod, sizeof(MODIFIERS));
//         return;
//     }
//     MEMCPY(mod, m, sizeof(MODIFIERS));
//     Free(m);
// }
// void free_keypress(KEYPRESS *kp) {
//     if (!kp) return;
//     Free(kp);
// }
U8 keypress_to_char(U32 kcode) {
    U8 *chars = (U8 *)SYSCALL(SYSCALL_KEYPRESS_TO_CHARS, kcode, 0, 0, 0, 0);
    if (!chars) return 0;
    U8 c = chars[0];
    Free(chars);
    return c;
}