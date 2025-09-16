#include "./VBE.h"
#include "../../../../STD/MATH.h"

/*
8x8 characters

0b11111110
0b11111110
0b11111110
0b11111110
0b11111110
0b11111110
0b11111110
0b00000000

Right is left empty
Bottom is left empty

Most are borrowed from https://gist.github.com/rothwerx/700f275d078b3483509f
*/
#define INVISIBLE_CHARACTER {0, 0, 0, 0, 0, 0, 0, 0}
#define CHARACTER(...) { __VA_ARGS__ }
VBE_LETTERS_TYPE VBE_LETTERS[VBE_MAX_CHARS][VBE_CHAR_HEIGHT] = {
    CHARACTER(
        0b00110000,
        0b01111000,
        0b01111000,
        0b00110000,
        0b00110000,
        0b00000000,
        0b00110000,
        0b00000000,
    ), // !
    CHARACTER(
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
    ), // TODO: " 
    CHARACTER(
        0b01101100,
        0b01101100,
        0b11111110,
        0b01101100,
        0b11111110,
        0b01101100,
        0b01101100,
        0b00000000,
    ), // #
    CHARACTER(
        0b00110000,
        0b01111100,
        0b11000000,
        0b01111000,
        0b00001100,
        0b11111000,
        0b00110000,
        0b00000000,
    ), // $
    CHARACTER(
        0b00000000,
        0b11000110,
        0b11001100,
        0b00011000,
        0b00110000,
        0b01100110,
        0b11000110,
        0b00000000,
    ), // %
    CHARACTER(
        0b00111000,
        0b01101100,
        0b00111000,
        0b01110110,
        0b11011100,
        0b11001100,
        0b01110110,
        0b00000000,
    ), // &
    CHARACTER(
        0b00111000,
        0b00010000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
    ), // '
    CHARACTER(
        0b00111000,
        0b01100000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01100000,
        0b00111000,
        0b00000000,
    ), // (
    CHARACTER(
        0b01110000,
        0b00011000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00011000,
        0b01110000,
        0b00000000,
    ), // )
    CHARACTER(
        0b00000000,
        0b00101010,
        0b00011100,
        0b00111110,
        0b00011100,
        0b00101010,
        0b00000000,
        0b00000000,
    ), // *
    CHARACTER(
        0b00000000,
        0b00010000,
        0b00010000,
        0b01111100,
        0b00010000,
        0b00010000,
        0b00000000,
        0b00000000,
    ), // +
    CHARACTER(
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01100000,
        0b11000000,
        0b00000000,

    ), // ,
    CHARACTER(
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b11111100,
        0b00000000,
        0b00000000,
        0b00000000,
    ), // -
    CHARACTER(
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b01100000,
        0b01100000,
        0b00000000,

    ), // .
    CHARACTER(
        0b00000110,
        0b00001100,
        0b00011000,
        0b00110000,
        0b01100000,
        0b11000000,
        0b10000000,
        0b00000000,

    ), // /   
    CHARACTER(
        0b01111000,
        0b11001100,
        0b11011100,
        0b11111100,
        0b11101100,
        0b11001100,
        0b01111100,
        0b00000000,
    ), // 0
    CHARACTER(
        0b00110000,
        0b01110000,
        0b00110000,
        0b00110000,
        0b00110000,
        0b00110000,
        0b11111100,
        0b00000000,
    ), // 1
    CHARACTER(
        0b01111000,
        0b11001100,
        0b00001100,
        0b00111000,
        0b01100000,
        0b11001100,
        0b11111100,
        0b00000000,
    ), // 2
    CHARACTER(
        0b01111000,
        0b11001100,
        0b00001100,
        0b00111000,
        0b00001100,
        0b11001100,
        0b01111000,
        0b00000000,
    ), // 3
    CHARACTER(
        0b00011100,
        0b00111100,
        0b01101100,
        0b11001100,
        0b11111110,
        0b00001100,
        0b00011110,
        0b00000000,
    ), // 4
    CHARACTER(
        0b11111100,
        0b11000000,
        0b11111000,
        0b00001100,
        0b00001100,
        0b11001100,
        0b01111000,
        0b00000000,
    ), // 5
    CHARACTER(
        0b00111000,
        0b01100000,
        0b11000000,
        0b11111000,
        0b11001100,
        0b11001100,
        0b01111000,
        0b00000000,
    ), // 6
    CHARACTER(
        0b11111100,
        0b11001100,
        0b00001100,
        0b00011000,
        0b00110000,
        0b00110000,
        0b00110000,
        0b00000000,
    ), // 7
    CHARACTER(
        0b01111000,
        0b11001100,
        0b11001100,
        0b01111000,
        0b11001100,
        0b11001100,
        0b01111000,
        0b00000000,
    ), // 8
    CHARACTER(
        0b01111000,
        0b11001100,
        0b11001100,
        0b01111100,
        0b00001100,
        0b00011000,
        0b01110000,
        0b00000000,
    ), // 9
    CHARACTER(
        0b00110000,
        0b00110000,
        0b00110000,
        0b00000000,
        0b00110000,
        0b00110000,
        0b00110000,
        0b00000000,
    ), // :
    CHARACTER(
        0b00110000,
        0b00110000,
        0b00110000,
        0b00000000,
        0b00110000,
        0b00110000,
        0b01100000,
        0b00000000,
    ), // ;
    CHARACTER(
        0b00000000,
        0b00001110,
        0b00111000,
        0b01110000,
        0b11100000,
        0b00111000,
        0b00001110,
        0b00000000,
    ), // <
    CHARACTER(
        0b00000000,
        0b00000000,
        0b11111100,
        0b00000000,
        0b11111100,
        0b00000000,
        0b00000000,
        0b00000000,
    ), // =
    CHARACTER(
        0b00000000,
        0b11100000,
        0b00111000,
        0b00011100,
        0b00001110,
        0b00111000,
        0b11100000,
        0b00000000,
    ), // >
    CHARACTER(
        0b01111000,
        0b11001100,
        0b00001100,
        0b00011000,
        0b00110000,
        0b00000000,
        0b00110000,
        0b00000000,
    ), //?
    CHARACTER(
        0b00111100,
        0b01100010,
        0b11001110,
        0b11011010,
        0b11010010,
        0b11011110,
        0b01100110,
        0b00000000,
    ), // @
    CHARACTER(
        0b00110000,
        0b01111000,
        0b11001100,
        0b11001100,
        0b11111100,
        0b11001100,
        0b11001100,
        0b00000000,
    ), // A
    CHARACTER(
        0b11111100,
        0b01100110,
        0b01100110,
        0b01111100,
        0b01100110,
        0b01100110,
        0b11111100,
        0b00000000,
    ), // B
    CHARACTER(
        0b00111100,
        0b01100110,
        0b11000000,
        0b11000000,
        0b11000000,
        0b01100110,
        0b00111100,
        0b00000000,
    ), // C
    CHARACTER(
        0b11111000,
        0b01101100,
        0b01100110,
        0b01100110,
        0b01100110,
        0b01101100,
        0b11111000,
        0b00000000,
    ), // D
    CHARACTER(
        0b11111110,
        0b01100010,
        0b01101000,
        0b01111000,
        0b01101000,
        0b01100010,
        0b11111110,
        0b00000000,
    ), // E
    CHARACTER(
        0b11111110,
        0b01100010,
        0b01101000,
        0b01111000,
        0b01101000,
        0b01100000,
        0b11110000,
        0b00000000,
    ), // F
    CHARACTER(
        0b00111100,
        0b01100110,
        0b11000000,
        0b11000000,
        0b11001110,
        0b01100110,
        0b00111110,
        0b00000000,
    ), // G
    CHARACTER(
        0b11001100,
        0b11001100,
        0b11001100,
        0b11111100,
        0b11001100,
        0b11001100,
        0b11001100,
        0b00000000,
    ), // H
    CHARACTER(
        0b01111000,
        0b00110000,
        0b00110000,
        0b00110000,
        0b00110000,
        0b00110000,
        0b01111000,
        0b00000000,
    ), // I
    CHARACTER(
        0b00011110,
        0b00001100,
        0b00001100,
        0b00001100,
        0b11001100,
        0b11001100,
        0b01111000,
        0b00000000,
    ), // J
    CHARACTER(
        0b11110110,
        0b01100110,
        0b01101100,
        0b01111000,
        0b01101100,
        0b01100110,
        0b11110110,
        0b00000000,
    ), // K
    CHARACTER(
        0b11110000,
        0b01100000,
        0b01100000,
        0b01100000,
        0b01100010,
        0b01100110,
        0b11111110,
        0b00000000,
    ), // L
    CHARACTER(
        0b11000110,
        0b11101110,
        0b11111110,
        0b11111110,
        0b11010110,
        0b11000110,
        0b11000110,
        0b00000000,
    ), // M
    CHARACTER(
        0b11000110,
        0b11100110,
        0b11110110,
        0b11011110,
        0b11001110,
        0b11000110,
        0b11000110,
        0b00000000,
    ), // N
    CHARACTER(
        0b00111000,
        0b01101100,
        0b11000110,
        0b11000110,
        0b11000110,
        0b01101100,
        0b00111000,
        0b00000000,
    ), // O
    CHARACTER(
        0b11111100,
        0b01100110,
        0b01100110,
        0b01111100,
        0b01100000,
        0b01100000,
        0b11110000,
        0b00000000,
    ), // P
    CHARACTER(
        0b01111000,
        0b11001100,
        0b11001100,
        0b11001100,
        0b11011100,
        0b01111000,
        0b00011100,
        0b00000000,
    ), // Q
    CHARACTER(
        0b11111100,
        0b01100110,
        0b01100110,
        0b01111100,
        0b01101100,
        0b01100110,
        0b11110110,
        0b00000000,
    ), // R
    CHARACTER(
        0b01111000,
        0b11001100,
        0b11100000,
        0b01110000,
        0b00011100,
        0b11001100,
        0b01111000,
        0b00000000,
    ), // S
    CHARACTER(
        0b11111100,
        0b10110100,
        0b00110000,
        0b00110000,
        0b00110000,
        0b00110000,
        0b01111000,
        0b00000000,
    ), // T
    CHARACTER(
        0b11001100,
        0b11001100,
        0b11001100,
        0b11001100,
        0b11001100,
        0b11001100,
        0b11111100,
        0b00000000,
    ), // U
    CHARACTER(
        0b11001100,
        0b11001100,
        0b11001100,
        0b11001100,
        0b11001100,
        0b01111000,
        0b00110000,
        0b00000000,
    ), // V
    CHARACTER(
        0b11000110,
        0b11000110,
        0b11000110,
        0b11010110,
        0b11111110,
        0b11101110,
        0b11000110,
        0b00000000,
    ), // W
    CHARACTER(
        0b11000110,
        0b11000110,
        0b01101100,
        0b00111000,
        0b00111000,
        0b01101100,
        0b11000110,
        0b00000000,
    ), // X
    CHARACTER(
        0b11001100,
        0b11001100,
        0b11001100,
        0b01111000,
        0b00110000,
        0b00110000,
        0b01111000,
        0b00000000,
    ), // Y
    CHARACTER(
        0b11111110,
        0b11000110,
        0b10001100,
        0b00011000,
        0b00110010,
        0b01100110,
        0b11111110,
        0b00000000,
    ), // Z
    CHARACTER(
        0b01111000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01111000,
        0b00000000,
    ), // [
    CHARACTER(
        0b10000000,
        0b11000000,
        0b01100000,
        0b00110000,
        0b00011000,
        0b00001100,
        0b00000110,
        0b00000000,
    ), // \ 

    CHARACTER(
        0b01111000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b01111000,
        0b00000000,
    ), // ]
    CHARACTER(
        0b00110000,
        0b01111000,
        0b11001100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
    ), // ^
    CHARACTER(
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b11111100,
        0b00000000,
    ), // _
    CHARACTER(
        0b11100000,
        0b00110000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
    ), // `
    CHARACTER(
        0b00000000,
        0b00000000,
        0b01111000,
        0b00001100,
        0b01111100,
        0b11001100,
        0b01110110,
        0b00000000,
    ), // a
    CHARACTER(
        0b11100000,
        0b01100000,
        0b01100000,
        0b01111100,
        0b01100110,
        0b01100110,
        0b11011100,
        0b00000000,
    ), // b
    CHARACTER(
        0b00000000,
        0b00000000,
        0b01111000,
        0b11001100,
        0b11000000,
        0b11001100,
        0b01111000,
        0b00000000,
    ), // c
    CHARACTER(
        0b00011100,
        0b00001100,
        0b00001100,
        0b01111100,
        0b11001100,
        0b11001100,
        0b01110110,
        0b00000000,
    ), // d
    CHARACTER(
        0b00000000,
        0b00000000,
        0b01111000,
        0b11001100,
        0b11111100,
        0b11000000,
        0b01111000,
        0b00000000,
    ), // e
    CHARACTER(
        0b00111000,
        0b01101100,
        0b01100000,
        0b11110000,
        0b01100000,
        0b01100000,
        0b11110000,
        0b00000000,
    ), // f
    CHARACTER(
        0b00000000,
        0b00000000,
        0b01110110,
        0b11001100,
        0b11001100,
        0b01111100,
        0b00001100,
        0b11111000,
    ), // g
    CHARACTER(
        0b11100000,
        0b01100000,
        0b01101100,
        0b01110110,
        0b01100110,
        0b01100110,
        0b11100110,
        0b00000000,
    ), // h
    CHARACTER(
        0b00110000,
        0b00000000,
        0b01110000,
        0b00110000,
        0b00110000,
        0b00110000,
        0b01111000,
        0b00000000,
    ), // i
    CHARACTER(
        0b00001100,
        0b00000000,
        0b00001100,
        0b00001100,
        0b00001100,
        0b11001100,
        0b11001100,
        0b01111000,
    ), // j
    CHARACTER(
        0b11100000,
        0b01100000,
        0b01100110,
        0b01101100,
        0b01111000,
        0b01101100,
        0b11100110,
        0b00000000,
    ), // k
    CHARACTER(
        0b01110000,
        0b00110000,
        0b00110000,
        0b00110000,
        0b00110000,
        0b00110000,
        0b01111000,
        0b00000000,
    ), // l
    CHARACTER(
        0b00000000,
        0b00000000,
        0b11001100,
        0b11111110,
        0b11111110,
        0b11010110,
        0b11000110,
        0b00000000,
    ), // m
    CHARACTER(
        0b00000000,
        0b00000000,
        0b11111000,
        0b11001100,
        0b11001100,
        0b11001100,
        0b11001100,
        0b00000000,
    ), // n
    CHARACTER(
        0b00000000,
        0b00000000,
        0b01111000,
        0b11001100,
        0b11001100,
        0b11001100,
        0b01111000,
        0b00000000,
    ), // o
    CHARACTER(
        0b00000000,
        0b00000000,
        0b11011100,
        0b01100110,
        0b01100110,
        0b01111100,
        0b01100000,
        0b11110000,
    ), // p
    CHARACTER(
        0b00000000,
        0b00000000,
        0b01110110,
        0b11001100,
        0b11001100,
        0b01111100,
        0b00001100,
        0b00011110,
    ), // q
    CHARACTER(
        0b00000000,
        0b00000000,
        0b10011100,
        0b01110110,
        0b01100110,
        0b01100000,
        0b11110000,
        0b00000000,
    ), // r
    CHARACTER(
        0b00000000,
        0b00000000,
        0b01111100,
        0b11000000,
        0b01111000,
        0b00001100,
        0b11111000,
        0b00000000,
    ), // s
    CHARACTER(
        0b00010000,
        0b00110000,
        0b01111100,
        0b00110000,
        0b00110000,
        0b00110100,
        0b00011000,
        0b00000000,
    ), // t
    CHARACTER(
        0b00000000,
        0b00000000,
        0b11001100,
        0b11001100,
        0b11001100,
        0b11001100,
        0b01110110,
        0b00000000,
    ), // u
    CHARACTER(
        0b00000000,
        0b00000000,
        0b11001100,
        0b11001100,
        0b11001100,
        0b01111000,
        0b00110000,
        0b00000000,
    ), // v
    CHARACTER(
        0b00000000,
        0b00000000,
        0b11000110,
        0b11000110,
        0b11010110,
        0b11111110,
        0b01101100,
        0b00000000,
    ), // w
    CHARACTER(
        0b00000000,
        0b00000000,
        0b11000110,
        0b01101100,
        0b00111000,
        0b01101100,
        0b11000110,
        0b00000000,
    ), // x
    CHARACTER(
        0b00000000,
        0b00000000,
        0b11001100,
        0b11001100,
        0b11001100,
        0b01111100,
        0b00001100,
        0b11111000,
    ), // y
    CHARACTER(
        0b00000000,
        0b00000000,
        0b11111100,
        0b10011000,
        0b00110000,
        0b01100100,
        0b11111100,
        0b00000000,
    ), // z
    CHARACTER(
        0b00001110,
        0b00011000,
        0b00011000,
        0b01100000,
        0b00011000,
        0b00011000,
        0b00001110,
        0b00000000,
    ), // {
    CHARACTER(
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00000000,
    ), // |
    CHARACTER(
        0b01110000,
        0b00011000,
        0b00011000,
        0b00000110,
        0b00011000,
        0b00011000,
        0b01110000,
        0b00000000,
    ), // }
    CHARACTER(
        0b00000000,
        0b01110110,
        0b11011100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
    ), // ~
};

