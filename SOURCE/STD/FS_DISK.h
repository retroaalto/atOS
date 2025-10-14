/**
 * @file FS_DISK.h
 * @author Antonako1
 * @brief Filesystem and Disk IO functions
 */
#ifndef STD_FS_DISK_H
#define STD_FS_DISK_H

#include <STD/TYPEDEF.h>

#define ISO9660_ONLY_DEFINES
#include <FS/ISO9660/ISO9660.h> // For ISO9660 filesystem types

#define FAT_ONLY_DEFINES
#include <FS/FAT/FAT.h>     // For FAT32 filesystem types

#undef ISO9660_ONLY_DEFINES
#undef FAT_ONLY_DEFINES

/**
 * Filesystem types
 */

/// @brief Reads an ISO9660 file record from the filesystem.
/// @param path Path to the file/ directory inside the ISO9660 image
/// @note Path must NOT be ISO9660 normalized, the function will do it.
/// @note Free the returned pointer with FREE_ISO9660_MEMORY when done.
/// @return Pointer to the file record structure or NULL on failure
IsoDirectoryRecord *READ_ISO9660_FILERECORD(CHAR *path);

/// @brief Reads the contents of an ISO9660 file.
/// @param dir_ptr Pointer to the file record structure
/// @note Free the returned pointer with FREE_ISO9660_MEMORY when done.
/// @return Pointer to the file contents or NULL on failure
VOIDPTR READ_ISO9660_FILECONTENTS(IsoDirectoryRecord *dir_ptr);

/// @brief Frees memory allocated by ISO9660 functions.
/// @param ptr Pointer to the memory to free
VOID FREE_ISO9660_MEMORY(VOIDPTR ptr);

/**
 * Disk IO
 */
/// @brief Reads sectors from main CD-ROM drive.
/// @param lba Logical Block Address to start reading from
/// @param sectors Number of sectors to read
/// @param buf Buffer to store read data
/// @note Buffer must be at least 2048 bytes (CD-ROM sector size)
/// @return Non zero on success, zero on failure
U32 CDROM_READ(U32 lba, U32 sectors, U8 *buf);

/// @brief Reads from a hard disk
/// @param lba Lba to read from disk
/// @param sectors Sectors to read (Max 256)
/// @param buf Buffer, must (sectors * 512)
/// @return Non zero on success, zero on failure
U32 HDD_READ(U32 lba, U32 sectors, U8 *buf);

/// @brief Reads from a hard disk
/// @param lba Lba to write to disk
/// @param sectors Sectors to write (Max 256)
/// @param buf Buffer, must (sectors * 512)
/// @return Non zero on success, zero on failure
U32 HDD_WRITE(U32 lba, U32 sectors, U8 *buf);

#endif // STD_FS_DISK_H