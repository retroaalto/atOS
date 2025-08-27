#include "./ISO9660.h"
#include "../MEMORY/MEMORY.h"

BOOLEAN strncmp(const char *s1, const char *s2, U32 n) {
    while (n--) {
        if (*s1 != *s2) {
            return FALSE;
        }
        if (*s1 == '\0') {
            return TRUE;
        }
        s1++;
        s2++;
    }
    return TRUE;
}
BOOLEAN strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return (*s1 == *s2);
}

BOOLEAN ISO9660_IMAGE_CHECK(U0) {
    PrimaryVolumeDescriptor pvd;
    if(!ISO9660_READ_PRIMARYVOLUMEDESCRIPTOR(&pvd)) {
        return FALSE;
    }
    if(!strncmp(pvd.standardIdentifier, "CD001", 5)) {
        return FALSE;
    }
    return TRUE;
}
BOOLEAN ISO9660_READ_PRIMARYVOLUMEDESCRIPTOR(PrimaryVolumeDescriptor *descriptor) {
    // Read the primary volume descriptor from the ISO9660 image via ATAPI
    U32 res = ATAPI_CHECK();
    
    return TRUE;
}
BOOLEAN ISO9660_NORMALIZE_PATH(CHAR *path) {
    return TRUE;
}
BOOLEAN ISO9660_READ_DIRECTORY_RECORD(CHAR *path, IsoDirectoryRecord *record, U32 size) {
    return TRUE;
}
BOOLEAN ISO9660_READFILE_TO_MEMORY(CHAR *path, U0 *outBuffer, U32 outSize) {
    return TRUE;
}

