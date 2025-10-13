#ifndef ATA_PIIX3_DRIVER_H
#define ATA_PIIX3_DRIVER_H
#include <STD/TYPEDEF.h>
#include <DRIVERS/ATA_SHARED.h>
#include <DRIVERS/ATA_PIO/ATA_PIO.h>
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

// BM_STATUS bits
#define BM_STATUS_STOP   0x0
#define BM_STATUS_ACTIVE 0x01  // 1 == DMA active
#define BM_STATUS_ERROR  0x02
#define BM_STATUS_IRQ    0x04  // bit set when device triggered IRQ; write-1 to clear
#define BM_STATUS_ACK    0x06

// Public API
BOOLEAN ATA_PIIX3_INIT(VOID);

/*
Return values as follows
#define ATA_PRIMARY_MASTER          'E'
#define ATA_PRIMARY_SLAVE           'F'
#define ATA_SECONDARY_MASTER        'G'
#define ATA_SECONDARY_SLAVE         'H'
*/
U32 ATA_GET_IDENTIFIER(VOID);
U32 ATA_IDENTIFY(VOID);

BOOLEAN ATA_PIIX3_READ_SECTORS(U8 device_id, U32 lba, U8 sector_count, VOIDPTR out_buffer);
BOOLEAN ATA_PIIX3_WRITE_SECTORS(U8 device_id, U32 lba, U8 sector_count, VOIDPTR in_buffer);

#endif // ATA_PIIX3_DRIVER_H
