#include <SHELL/SHELL.h>

U0 SHELL_LOOP(U0) {
    // Placeholder for shell loop function
    U32 strpos = 0;
    U32 row = 20;
    U8 buf[256] = {0};
    for (;;) {
        KEYPRESS kp = GET_CURRENT_KEY_PRESSED();
        U8 *keychar = KEYPRESS_TO_CHARS(&kp);
        if(kp.pressed == FALSE) continue; // Key released
        if(kp.keycode == KEY_BACKSPACE) {
            if(strpos > 0) {
                buf[--strpos] = '\0';
            }
            VBE_DRAW_RECTANGLE_FILLED(0, row, SCREEN_WIDTH, VBE_CHAR_HEIGHT + 2, VBE_BLACK);
            VBE_DRAW_STRING(0, row, buf, VBE_AQUA, VBE_BLACK);
            VBE_UPDATE_VRAM();
            continue;
        }
        if(!keychar || !keychar[0]) continue; // No key pressed or key released
        if(kp.keycode == KEY_UNKNOWN) continue; // Unknown key

        if(strpos >= sizeof(buf) - 1) {
            strpos = 0;
        }
        STRNCONCAT(buf, strpos++, keychar, sizeof(buf) - 1);


        VBE_DRAW_STRING(0, row, buf, VBE_AQUA, VBE_BLACK);
        VBE_UPDATE_VRAM();
    }
}

U0 SHELL_START(U0) {
    SHELL_LOOP();
}