BOOLEAN VBE_DRAW_CHARACTER(U32 x, U32 y, U8 c, VBE_PIXEL_COLOUR fg, VBE_PIXEL_COLOUR bg) {
    VBE_MODE* mode = GET_VBE_MODE();
    if (x >= (U32)(mode->XResolution + VBE_CHAR_WIDTH) || y >= (U32)(mode->YResolution + VBE_CHAR_HEIGHT)) return FALSE;

    c -= UNUSABLE_CHARS; // Align char according to VBE_MAX_CHARS
    if(c >= (U32)VBE_MAX_CHARS) return FALSE; // Invalid character
    // Draw the character
    for (U32 i = 0; i < 8; i++) {
        VBE_LETTERS_TYPE row = VBE_LETTERS[c][i];
        for (U32 j = 0; j < 8; j++) {
            VBE_PIXEL_COLOUR colour = (row & (1 << (7 - j))) ? fg : bg;
            VBE_DRAW_PIXEL(CREATE_VBE_PIXEL_INFO(x + j, y + i, colour));
        }
    }

    return TRUE;
}

U0 ___memcpy(void* dest, const void* src, U32 n) {
    U8* d = (U8*)dest;
    const U8* s = (const U8*)src;
    for (U32 i = 0; i < n; i++) {
        d[i] = s[i];
    }
}

