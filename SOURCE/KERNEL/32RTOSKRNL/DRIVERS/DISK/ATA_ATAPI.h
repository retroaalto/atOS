#ifndef ATA_ATAPI_DRIVER_H
#define ATA_ATAPI_DRIVER_H

#include "../../../../STD/ATOSMINDEF.h"

#define ATA_DATA  0
#define ATA_ERR  1
#define ATA_SECCOUNT  2
#define ATA_LBA_LO  3
#define ATA_LBA_MID  4
#define ATA_LBA_HI  5
#define ATA_DRIVE_HEAD  6
#define ATA_COMM_REG  7


#define ATA_PRIMARY_BASE                    0x1F0 // Base port for ATA
#define ATA_PRIMARY_DATA            ATA_PRIMARY_BASE + ATA_DATA// Primary ATA port
#define ATA_PRIMARY_ERR             ATA_PRIMARY_BASE + ATA_ERR // Error register
#define ATA_PRIMARY_SECCOUNT        ATA_PRIMARY_BASE + ATA_SECCOUNT  // Sector count register
#define ATA_PRIMARY_LBA_LO          ATA_PRIMARY_BASE + ATA_LBA_LO // LBA low register
#define ATA_PRIMARY_LBA_MID         ATA_PRIMARY_BASE + ATA_LBA_MID // LBA mid register
#define ATA_PRIMARY_LBA_HI          ATA_PRIMARY_BASE + ATA_LBA_HI // LBA high register
#define ATA_PRIMARY_DRIVE_HEAD      ATA_PRIMARY_BASE + ATA_DRIVE_HEAD // Drive/Head register
#define ATA_PRIMARY_COMM_REGSTAT    ATA_PRIMARY_BASE + ATA_COMM_REG // Status register
#define ATA_PRIMARY_ALTSTAT_DCR     0x3F6 // Alternate status register

#define ATA_SECONDARY_BASE            0x170 // Base port for secondary ATA
#define ATA_SECONDARY_DATA          ATA_SECONDARY_BASE + ATA_DATA// Secondary ATA port
#define ATA_SECONDARY_ERR           ATA_SECONDARY_BASE + ATA_ERR // Error register
#define ATA_SECONDARY_SECCOUNT      ATA_SECONDARY_BASE + ATA_SECCOUNT  // Sector count register
#define ATA_SECONDARY_LBA_LO        ATA_SECONDARY_BASE + ATA_LBA_LO // LBA low register
#define ATA_SECONDARY_LBA_MID       ATA_SECONDARY_BASE + ATA_LBA_MID // LBA mid register
#define ATA_SECONDARY_LBA_HI        ATA_SECONDARY_BASE + ATA_LBA_HI // LBA high register
#define ATA_SECONDARY_DRIVE_HEAD    ATA_SECONDARY_BASE + ATA_DRIVE_HEAD // Drive/Head register
#define ATA_SECONDARY_COMM_REGSTAT  ATA_SECONDARY_BASE + ATA_COMM_REG // Status register
#define ATA_SECONDARY_ALTSTAT_DCR   0x376 // Alternate status register

#define ATA_CONTROL_REG             0x206 // Control register for ATA
#define ATA_ALTERNATE_STATUS  0 // Alternate status

