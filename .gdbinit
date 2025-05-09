set architecture i8086
target remote localhost:1234
set disassembly-flavor intel
break *0x7c00
x/10i 0x7c00