U0 __memcpy_safe_chunks(void* dest, const void* src, U32 n) {
    U8* d = (U8*)dest;
    const U8* s = (const U8*)src;

    // Copy leading bytes until dest is 4-byte aligned
    while (((U32)d & 3) && n) {
        *d++ = *s++;
        n--;
    }

    // Copy 4 bytes at a time
    U32* d32 = (U32*)d;
    const U32* s32 = (const U32*)s;
    while (n >= 4) {
        *d32++ = *s32++;
        n -= 4;
    }

    // Copy remaining bytes
    d = (U8*)d32;
    s = (const U8*)s32;
    while (n--) {
        *d++ = *s++;
    }
}


BOOL vbe_check(U0) {
    VBE_MODE* mode = (VBE_MODE*)(VBE_MODE_LOAD_ADDRESS_PHYS);
    // Check if the mode is valid
    if (mode->ModeAttributes == 0) {
        return FALSE;
    }
    // Check if the physical base pointer is valid
    if (mode->PhysBasePtr == 0) {
        return FALSE;
    }
    // Check if the mode is compatible with the screen size
    if (mode->XResolution < SCREEN_WIDTH || mode->YResolution < SCREEN_HEIGHT) {
        return FALSE;
    }
    return TRUE;
}

U32 strlen(const char* str) {
    U32 length = 0;
    while (str[length] != '\0') {
        length++;
    }
    return length;
}

