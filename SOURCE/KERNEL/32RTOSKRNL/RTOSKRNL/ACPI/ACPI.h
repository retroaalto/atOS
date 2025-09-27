#ifndef ACPI_H
#define ACPI_H

#include <STD/TYPEDEF.h>

typedef struct {
    U8 Signature[8];
    U8 Checksum;
    U8 OEMId[6];
    U8 Revision;
    U32 RSDTAddress;
    U32 Length;
    U32 XSTDAddressLow;
    U32 XSTDAddressHigh;
    U8 ExtendedChecksum;
    U8 Reserved[3];
} __attribute__((packed)) RSDP2;

typedef struct {
    U8 Signature[4];
    U32 Length;
    U8 Revision;
    U8 Checksum;
    U8 OEMID[6];
    U8 OEMTableID[8];
    U32 OEMRevision;
    U32 CreatorID;
    U32 CreatorRevision;
}__attribute__((packed)) SDTHeader;

typedef struct {
    SDTHeader Header;
    U32 Reserved1;
    U32 Reserved2;
}__attribute__((packed)) MCFGHeader;

typedef struct {
    U32 BaseAddressLow;
    U32 BaseAddressHigh;
    U16 PCISegGroup;
    U8 StartBus;
    U8 EndBus;
    U32 Reserved;
}__attribute__((packed)) DeviceConfig;

VOID *FIND_TABLE(SDTHeader *header, U8 *signature);

#endif // ACPI_H