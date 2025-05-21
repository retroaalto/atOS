%ifndef MEMORY_INC
%define MEMORY_INC

KRNL_ADDRESS    equ     0x2000
KRNL_SIZE       equ     0x6000
KRNL_END        equ     (KRNL_ADDRESS + KRNL_SIZE - 1)

E820_ADDRESS    equ     0x8000
E820_SIZE       equ     0x1000
E820_END        equ     (E820_ADDRESS + E820_SIZE - 1)

RTOSKRNL_ADDRESS equ    0x00200000
RTOSKRNL_SIZE    equ    0x00200000
RTOSKRNL_END    equ     (RTOSKRNL_ADDRESS + RTOSKRNL_SIZE)

STACK_ADDRESS   equ    0x90000
STACK_END       equ    0xA0000
STACK_SIZE      equ    (STACK_END - STACK_ADDRESS)

; Program memory. Programs are loaded here.
PROGRAM_ADDRESS equ    0x00800000
PROGRAM_END     equ    0x00FFFFFF
PROGRAM_SIZE    equ    (PROGRAM_END - PROGRAM_ADDRESS)

; Data memory. Data is loaded here.
DATA_ADDRESS    equ    0x00100000
DATA_END        equ    0x001FFFFF
DATA_SIZE       equ    (DATA_END - DATA_ADDRESS)

%endif ; MEMORY_INC