BOOLEAN VBE_DRAW_STRING(U32 x, U32 y, const char* str, VBE_PIXEL_COLOUR fg, VBE_PIXEL_COLOUR bg) {
    U32 length = strlen(str);
    for (U32 i = 0; i < length; i++) {
        VBE_DRAW_CHARACTER(x + i * VBE_CHAR_WIDTH, y, str[i], fg, bg);
    }
    return TRUE;
}

BOOLEAN VBE_FLUSH_SCREEN(U0) {
    VBE_CLEAR_SCREEN(VBE_BLACK);
    VBE_UPDATE_VRAM();
    return TRUE;
}


U0 VBE_UPDATE_VRAM(U0) {
    VBE_MODE* mode = GET_VBE_MODE();
    if (!mode) return;

    // Copy only the required framebuffer bytes
    U32 copy_size = mode->BytesPerScanLineLinear * mode->YResolution;
    if (copy_size > FRAMEBUFFER_SIZE) copy_size = FRAMEBUFFER_SIZE;
    ___memcpy((void*)mode->PhysBasePtr, (void*)FRAMEBUFFER_ADDRESS, copy_size);
    // __memcpy_safe_chunks((void*)mode->PhysBasePtr, (void*)FRAMEBUFFER_ADDRESS, copy_size);
}

