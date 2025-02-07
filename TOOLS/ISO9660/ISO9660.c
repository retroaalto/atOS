/*+++
ISO 9660 reader

This program reads the primary volume descriptor of an ISO 9660 image and prints
the volume identifier and volume space size.

Usage: ISO9660 <iso_image>

Compile with:
    gcc -o ISO9660 ISO9660.c -I. -Wall -Wextra

Run with:
    ISO9660 <iso_image>

---*/

#include "ISO9660.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <iso_image>\n", argv[0]);
        return 1;
    }
    
    FILE *iso = fopen(argv[1], "rb");
    if (!iso) {
        perror("Failed to open ISO file");
        return 1;
    }
    
    read_primary_volume_descriptor(iso);
    
    fclose(iso);
    return 0;
}
