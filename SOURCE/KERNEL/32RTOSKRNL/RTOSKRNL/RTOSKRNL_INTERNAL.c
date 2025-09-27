#include <RTOSKRNL/RTOSKRNL_INTERNAL.h>
#include <VIDEO/VBE.h>
#include <STD/ASM.h>

void panic(const U8 *msg) {
    VBE_DRAW_STRING(0, 16, msg, VBE_WHITE, VBE_BLACK);
    VBE_UPDATE_VRAM();
    ASM_VOLATILE("cli; hlt");
}