U0 VBE_STOP_DRAWING(U0) {
    VBE_UPDATE_VRAM();
}

BOOLEAN VBE_DRAW_FRAMEBUFFER(U32 pos, VBE_PIXEL_COLOUR colour) {
    VBE_MODE* mode = (VBE_MODE*)(VBE_MODE_LOAD_ADDRESS_PHYS);
    U8* framebuffer = (U8*)(FRAMEBUFFER_ADDRESS);
    if(colour == VBE_SEE_THROUGH) return TRUE;

    if (!framebuffer || !mode) return FALSE;

    U32 bytes_per_pixel = (mode->BitsPerPixel + 7) / 8;
    if (pos + bytes_per_pixel > mode->BytesPerScanLineLinear * mode->YResolution) return FALSE;
    if (pos + bytes_per_pixel > FRAMEBUFFER_SIZE) return FALSE; // RAM framebuffer bounds

    switch (bytes_per_pixel) {
        case 1:
            framebuffer[pos] = (U8)(colour & 0xFF);
            break;

        case 2: {
            U16 val16 = (U16)colour;
            framebuffer[pos + 0] = (U8)(val16 & 0xFF);
            framebuffer[pos + 1] = (U8)((val16 >> 8) & 0xFF);
            break;
        }

        case 3:
            framebuffer[pos + 0] = (U8)(colour & 0xFF);
            framebuffer[pos + 1] = (U8)((colour >> 8) & 0xFF);
            framebuffer[pos + 2] = (U8)((colour >> 16) & 0xFF);
            break;

        case 4:
            framebuffer[pos + 0] = (U8)(colour & 0xFF);
            framebuffer[pos + 1] = (U8)((colour >> 8) & 0xFF);
            framebuffer[pos + 2] = (U8)((colour >> 16) & 0xFF);
            framebuffer[pos + 3] = (U8)((colour >> 24) & 0xFF);
            break;

        default:
            return FALSE;
    }

    return TRUE;
}

