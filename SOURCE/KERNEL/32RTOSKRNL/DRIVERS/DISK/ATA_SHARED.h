#ifndef ATA_DRIVER_SHARED_H
#define ATA_DRIVER_SHARED_H
#include "../../../../STD/ATOSMINDEF.h"
#include "../../../../STD/ASM.h"

#define ATA_DATA  0
#define ATA_ERR  1
#define ATA_SECCOUNT  2
#define ATA_LBA_LO  3
#define ATA_LBA_MID  4
#define ATA_LBA_HI  5
#define ATA_DRIVE_HEAD  6
#define ATA_COMM_REGSTAT  7

#define ATA_ALTERNATE_STATUS  0 // Alternate status

#define ATA_PRIMARY_BASE                    0x1F0 // Base port for ATA
#define ATA_PRIMARY_DATA            ATA_PRIMARY_BASE + ATA_DATA// Primary ATA port
#define ATA_PRIMARY_ERR             ATA_PRIMARY_BASE + ATA_ERR // Error register
#define ATA_PRIMARY_SECCOUNT        ATA_PRIMARY_BASE + ATA_SECCOUNT  // Sector count register
#define ATA_PRIMARY_LBA_LO          ATA_PRIMARY_BASE + ATA_LBA_LO // LBA low register
#define ATA_PRIMARY_LBA_MID         ATA_PRIMARY_BASE + ATA_LBA_MID // LBA mid register
#define ATA_PRIMARY_LBA_HI          ATA_PRIMARY_BASE + ATA_LBA_HI // LBA high register
#define ATA_PRIMARY_DRIVE_HEAD      ATA_PRIMARY_BASE + ATA_DRIVE_HEAD // Drive/Head register
#define ATA_PRIMARY_COMM_REGSTAT    ATA_PRIMARY_BASE + ATA_COMM_REGSTAT // Status register
#define ATA_PRIMARY_ALTSTAT_DCR     0x3F6 // Alternate status register

#define ATA_SECONDARY_BASE            0x170 // Base port for secondary ATA
#define ATA_SECONDARY_DATA          ATA_SECONDARY_BASE + ATA_DATA// Secondary ATA port
#define ATA_SECONDARY_ERR           ATA_SECONDARY_BASE + ATA_ERR // Error register
#define ATA_SECONDARY_SECCOUNT      ATA_SECONDARY_BASE + ATA_SECCOUNT  // Sector count register
#define ATA_SECONDARY_LBA_LO        ATA_SECONDARY_BASE + ATA_LBA_LO // LBA low register
#define ATA_SECONDARY_LBA_MID       ATA_SECONDARY_BASE + ATA_LBA_MID // LBA mid register
#define ATA_SECONDARY_LBA_HI        ATA_SECONDARY_BASE + ATA_LBA_HI // LBA high register
#define ATA_SECONDARY_DRIVE_HEAD    ATA_SECONDARY_BASE + ATA_DRIVE_HEAD // Drive/Head register
#define ATA_SECONDARY_COMM_REGSTAT  ATA_SECONDARY_BASE + ATA_COMM_REGSTAT // Status register
#define ATA_SECONDARY_ALTSTAT_DCR   0x376 // Alternate status register

#define ATA_CONTROL_REG             0x206 // Control register for ATA

// -------------------------
//  Commands
// -------------------------
#define ATAPI_PACKET_CMD     0xA0
#define ATAPI_IDENTIFY_CMD   0xA1
#define ATA_IDENTIFY_CMD         0xEC // Command to identify drive
#define ATA_EXECUTE_DIAGNOSTIC   0x90 // Execute diagnostic command
#define ATAPI_CMD_READ12 0xA8

// -------------------------
//  Status register bits
// -------------------------
#define STAT_ERR            1 << 0 // Error bit			(0x01)
#define STAT_DRQ            1 << 3 // Data request bit  (0x08)
#define STAT_SRV            1 << 4 // Service bit       (0x10)
#define STAT_DF             1 << 5 // Device fault bit  (0x20)
#define STAT_RDY            1 << 6 // Drive ready bit   (0x40)
#define STAT_BSY            1 << 7 // Busy bit          (0x80)	


#define ATA_MASTER  0xA0 // Master drive
#define ATA_SLAVE   0xB0 // Slave drive


static inline void ata_io_wait(const U8 p) {
	U8 val = 0;
	inb(p + ATA_CONTROL_REG + ATA_ALTERNATE_STATUS, val);
	inb(p + ATA_CONTROL_REG + ATA_ALTERNATE_STATUS, val);
	inb(p + ATA_CONTROL_REG + ATA_ALTERNATE_STATUS, val);
	inb(p + ATA_CONTROL_REG + ATA_ALTERNATE_STATUS, val);
}

#endif // ATA_DRIVER_SHARED_H