#ifndef ATA_SHARED_H
#define ATA_SHARED_H

#include "../../../STD/TYPEDEF.h"
#include "../../../STD/ASM.h"

#define ATA_DATA  			0x0
#define ATA_ERR  			0x1
#define ATA_SECCOUNT  		0x2
#define ATA_LBA_LO  		0x3
#define ATA_LBA_MID  		0x4
#define ATA_LBA_HI  		0x5
#define ATA_DRIVE_HEAD  	0x6
#define ATA_COMM_REG  		0x7
#define ATA_CTRL_REG 		0x0E


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

#define ATA_CTRL_NO_IRQ           	0x02 // Disable Interrupts (nIEN = 1)
#define ATA_CTRL_SOFT_RESET       	0x04 // Software Reset

#define ATA_CMD_IDENTIFY          	0xEC
#define ATAPI_CMD_IDENTIFY        	0xA1
#define ATA_PIO_CMD_READ28			0x20
#define ATA_PIO_CMD_WRITE28	  		0x30


#define ATA_MDA_CMD_READ28               0xC8
#define ATA_MDA_CMD_WRITE28              0xCA
#define ATA_MDA_CMD_IDENTIFY            0xEC


// -------------------------
//  Status register bits
// -------------------------
#define STAT_ERR            1 << 0 // Error bit			(0x01)
#define STAT_DRQ            1 << 3 // Data request bit  (0x08)
#define STAT_SRV            1 << 4 // Service bit       (0x10)
#define STAT_DF             1 << 5 // Device fault bit  (0x20)
#define STAT_RDY            1 << 6 // Drive ready bit   (0x40)
#define STAT_BSY            1 << 7 // Busy bit          (0x80)	

#define ATA_FAILED                0
#define ATA_SUCCESS               1

#ifndef ATA_MASTER
#define ATA_MASTER                  0xA0
#endif

#ifndef ATA_SLAVE
#define ATA_SLAVE                   0xB0
#endif

static inline void ata_io_wait(const U8 p) {
	_inb(p + ATA_CONTROL_REG + ATA_ALTERNATE_STATUS);
	_inb(p + ATA_CONTROL_REG + ATA_ALTERNATE_STATUS);
	_inb(p + ATA_CONTROL_REG + ATA_ALTERNATE_STATUS);
	_inb(p + ATA_CONTROL_REG + ATA_ALTERNATE_STATUS);
}

#endif // ATA_SHARED_H