BOOLEAN VBE_DRAW_PIXEL(VBE_PIXEL_INFO pixel_info) {
    VBE_MODE* mode = (VBE_MODE*)(VBE_MODE_LOAD_ADDRESS_PHYS);
    if (!mode) return FALSE;

    if ((I32)pixel_info.X < 0 || (I32)pixel_info.Y < 0) return FALSE;
    if ((U32)pixel_info.X >= SCREEN_WIDTH || (U32)pixel_info.Y >= SCREEN_HEIGHT) return FALSE;

    U32 bytes_per_pixel = (mode->BitsPerPixel + 7) / 8;
    U32 pos = (pixel_info.Y * mode->BytesPerScanLineLinear) + (pixel_info.X * bytes_per_pixel);

    return VBE_DRAW_FRAMEBUFFER(pos, pixel_info.Colour);
}



BOOLEAN VBE_DRAW_ELLIPSE(U32 x0, U32 y0, U32 a, U32 b, VBE_PIXEL_COLOUR fill_colours) {
    I32 x = 0;
    I32 y = b;
    U32 a2 = a*a;
    U32 b2 = b*b;
    I32 dx = 0;
    I32 dy = 2*a2*y;
    I32 d1 = b2 - a2*b + a2/4;

    while(dx < dy) {
        for(I32 px = -x; px <= x; px++) {
            VBE_DRAW_PIXEL(CREATE_VBE_PIXEL_INFO(x0 + px, y0 + y, fill_colours));
            VBE_DRAW_PIXEL(CREATE_VBE_PIXEL_INFO(x0 + px, y0 - y, fill_colours));
        }
        x++;
        dx += 2*b2;
        if(d1 < 0) {
            d1 += b2 + dx;
        } else {
            y--;
            dy -= 2*a2;
            d1 += b2 + dx - dy;
        }
    }

    I32 d2 = b2*(x*x + x) + a2*(y-1)*(y-1) - a2*b2;
    while(y >= 0) {
        for(I32 px = -x; px <= x; px++) {
            VBE_DRAW_PIXEL(CREATE_VBE_PIXEL_INFO(x0 + px, y0 + y, fill_colours));
            VBE_DRAW_PIXEL(CREATE_VBE_PIXEL_INFO(x0 + px, y0 - y, fill_colours));
        }
        y--;
        dy -= 2*a2;
        if(d2 > 0) {
            d2 += a2 - dy;
        } else {
            x++;
            dx += 2*b2;
            d2 += a2 - dy + dx;
        }
    }

    return TRUE;
}

BOOLEAN VBE_DRAW_LINE(U32 x0_in, U32 y0_in, U32 x1_in, U32 y1_in, VBE_PIXEL_COLOUR colour) {
    I32 x0 = (I32)x0_in;
    I32 y0 = (I32)y0_in;
    I32 x1 = (I32)x1_in;
    I32 y1 = (I32)y1_in;

    I32 dx = abs(x1 - x0);
    I32 sx = (x0 < x1) ? 1 : -1;
    I32 dy = -abs(y1 - y0);
    I32 sy = (y0 < y1) ? 1 : -1;
    I32 err = dx + dy;  // error term

    // safety: upper-bound on iterations to avoid infinite loops while debugging
    I32 max_iters = dx + abs(dy) + 4;
    if (max_iters < 0) max_iters = FRAMEBUFFER_SIZE; // paranoid fallback. Loops should not exceed framebuffer size
    I32 iters = 0;

    while (1) {
        // draw only when inside the screen bounds (VBE_DRAW_PIXEL already checks, but
        // avoiding the call can be a tiny speedup)
        if ((U32)x0 < SCREEN_WIDTH && (U32)y0 < SCREEN_HEIGHT) {
            VBE_DRAW_PIXEL(CREATE_VBE_PIXEL_INFO((U32)x0, (U32)y0, colour));
        }

        if (x0 == x1 && y0 == y1) break;

        if (++iters > max_iters) {
            // something went wrong — bail out to avoid hang
            return FALSE;
        }

        I32 e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }

    return TRUE;
}

