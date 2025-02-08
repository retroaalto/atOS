/*+++
ISO 9660 reader

This program reads the primary volume descriptor of an ISO 9660 image and prints
the volume identifier and volume space size.

Usage: ISO9660 <iso_image> [<file_name>]

Compile with:
    gcc -o ISO9660 ISO9660.c -I. -Wall -Wextra
    cl ISO9660.c /I. /W4

Run with:
    ISO9660 <iso_image> [<file_name>]

---*/

#include "./ISO9660.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <iso_image> [<file_name>]\n", argv[0]);
        return 1;
    }
    FILE *iso = fopen(argv[1], "rb");
    if (!iso) {
        fprintf(stderr, "Failed to open ISO file");
        return 1;
    }
    printf("Reading ISO 9660 image: %s\n", argv[1]);
    IsoPrimaryVolumeDescriptor pvd;
    read_primary_volume_descriptor(iso, &pvd);
    
    printf("Volume Identifier: %s\n", pvd.volumeIdentifier);
    printf("Volume Space Size: %u\n", pvd.volumeSpaceSize);

    if(argc < 3) {
        printf("No file name specified, exiting...\n");
        fclose(iso);
        return 0;
    }
    printf("Reading file: %s\n", argv[2]);
    fclose(iso);
    return 0;
}
