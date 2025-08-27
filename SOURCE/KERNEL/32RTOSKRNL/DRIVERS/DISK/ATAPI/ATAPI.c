#include "./ATAPI.h"
#include "../MEMORY/MEMORY.h"
#include "../../VIDEO/VBE.h"

// Check if ATAPI device exists on the given channel and drive (master/slave)
BOOLEAN atapi_cdrom_exists(U16 base_port, U8 drive) {
    U8 status;

    // Select drive (master or slave)
    outb(base_port + ATA_DRIVE_HEAD, drive);
    ata_io_wait(base_port);

    // Send IDENTIFY PACKET command (ATAPI)
    outb(base_port + ATA_COMM_REGSTAT, ATAPI_IDENTIFY_CMD);
    ata_io_wait(base_port);

    // Read status
    inb(base_port + ATA_COMM_REGSTAT, status);

    // If status is zero, no device exists
    if (status == 0) return FALSE;

    status = _inb(base_port + ATA_COMM_REGSTAT);
    if (status == 0) return FALSE;
    while (status & STAT_BSY) status = _inb(base_port + ATA_COMM_REGSTAT);
    // Check if DRQ is set (indicates ready to transfer data)
    if (status & STAT_DRQ) return TRUE;
    // Check for errors
    if (status & STAT_ERR) return FALSE;
    return FALSE;
}

U32 ATAPI_CHECK() {
    // Check primary master
    if (atapi_cdrom_exists(ATA_PRIMARY_BASE, ATA_MASTER)) return ATAPI_PRIMARY_MASTER;

    // Check primary slave
    if (atapi_cdrom_exists(ATA_PRIMARY_BASE, ATA_SLAVE)) return ATAPI_PRIMARY_SLAVE;

    // Check secondary master
    if (atapi_cdrom_exists(ATA_SECONDARY_BASE, ATA_MASTER)) return ATAPI_SECONDARY_MASTER;

    // Check secondary slave
    if (atapi_cdrom_exists(ATA_SECONDARY_BASE, ATA_SLAVE)) return ATAPI_SECONDARY_SLAVE;

    return 0; // No ATAPI CD-ROM found
}


BOOLEAN READ_CDROM(U32 ATAPICheckRes, U32 lba, U32 sectors, U16 *buf) {
    U32 base_port;
    U8 drive;

    switch (ATAPICheckRes) {
        case ATAPI_PRIMARY_MASTER:   base_port = ATA_PRIMARY_BASE;   drive = ATA_MASTER; break;
        case ATAPI_PRIMARY_SLAVE:    base_port = ATA_PRIMARY_BASE;   drive = ATA_SLAVE;  break;
        case ATAPI_SECONDARY_MASTER: base_port = ATA_SECONDARY_BASE; drive = ATA_MASTER; break;
        case ATAPI_SECONDARY_SLAVE:  base_port = ATA_SECONDARY_BASE; drive = ATA_SLAVE;  break;
        default: return ATAPI_FAILED;
    }

    for (U32 sector = 0; sector < sectors; sector++) {
        U8 packet[12] = {0};
        packet[0] = ATAPI_CMD_READ12;
        packet[2] = (lba >> 24) & 0xFF;
        packet[3] = (lba >> 16) & 0xFF;
        packet[4] = (lba >> 8) & 0xFF;
        packet[5] = lba & 0xFF;
        packet[9] = 1;  // read 1 sector

        outb(base_port + ATA_DRIVE_HEAD, drive);
        ata_io_wait(base_port);

        outb(base_port + ATA_COMM_REGSTAT, ATAPI_PACKET_CMD);
        ata_io_wait(base_port);

        for (int i = 0; i < 12; i += 2) {
            outw(base_port + ATA_DATA, *(U16 *)(packet + i));
        }

        while (1) {
            U8 status = _inb(base_port + ATA_COMM_REGSTAT);
            if (status & STAT_ERR) return ATAPI_FAILED;
            if (!(status & STAT_BSY) && (status & STAT_DRQ)) break;
            ata_io_wait(base_port);
        }

        for (int i = 0; i < ATAPI_SECTOR_SIZE / 2; i++) {
            buf[i] = _inw(base_port + ATA_DATA);
        }

        buf += ATAPI_SECTOR_SIZE / 2;
        lba++;
    }

    return ATAPI_SUCCESS;
}
