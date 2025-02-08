/*+++
FAT16 file system reader

Reads a file from a FAT16 image and prints its contents.

Usage: FAT16 <iso_image> <file_name>

Compile with:
    gcc -o FAT16 FAT16.c -I. -I../ISO9660 -Wall -Wextra

Run with:
    FAT16 <iso_image> <file_name>
---*/

#include "../ISO9660/ISO9660.h"

int main(int argc, char *argv[]){
    if(argc < 3){
        printf("Usage: %s <iso_image> <file_name>\n", argv[0]);
        return 1;
    }
    
    return 0;
}