BOOLEAN VBE_CLEAR_SCREEN(VBE_PIXEL_COLOUR colour) {
    VBE_MODE* mode = GET_VBE_MODE();
    if (!mode) return FALSE;

    U32 bytes_per_pixel = (mode->BitsPerPixel + 7) / 8;
    U32 total_bytes = mode->BytesPerScanLineLinear * mode->YResolution;
    U32 total_pixels = total_bytes / bytes_per_pixel;
    U32 pos = 0;

    for (U32 i = 0; i < total_pixels; i++) {
        if (!VBE_DRAW_FRAMEBUFFER(pos, colour)) {
            return FALSE;
        }
        pos += bytes_per_pixel;
    }

    return TRUE;
}




BOOLEAN VBE_DRAW_RECTANGLE(U32 x, U32 y, U32 width, U32 height, VBE_PIXEL_COLOUR colours) {
    if (width == 0 || height == 0) {
        return TRUE;
    }

    U32 errcnt = 0;

    if(!VBE_DRAW_LINE(x, y, width, y, colours)) errcnt++;
    if(!VBE_DRAW_LINE(width, y, width, height, colours)) errcnt++;
    if(!VBE_DRAW_LINE(width, height, x, height, colours)) errcnt++;
    if(!VBE_DRAW_LINE(x, height, x, y, colours)) errcnt++;

    return errcnt == 0;
}
BOOLEAN VBE_DRAW_TRIANGLE(U32 x1, U32 y1, U32 x2, U32 y2, U32 x3, U32 y3, VBE_PIXEL_COLOUR colours) {
    VBE_DRAW_LINE(x1, y1, x2, y2, colours);
    VBE_DRAW_LINE(x2, y2, x3, y3, colours);
    VBE_DRAW_LINE(x3, y3, x1, y1, colours);
    return TRUE;
}

#include <stddef.h> /* for NULL if needed */

/* assume these types/macros exist in your codebase */
#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

/* prototypes already exist in your codebase:
   BOOLEAN VBE_DRAW_PIXEL(VBE_PIXEL_INFO pixel_info);
   extern const U32 SCREEN_WIDTH, SCREEN_HEIGHT;
   VBE_PIXEL_INFO has members: U32 X, Y; VBE_PIXEL_COLOUR Colour;
*/

BOOLEAN VBE_DRAW_RECTANGLE_FILLED(U32 x, U32 y, U32 width, U32 height, VBE_PIXEL_COLOUR colours) {
    if (width == 0 || height == 0) return FALSE;

    /* calculate clipping boundaries */
    U32 x0 = x;
    U32 y0 = y;
    U32 x1 = x + width;   /* exclusive */
    U32 y1 = y + height;  /* exclusive */

    /* clip to screen */
    if (x0 >= SCREEN_WIDTH || y0 >= SCREEN_HEIGHT) return FALSE;
    if (x1 > SCREEN_WIDTH) x1 = SCREEN_WIDTH;
    if (y1 > SCREEN_HEIGHT) y1 = SCREEN_HEIGHT;

    if (x0 >= x1 || y0 >= y1) return FALSE;

    VBE_PIXEL_INFO p;
    p.Colour = colours;
    BOOLEAN any = FALSE;

    for (U32 yy = y0; yy < y1; ++yy) {
        p.Y = yy;
        for (U32 xx = x0; xx < x1; ++xx) {
            p.X = xx;
            if (VBE_DRAW_PIXEL(p)) any = TRUE;
        }
    }

    return any;
}


