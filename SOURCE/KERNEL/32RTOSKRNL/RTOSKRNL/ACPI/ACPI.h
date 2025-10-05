#ifndef ACPI_H
#define ACPI_H

#include <STD/TYPEDEF.h>
#include <STD/MEM.h>
#include <STD/ASM.h>

/* ACPI Table Signatures */
#define ACPI_SIG_RSDP "RSD PTR "
#define ACPI_SIG_FADT "FACP"

/* Root System Description Pointer (RSDP) */
typedef struct {
    char Signature[8];      // "RSD PTR "
    U8   Checksum;
    char OEMID[6];
    U8   Revision;
    U32  RsdtAddress;       // 32-bit RSDT pointer
    U32  Length;            // ACPI 2.0+
    U32  XsdtAddressLow;       // 64-bit XSDT pointer
    U32  XsdtAddressHigh;      // 64-bit XSDT pointer
    U8   ExtendedChecksum;
    U8   Reserved[3];
} 
__attribute__((packed))
ACPI_RSDP;

/* Standard ACPI Table Header */
typedef struct {
    char Signature[4];
    U32  Length;
    U8   Revision;
    U8   Checksum;
    char OEMID[6];
    char OEMTableID[8];
    U32  OEMRevision;
    U32  CreatorID;
    U32  CreatorRevision;
} 
__attribute__((packed))
ACPI_SDT_HEADER;

typedef struct {
    ACPI_SDT_HEADER Header;
    U64 Entries[]; // 64-bit physical addresses of tables
} 
__attribute__((packed))
ACPI_XSDT;

/* RSDT */
typedef struct {
    ACPI_SDT_HEADER Header;
    U32 Entries[]; // 32-bit physical addresses
} 
__attribute__((packed))
ACPI_RSDT;

/* FADT (Fixed ACPI Description Table) */
typedef struct {
    ACPI_SDT_HEADER Header;

    U32 FirmwareCtrl;
    U32 Dsdt;

    U8 Reserved;

    U8 PreferredPowerManagementProfile;
    U16 SciInterrupt;
    U32 SmiCommandPort;
    U8 AcpiEnable;
    U8 AcpiDisable;
    U8 S4BiosRequest;
    U8 PstateControl;
    U32 Pm1aEvtBlk;
    U32 Pm1bEvtBlk;
    U32 Pm1aCntBlk;
    U32 Pm1bCntBlk;
    U32 Pm2CntBlk;
    U32 PmTmrBlk;
    U32 Gpe0Blk;
    U32 Gpe1Blk;
    U8  Pm1EvtLen;
    U8  Pm1CntLen;
    U8  Pm2CntLen;
    U8  PmTmrLen;
    U8  Gpe0BlkLen;
    U8  Gpe1BlkLen;
    U8  Gpe1Base;
    U8  CstCnt;
    U16 PLvl2Lat;
    U16 PLvl3Lat;
    U16 FlushSize;
    U16 FlushStride;
    U8  DutyOffset;
    U8  DutyWidth;
    U8  DayAlarm;
    U8  MonthAlarm;
    U8  Century;

    U16 IaPcBootArch;
    U8  Reserved2;
    U32 Flags;

    // ACPI 2.0+
    U32 ResetReg;
    U8  ResetValue;
    U8  Reserved3[3];

    U32 X_FirmwareCtrl_1;
    U32 X_FirmwareCtrl_2;
    U32 X_DSDT_1;
    U32 X_DSDT_2;

    U32 X_Pm1aEvtBlk;
    U32 X_Pm1bEvtBlk;
    U32 X_Pm1aCntBlk;
    U32 X_Pm1bCntBlk;
    U32 X_Pm2CntBlk;
    U32 X_PmTmrBlk;
    U32 X_Gpe0Blk;
    U32 X_Gpe1Blk;

    U8 S5SleepTypeA;
    U8 S5SleepTypeB;
} 
__attribute__((packed))
ACPI_FADT;

void ACPI_SHUTDOWN_SYSTEM(void);

#endif // ACPI_H
