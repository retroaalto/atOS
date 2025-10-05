#include <PROGRAMS/SHELL/SHELL.h>
#include <STD/STRING.h>
#include <STD/TYPEDEF.h>
#include <VIDEO/VBE.h>
// #include <DRIVERS/VIDEO/VOUTPUT.h>
// #include <DRIVERS/PS2/KEYBOARD.h>

U0 SHELL_LOOP(U0);

U0 _start(U0) {
    SHELL_LOOP();
}

U0 SHELL_LOOP(U0) {
    VBE_CLEAR_SCREEN(VBE_WHITE);
    VBE_DRAW_STRING(500, 20, "Hello from atOShell!", VBE_WHITE, VBE_BLUE);
    VBE_UPDATE_VRAM();
    while(1) {
    }
}