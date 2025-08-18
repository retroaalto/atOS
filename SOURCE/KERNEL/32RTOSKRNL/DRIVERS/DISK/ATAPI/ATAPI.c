#include "./ATAPI.h"
// Return: TRUE if successful, FALSE if error
BOOLEAN read_cdrom(U16 port, BOOLEAN slave, U32 lba, U32 sectors, U16 *buf) {
    VOLATILE U8 read_cmd[12] = {0xA8, 0,
                                (lba >> 0x18) & 0xFF, (lba >> 0x10) & 0xFF, (lba >> 0x08) & 0xFF,
                                (lba >> 0x00) & 0xFF,
                                (sectors >> 0x18) & 0xFF, (sectors >> 0x10) & 0xFF, (sectors >> 0x08) & 0xFF,
                                (sectors >> 0x00) & 0xFF,
                                0, 0};

    outb(port + ATA_DRIVE_HEAD, 0xA0 | (slave << 4)); // Select drive
    ata_io_wait(port);
    outb(port + ATA_ERR, 0);
    outb(port + ATA_LBA_MID, 2048 & 0xFF); // Set sector size to 2048 bytes
    outb(port + ATA_LBA_HI, 2048 >> 8); // Set sector size to 2048 bytes
    outb(port + ATA_COMM_REGSTAT, ATAPI_PACKET_CMD);
    ata_io_wait(port);

    while (1) {
        U8 status = inb(port + ATA_COMM_REGSTAT);
        if ((status & STAT_ERR)) return FALSE;
        if (!(status & STAT_BSY) && (status & STAT_DRQ)) break;
        ata_io_wait(port);
    }

    outsw(port + ATA_DATA, (U16*)read_cmd, 6); // Send 12-byte command as 6 words
    ata_io_wait(port);
    
    for (U32 i = 0; i < sectors; i++) {
        while (1) {
            U8 status = inb(port + ATA_COMM_REGSTAT);
            if ((status & STAT_ERR)) return FALSE;
            if (!(status & STAT_BSY) && (status & STAT_DRQ)) break;
        }

        U32 size = (inb(port + ATA_LBA_HI) << 8) | inb(port + ATA_LBA_MID);

        insw(port + ATA_DATA, buf + (i * (size / 2)), size / 2);
    }

    return TRUE;
}