// -------------------------
//  Commands
// -------------------------
#define ATAPI_CMD_TEST_UNIT_READY 	0x00
#define ATAPI_CMD_REQUEST_SENSE 	0x03
#define ATAPI_CMD_FORMAT_UNIT 	0x04
#define ATAPI_CMD_INQUIRY 	0x12
#define ATAPI_CMD_START_STOP_UNIT_EJECT_DEVICE 	0x1B
#define ATAPI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL 	0x1E
#define ATAPI_CMD_READ_FORMAT_CAPACITIES 	0x23
#define ATAPI_CMD_READ_CAPACITY 	0x25
#define ATAPI_CMD_READ10 	0x28
#define ATAPI_CMD_WRITE10 	0x2A
#define ATAPI_CMD_SEEKK10 	0x2B
#define ATAPI_CMD_WRITE_AND_VERIFY10 	0x2E
#define ATAPI_CMD_VERIFY10 	0x2F
#define ATAPI_CMD_SYNCHRONIZE_CACHE 	0x35
#define ATAPI_CMD_WRITE_BUFFER 	0x3B
#define ATAPI_CMD_READ_BUFFER 	0x3C
#define ATAPI_CMD_READ_TOC_PMA_ATIP 	0x43
#define ATAPI_CMD_GET_CONFIGURATION 	0x46
#define ATAPI_CMD_GET_EVENT_STATUS_NOTIFICATION 	0x4A
#define ATAPI_CMD_READ_DISC_INFORMATION 	0x51
#define ATAPI_CMD_READ_TRACK_INFORMATION 	0x52
#define ATAPI_CMD_RESERVE_TRACK 	0x53
#define ATAPI_CMD_SEND_OPC_INFORMATION 	0x54
#define ATAPI_CMD_MODE_SELECT_10 	0x55
#define ATAPI_CMD_REPAIR_TRACK 	0x58
#define ATAPI_CMD_MODE_SENSE10 	0x5A
#define ATAPI_CMD_CLOSE_TRACK_SESSION 	0x5B
#define ATAPI_CMD_READ_BUFFER_CAPACITY 	0x5C
#define ATAPI_CMD_SEND_CUE_SHEET 	0x5D
#define ATAPI_CMD_REPORT_LUNS 	0xA0
#define ATAPI_CMD_SEND_PACKET 0xA0
#define ATAPI_CMD_BLANK 	0xA1
#define ATAPI_CMD_IDENTIFY 0xA1
#define ATAPI_CMD_SECURITY_PROTOCOL_IN 	0xA2
#define ATAPI_CMD_SEND_KEY 	0xA3
#define ATAPI_CMD_REPORT_KEY 	0xA4
#define ATAPI_CMD_LOAD_UNLOAD_MEDIUM 	0xA6
#define ATAPI_CMD_SET_READ_AHEAD 	0xA7
#define ATAPI_CMD_READ12 	0xA8
#define ATAPI_CMD_WRITE12 	0xAA
#define ATAPI_CMD_READ_MEDIA_SERIAL_NUMBER12 	0xAB
#define ATAPI_CMD_GET_PERFORMANCE 	0xAC
#define ATAPI_CMD_READ_DISC_STRUCTURE 	0xAD
#define ATAPI_CMD_SECURITY_PROTOCOL_OUT 	0xB5
#define ATAPI_CMD_SET_STREAMING 	0xB6
#define ATAPI_CMD_READ_CD_MSF 	0xB9
#define ATAPI_CMD_SET_CD_SPEED 	0xBB
#define ATAPI_CMD_MECHANISM_STATUS 	0xBD
#define ATAPI_CMD_READ_CD 	0xBE
#define ATAPI_CMD_SEND_DISC_STRUCTURE 	0xBF

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



#define ATAPI_SECTOR_SIZE 2048
#define ATAPI_SECTORS(num) (ATAPI_SECTOR_SIZE * (num))

#define ATAPI_FAILED                0
#define ATAPI_SUCCESS               1
#define ATAPI_PRIMARY_MASTER        'A'//1
#define ATAPI_PRIMARY_SLAVE         'B'//2
#define ATAPI_SECONDARY_MASTER      'C'//3
#define ATAPI_SECONDARY_SLAVE       'D'//4
/*+++
U32 ATAPI_CHECK(U0);

DESCRIPTION
    Checks which ATAPI CD-ROM drives are present.
    Save the results as it will be used later if you need to access data from ATAPI
    
RETURN
    ATAPI_FAILED
    ATAPI_PRIMARY_MASTER
    ATAPI_PRIMARY_SLAVE
    ATAPI_SECONDARY_MASTER
    ATAPI_SECONDARY_SLAVE

AUTHORS
    Antonako1

REVISION HISTORY
    2025/08/27 - Antonako1
        Initial version.
---*/
U32 ATAPI_CHECK(U0);

/*+++
U32 READ_CDROM(U32 ATAPICheckRes, U32 lba, U32 sectors, U8 *buf);

DESCRIPTION
    Reads sectors from a CD-ROM drive.

PARAMETERS
    ATAPICheckRes - The result of the ATAPI check to identify the drive.
    lba - The logical block address to start reading from.
    sectors - The number of sectors to read.
    buf - The buffer to store the read data.

RETURN
    ATAPI_FAILED - Failed to read from the CD-ROM.
    ATAPI_SUCCESS - Successfully read from the CD-ROM.

AUTHORS
    Antonako1

REVISION HISTORY
    2025/08/28 - Antonako1
        Initial version.

Remarks
    Buffer must be at least ATAPI_SECTOR_SIZE size in bytes
---*/
U32 READ_CDROM(U32 ATAPICheckRes, U32 lba, U32 sectors, U8 *buf);


#ifndef KERNEL_ENTRY
#endif // KERNEL_ENTRY
#endif // ATA_ATAPI_DRIVER_H