BOOLEAN VBE_DRAW_TRIANGLE_FILLED(U32 x1, U32 y1, U32 x2, U32 y2, U32 x3, U32 y3, VBE_PIXEL_COLOUR colours) {
    /* Compute bounding box (inclusive) */
    U32 minx = x1;
    if (x2 < minx) minx = x2;
    if (x3 < minx) minx = x3;

    U32 maxx = x1;
    if (x2 > maxx) maxx = x2;
    if (x3 > maxx) maxx = x3;

    U32 miny = y1;
    if (y2 < miny) miny = y2;
    if (y3 < miny) miny = y3;

    U32 maxy = y1;
    if (y2 > maxy) maxy = y2;
    if (y3 > maxy) maxy = y3;

    /* Quick reject if bbox completely off-screen */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    if (maxx < 0 || maxy < 0) return FALSE; /* defensive, though U32 can't be <0 */
#pragma GCC diagnostic pop
    if (minx >= SCREEN_WIDTH || miny >= SCREEN_HEIGHT) return FALSE;
    /* Clip bounding box to screen */
    if (minx >= SCREEN_WIDTH) return FALSE;
    if (miny >= SCREEN_HEIGHT) return FALSE;

    U32 bx0 = minx;
    U32 by0 = miny;
    U32 bx1 = maxx;
    U32 by1 = maxy;

    if (bx0 >= SCREEN_WIDTH) bx0 = SCREEN_WIDTH - 1;
    if (by0 >= SCREEN_HEIGHT) by0 = SCREEN_HEIGHT - 1;
    if (bx1 >= SCREEN_WIDTH) bx1 = SCREEN_WIDTH - 1;
    if (by1 >= SCREEN_HEIGHT) by1 = SCREEN_HEIGHT - 1;

    /* Use integer edge functions (64-bit to avoid overflow) */
    const I32 ax = (I32)x1;
    const I32 ay = (I32)y1;
    const I32 bx = (I32)x2;
    const I32 by = (I32)y2;
    const I32 cx = (I32)x3;
    const I32 cy = (I32)y3;

    VBE_PIXEL_INFO p;
    p.Colour = colours;
    BOOLEAN any = FALSE;

    for (U32 yy = bx0 /* dummy init */; yy <= bx1; ++yy) { /* we will overwrite loops below */
        break;
    }
    /* iterate y then x within clipped bbox */
    for (U32 yy = by0; yy <= by1; ++yy) {
        for (U32 xx = bx0; xx <= bx1; ++xx) {
            /* compute edge functions relative to point (xx, yy) */
            I32 px = (I32)xx;
            I32 py = (I32)yy;

            I32 e0 = (bx - ax) * (py - ay) - (by - ay) * (px - ax); /* edge AB */
            I32 e1 = (cx - bx) * (py - by) - (cy - by) * (px - bx); /* edge BC */
            I32 e2 = (ax - cx) * (py - cy) - (ay - cy) * (px - cx); /* edge CA */

            /* point is inside if all edge functions have same sign (or zero) */
            if ((e0 >= 0 && e1 >= 0 && e2 >= 0) || (e0 <= 0 && e1 <= 0 && e2 <= 0)) {
                p.X = xx;
                p.Y = yy;
                if (VBE_DRAW_PIXEL(p)) any = TRUE;
            }
        }
    }

    return any;
}



/* Saved for later:
Bugs and draws out line patterns
BOOLEAN VBE_DRAW_LINE(U32 x1, U32 y1, U32 x2, U32 y2, VBE_PIXEL_COLOUR colours, U32 thickness) {
    I32 t_2 = (I32)thickness / 2;
    I32 A = (I32)y2 - (I32)y1;
    I32 B = (I32)x1 - (I32)x2;
    I32 C = (I32)x2 * (I32)y1 - (I32)x1 * (I32)y2;
    I32 A2 = A * A;
    I32 B2 = B * B;

    I32 xmin = (I32)x1 < (I32)x2 ? (I32)x1 : (I32)x2;
    I32 xmax = (I32)x1 > (I32)x2 ? (I32)x1 : (I32)x2;
    I32 ymin = (I32)y1 < (I32)y2 ? (I32)y1 : (I32)y2;
    I32 ymax = (I32)y1 > (I32)y2 ? (I32)y1 : (I32)y2;

    xmin -= t_2; xmax += t_2;
    ymin -= t_2; ymax += t_2;

    I32 threshold2 = pow2(t_2) * (A2 + B2);
    VBE_MODE* mode = GET_VBE_MODE();
    for(I32 x = xmin; x <= xmax; x++) {
        for(I32 y = ymin; y <= ymax; y++) {
            if(x < 0 || y < 0 || x >= mode->XResolution || y >= mode->YResolution) continue;
            I32 d = A * x + B * y + C;
            if (d*d <= threshold2) {
                VBE_DRAW_PIXEL(CREATE_VBE_PIXEL_INFO(x, y, colours));
            }
        }
    }
    return TRUE;
}
*/