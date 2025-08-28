#include "./ATA_ATAPI.h"
#include "../MEMORY/MEMORY.h"
#include "../VIDEO/VBE.h"
#include "../../../../STD/ASM.h"

static inline void ata_io_wait(const U8 p) {
	U8 val = 0;
	inb(p + ATA_CONTROL_REG + ATA_ALTERNATE_STATUS, val);
	inb(p + ATA_CONTROL_REG + ATA_ALTERNATE_STATUS, val);
	inb(p + ATA_CONTROL_REG + ATA_ALTERNATE_STATUS, val);
	inb(p + ATA_CONTROL_REG + ATA_ALTERNATE_STATUS, val);
}


// Check if ATAPI device exists on the given channel and drive (master/slave)
BOOLEAN atapi_cdrom_exists(U16 base_port, U8 drive) {
    U8 status;

    // Select drive
    _outb(base_port + ATA_DRIVE_HEAD, drive);
    ata_io_wait(base_port);

    // Send IDENTIFY PACKET (0xA1)
    _outb(base_port + ATA_COMM_REG, ATAPI_CMD_IDENTIFY);
    ata_io_wait(base_port);

    status = _inb(base_port + ATA_COMM_REG);
    if (status == 0) return FALSE;  // no device at all

    // Wait for BSY to clear
    while (status & STAT_BSY) status = _inb(base_port + ATA_COMM_REG);

    if (status & STAT_ERR) return FALSE; // not ATAPI
    if (!(status & STAT_DRQ)) return FALSE; // not ready to transfer data

    // Read 256 words of IDENTIFY data
    U16 ident[256];
    for (int i = 0; i < 256; i++) {
        ident[i] = _inw(base_port + ATA_DATA);
    }

    // Word 0: device type info
    if ((ident[0] & 0xC000) == 0x8000) {
        return TRUE; // ATAPI device (CD/DVD)
    }

    return FALSE; // some other device
}


BOOLEAN ATA_HDD_EXISTS(U0) {
    return TRUE;
}

BOOLEAN ATA_CHECK() {
    return TRUE;
}

U32 ATAPI_CHECK() {
    // Check primary master
    if (atapi_cdrom_exists(ATA_PRIMARY_BASE, ATA_MASTER)) return ATAPI_PRIMARY_MASTER;

    // Check primary slave
    if (atapi_cdrom_exists(ATA_PRIMARY_BASE, ATA_SLAVE)) return ATAPI_PRIMARY_SLAVE;

    // Check secondary slave
    if (atapi_cdrom_exists(ATA_SECONDARY_BASE, ATA_SLAVE)) return ATAPI_SECONDARY_SLAVE;

    // Check secondary master
    if (atapi_cdrom_exists(ATA_SECONDARY_BASE, ATA_MASTER)) return ATAPI_SECONDARY_MASTER;

    return 0; // No ATAPI CD-ROM found
}



static inline void ata_wait_not_bsy(U32 base) {
    U8 s;
    do { s = _inb(base + ATA_COMM_REG); } while (s & STAT_BSY);
}

static inline BOOLEAN ata_wait_drq_ready(U32 base) {
    U8 s;
    do {
        s = _inb(base + ATA_COMM_REG);
        if (s & STAT_ERR) return 0;
    } while (!(s & STAT_DRQ));
    return 1;
}

int read_cdrom(U32 atapiWhere, U32 lba, U32 sectors, U16 *buffer) {
    U16 port = 0;
    BOOLEAN slave = 0;
    switch(atapiWhere){
        case ATAPI_PRIMARY_MASTER:
            port = ATA_PRIMARY_BASE;
            break;
        case ATAPI_PRIMARY_SLAVE:
            port = ATA_PRIMARY_BASE;
            slave = 1;
            break;
        case ATAPI_SECONDARY_MASTER:
            port = ATA_SECONDARY_BASE;
            break;
        case ATAPI_SECONDARY_SLAVE:
            port = ATA_SECONDARY_BASE;
            slave = 1;
            break;
    }

        // The command
	volatile U8 read_cmd[12] = {0xA8, 0,
	                                 (lba >> 0x18) & 0xFF, (lba >> 0x10) & 0xFF, (lba >> 0x08) & 0xFF,
	                                 (lba >> 0x00) & 0xFF,
	                                 (sectors >> 0x18) & 0xFF, (sectors >> 0x10) & 0xFF, (sectors >> 0x08) & 0xFF,
	                                 (sectors >> 0x00) & 0xFF,
	                                 0, 0};

	_outb(port + ATA_DRIVE_HEAD, 0xA0 & (slave << 4)); // Drive select
	ata_io_wait(port);
	_outb(port + ATA_ERR, 0x00);
	_outb(port + ATA_LBA_MID, 2048 & 0xFF);
	_outb(port + ATA_LBA_HI, 2048 >> 8);
	_outb(port + ATA_COMM_REG, 0xA0); // Packet command
	ata_io_wait(port); // I think we might need this delay, not sure, so keep this
 
        // Wait for status
	while (1) {
		U8 status = _inb(port + ATA_COMM_REG);
		if ((status & 0x01) == 1)
			return 1;
		if (!(status & 0x80) && (status & 0x08))
			break;
		ata_io_wait(port);
	}

        // Send command
	_outsw(port + ATA_DATA, (U16 *) read_cmd, 6);

        // Read words
	for (U32 i = 0; i < sectors; i++) {
                // Wait until ready
		while (1) {
			U8 status = _inb(port + ATA_COMM_REG);
			if (status & 0x01)
				return 1;
			if (!(status & 0x80) && (status & 0x08))
				break;
		}

		int size = _inb(port + ATA_LBA_HI) << 8
		           | _inb(port + ATA_LBA_MID); // Get the size of transfer

		_insw(port + ATA_DATA, (U16 *) ((U8 *) buffer + i * 0x800), size / 2); // Read it
	}

	return ATAPI_SUCCESS;
}

U32 READ_CDROM(U32 atapiWhere, U32 lba, U32 sectors, U8 *buf) {
    // return ATAPI_READ_SECTOR(atapiWhere, lba, sectors, buf);
    return read_cdrom(atapiWhere, lba, sectors, (U16 *) buf);
}


#ifndef KERNEL_ENTRY
#endif // KERNEL_ENTRY
