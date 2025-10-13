// ATA_PIIX3.h
#ifndef ATA_PIIX3_DRIVER_H
#define ATA_PIIX3_DRIVER_H

#include <STD/TYPEDEF.h>
#include <DRIVERS/ATA_SHARED.h>
#include <DRIVERS/PCI/PCI.h> // Include PCI interface

// DMA flags and limits
#define END_OF_TABLE_FLAG           0x8000
#define ATA_PIIX3_SECTOR_SIZE       512
#define ATA_PIIX3_MAX_SECTORS       128
#define ATA_PIIX3_BUFFER_ALIGN      0x1000
#define ATA_PIIX3_PRDT_ALIGN        0x1000
#define BUS_MASTER_LIMIT_PER_ENTRY  (ATA_PIIX3_SECTOR_SIZE * ATA_PIIX3_MAX_SECTORS) // 64 KB per PRDT entry

// Bus Master IDE offsets
#define BM_COMMAND_OFFSET           0x00
#define BM_STATUS_OFFSET            0x02
#define BM_PRDT_ADDR_OFFSET         0x04

// BM_COMMAND bits
#define BM_CMD_START_STOP           0x01
#define BM_CMD_READ                 0x08
#define BM_CMD_WRITE                0x00

// ATA Commands (for registers)
#define BM_CMD_READ28               0xC8
#define BM_CMD_WRITE28              0xCA

// BM_STATUS bits
#define BM_STATUS_INT               0x04
#define BM_STATUS_ERR               0x02
#define BM_STATUS_RESET             0x04  // Writing 1 clears interrupt/status

// ATA drive types
#define ATA_MASTER                  0xA0
#define ATA_SLAVE                   0xB0

// ATA Commands
#define ATA_CMD_IDENTIFY            0xEC

#define ATA_PRIMARY_MASTER          'E'
#define ATA_PRIMARY_SLAVE           'F'
#define ATA_SECONDARY_MASTER        'G'
#define ATA_SECONDARY_SLAVE         'H'

typedef struct {
    U32 phys_addr;
    U16 byte_count;
    U16 flags; // bit 15 = end-of-table
} ATTRIB_PACKED PRDT_ENTRY;

// Public API
BOOLEAN ATA_PIIX3_INIT(VOID);
U32 ATA_IDENTIFY(VOID);
U32 ATA_GET_IDENTIFIER(VOID);
U16* ATA_PIIX3_GET_DRIVE_IDENTIFY_INFO(U32 DEVICE_ID);
BOOLEAN ATA_PIIX3_READ_SECTORS(U8 device_id, U32 lba, U8 sector_count, VOIDPTR out_buffer);
BOOLEAN ATA_PIIX3_WRITE_SECTORS(U8 device_id, U32 lba, U8 sector_count, VOIDPTR in_buffer);

#endif // ATA_PIIX3_DRIVER_H
