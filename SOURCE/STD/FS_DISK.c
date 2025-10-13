#include <STD/FS_DISK.h>
#include <CPU/SYSCALL/SYSCALL.h>
#include <STD/MEM.h>
U32 CDROM_READ(U32 lba, U32 sectors, U8 *buf) {
    return SYSCALL3(SYSCALL_CDROM_READ, lba, sectors, buf);
}

IsoDirectoryRecord *READ_ISO9660_FILERECORD(CHAR *path) {
    CHARPTR p = MAlloc(ISO9660_MAX_PATH);
    if (!p) return NULLPTR;
    MEMSET(p, 0, ISO9660_MAX_PATH);
    U32 path_len = STRLEN(path);
    if (path_len >= ISO9660_MAX_PATH) {
        Free(p);
        return NULLPTR;
    }
    STRCPY(p, path);
    return (IsoDirectoryRecord *)SYSCALL1(SYSCALL_ISO9660_READ_ENTRY, (U32)path);
}
VOIDPTR READ_ISO9660_FILECONTENTS(IsoDirectoryRecord *dir_ptr) {
    if (!dir_ptr) return NULLPTR;
    return (VOIDPTR)SYSCALL1(SYSCALL_ISO9660_FILECONTENTS, (U32)dir_ptr);
}
VOID FREE_ISO9660_MEMORY(VOIDPTR ptr) {
    if (!ptr) return;
    SYSCALL1(SYSCALL_ISO9660_FREE_MEMORY, (U32)ptr);
}

U32 HDD_WRITE(U32 lba, U32 sectors, U8 *buf) {
    return SYSCALL3(SYSCALL_HDD_WRITE, lba, sectors, (U32)buf);
}
U32 HDD_READ(U32 lba, U32 sectors, U8 *buf) {
    return SYSCALL3(SYSCALL_HDD_READ, lba, sectors, (U32)buf);
}
