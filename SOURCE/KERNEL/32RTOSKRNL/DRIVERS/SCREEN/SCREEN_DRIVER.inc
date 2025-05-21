;
%ifndef SCREEN_DRIVER_INC
%define SCREEN_DRIVER_INC

; ----------------------------------------
; SuperVGA Mode Info Block
; ----------------------------------------
;VESA_START:
;
;MODE_ATTRIB        dw 0x0000      ; 0x00: ModeAttributes (2 bytes)
;WIN_A_ATTRIB       db 0x00        ; 0x02: WinAAttributes (1 byte)
;WIN_B_ATTRIB       db 0x00        ; 0x03: WinBAttributes (1 byte)
;WIN_GRANULARITY    dw 0x0000      ; 0x04: WinGranularity (2 bytes)
;WIN_SIZE           dw 0x0000      ; 0x06: WinSize (2 bytes)
;WIN_A_SEGMENT      dw 0x0000      ; 0x08: WinASegment (2 bytes)
;WIN_B_SEGMENT      dw 0x0000      ; 0x0A: WinBSegment (2 bytes)
;WIN_FUNC_PTR       dd 0x00000000  ; 0x0C: WinFuncPtr (4 bytes)
;BYTES_PER_SCANLINE dw 0x0000      ; 0x10: BytesPerScanLine (2 bytes)
;
;X_RESOLUTION       dw 0x0000      ; 0x12: XResolution (2 bytes)
;Y_RESOLUTION       dw 0x0000      ; 0x14: YResolution (2 bytes)
;X_CHAR_SIZE        db 0x00        ; 0x16: XCharSize (1 byte)
;Y_CHAR_SIZE        db 0x00        ; 0x17: YCharSize (1 byte)
;NUMBER_OF_PLANES   db 0x00        ; 0x18: NumberOfPlanes (1 byte)
;BITS_PER_PIXEL     db 0x00        ; 0x19: BitsPerPixel (1 byte)
;NUMBER_OF_BANKS    db 0x00        ; 0x1A: NumberOfBanks (1 byte)
;MEMORY_MODEL       db 0x00        ; 0x1B: MemoryModel (1 byte)
;BANK_SIZE          db 0x00        ; 0x1C: BankSize (1 byte)
;NUMBER_OF_IMAGES   db 0x00        ; 0x1D: NumberOfImagePages (1 byte)
;RESERVED1          db 0x00        ; 0x1E: Reserved (1 byte)
;
;RED_MASK_SIZE      db 0x00        ; 0x1F
;RED_FIELD_POS      db 0x00        ; 0x20
;GREEN_MASK_SIZE    db 0x00        ; 0x21
;GREEN_FIELD_POS    db 0x00        ; 0x22
;BLUE_MASK_SIZE     db 0x00        ; 0x23
;BLUE_FIELD_POS     db 0x00        ; 0x24
;RSVD_MASK_SIZE     db 0x00        ; 0x25
;RSVD_FIELD_POS     db 0x00        ; 0x26
;DIRECT_COLOR_MODE  db 0x00        ; 0x27
;
;PHYS_BASE_PTR      dd 0x00000000  ; 0x28: Physical address of linear frame buffer
;OFFSCREEN_MEM_OFF  dd 0x00000000  ; 0x2C
;OFFSCREEN_MEM_SIZE dw 0x0000      ; 0x30
;
;; Reserved 206 bytes from 0x32 to 0xFF
;times 206 db 0x00
;
;VESA_END:
;
;VESAR:
;    dw VESA_END - VESA_START - 1  ; Limit (size - 1)
;    dd VESA_START                  ; Base address (pointer to VESA structure)
;

%endif ; SCREEN_DRIVER_INC