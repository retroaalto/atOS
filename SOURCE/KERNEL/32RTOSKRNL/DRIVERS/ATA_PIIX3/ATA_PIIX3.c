// ATA_PIIX3.c
#include <DRIVERS/ATA_PIIX3/ATA_PIIX3.h>
#include <MEMORY/HEAP/KHEAP.h>
#include <RTOSKRNL_INTERNAL.h>
#include <STD/ASM.h>
#include <STD/MEM.h>
#include <STD/STRING.h>
#include <DRIVERS/VIDEO/VBE.h>

static PRDT_ENTRY* PRDT ATTRIB_DATA = NULL;
static U8* DMA_BUFFER ATTRIB_DATA = NULL;
static U32 BM_BASE_PRIMARY ATTRIB_DATA = 0;
static U32 BM_BASE_SECONDARY ATTRIB_DATA = 0;
static U32 IDENTIFIER ATTRIB_DATA = 0;

static BOOL BM_PRIMARY_IS_IO ATTRIB_DATA = FALSE;
static BOOL BM_SECONDARY_IS_IO ATTRIB_DATA = FALSE;

static inline void bm_write8(U32 bm_base, BOOL is_io, U32 offset, U8 value) {
    if (is_io) _outb((U16)(bm_base + offset), value);
    else        *(volatile U8*)(bm_base + offset) = value;
}

static inline U8 bm_read8(U32 bm_base, BOOL is_io, U32 offset) {
    if (is_io) return _inb((U16)(bm_base + offset));
    else        return *(volatile U8*)(bm_base + offset);
}

static inline void bm_write32(U32 bm_base, BOOL is_io, U32 offset, U32 value) {
    if (is_io) _outl((U16)(bm_base + offset), value);
    else        *(volatile U32*)(bm_base + offset) = value;
}

static void ATA_PIIX3_LOCATE_BUS_MASTER(void) {
    U32 pci_device_count = PCI_GET_DEVICE_COUNT();
    BM_BASE_PRIMARY = 0;
    BM_BASE_SECONDARY = 0;

    for (U32 i = 0; i < pci_device_count; i++) {
        PCI_DEVICE_ENTRY* dev = PCI_GET_DEVICE(i);
        if (dev->header.class_code == PCI_CLASS_MASS_STORAGE &&
            dev->header.subclass  == PCI_SUBCLASS_IDE) {

            BM_BASE_PRIMARY   = dev->header.bar4 & 0xFFFFFFFC;
            BM_PRIMARY_IS_IO  = dev->header.bar4 & 0x01;
            BM_BASE_SECONDARY = dev->header.bar5 & 0xFFFFFFFC;
            BM_SECONDARY_IS_IO = dev->header.bar5 & 0x01;

            // Enable Bus Mastering for PIIX3
            U16 cmd = PCI_GET_COMMAND(dev->bus, dev->slot, dev->func);
            PCI_WRITE16(dev->bus, dev->slot, dev->func, PCI_COMMAND_OFFSET, cmd | 0x04);
        }
    }
}

// ATA drive identify
static int ata_read_identify_to_buffer(U16 base_port, U8 drive, U16 *ident_out) {
    _outb(base_port + ATA_DRIVE_HEAD, drive);
    ata_io_wait(base_port);
    _outb(base_port + ATA_COMM_REG, ATA_CMD_IDENTIFY);
    ata_io_wait(base_port);

    U8 status = _inb(base_port + ATA_COMM_REG);
    if (status == 0) return 0;

    U32 timeout = 1000000;
    while ((_inb(base_port + ATA_COMM_REG) & STAT_BSY) && timeout--) {}
    status = _inb(base_port + ATA_COMM_REG);
    if (!(status & STAT_ERR)) {
        for (int i = 0; i < 256; i++) ident_out[i] = _inw(base_port + ATA_DATA);
        return 1;
    }

    // ATAPI fallback
    _outb(base_port + ATA_ERR, 0);
    ata_io_wait(base_port);
    _outb(base_port + ATA_COMM_REG, ATAPI_CMD_IDENTIFY);
    ata_io_wait(base_port);

    status = _inb(base_port + ATA_COMM_REG);
    if (status == 0) return 0;

    timeout = 1000000;
    while ((_inb(base_port + ATA_COMM_REG) & STAT_BSY) && timeout--) {}
    status = _inb(base_port + ATA_COMM_REG);
    if (!(status & STAT_ERR) && (status & STAT_DRQ)) {
        for (int i = 0; i < 256; i++) ident_out[i] = _inw(base_port + ATA_DATA);
        return 2;
    }

    return 0;
}

BOOLEAN ATA_DRIVE_EXISTS(U16 base_port, U8 drive) {
    U16 ident[256];
    return ata_read_identify_to_buffer(base_port, drive, ident) != 0;
}

// Identify first connected drive
U32 ATA_IDENTIFY(void) {
    if (BM_BASE_PRIMARY) {
        if (ATA_DRIVE_EXISTS(BM_BASE_PRIMARY, ATA_MASTER))   return IDENTIFIER = ATA_PRIMARY_MASTER;
        if (ATA_DRIVE_EXISTS(BM_BASE_PRIMARY, ATA_SLAVE))    return IDENTIFIER = ATA_PRIMARY_SLAVE;
    }

    if (BM_BASE_SECONDARY) {
        if (ATA_DRIVE_EXISTS(BM_BASE_SECONDARY, ATA_MASTER)) return IDENTIFIER = ATA_SECONDARY_MASTER;
        if (ATA_DRIVE_EXISTS(BM_BASE_SECONDARY, ATA_SLAVE))  return IDENTIFIER = ATA_SECONDARY_SLAVE;
    }

    return IDENTIFIER = ATA_FAILED;
}

U32 ATA_GET_IDENTIFIER(VOID) {
    if(!IDENTIFIER) return ATA_IDENTIFY();
    return IDENTIFIER;
}

// DMA buffer info
U16* ATA_PIIX3_GET_DRIVE_IDENTIFY_INFO(U32 DEVICE_ID) {
    U16* ident = (U16*)KMALLOC(512);
    if(!ident) return NULL;
    MEMZERO(ident, 512);

    int res = 0;
    switch(DEVICE_ID) {
        case ATA_PRIMARY_MASTER:   res = ata_read_identify_to_buffer(ATA_PRIMARY_BASE, ATA_MASTER, ident); break;
        case ATA_PRIMARY_SLAVE:    res = ata_read_identify_to_buffer(ATA_PRIMARY_BASE, ATA_SLAVE, ident); break;
        case ATA_SECONDARY_MASTER: res = ata_read_identify_to_buffer(ATA_SECONDARY_BASE, ATA_MASTER, ident); break;
        case ATA_SECONDARY_SLAVE:  res = ata_read_identify_to_buffer(ATA_SECONDARY_BASE, ATA_SLAVE, ident); break;
        default: KFREE(ident); return NULL;
    }
    if(res == 0) { KFREE(ident); return NULL; }
    return ident;
}

// Initialize DMA (polling only)
BOOLEAN ATA_PIIX3_INIT(VOID) {
    if (PRDT) return TRUE;

    ATA_PIIX3_LOCATE_BUS_MASTER();
    U32 identification = ATA_GET_IDENTIFIER();
    if (identification == ATA_FAILED) return FALSE;

    // Allocate PRDT + DMA buffer
    PRDT = (PRDT_ENTRY*)KMALLOC_ALIGN(sizeof(PRDT_ENTRY), ATA_PIIX3_PRDT_ALIGN);
    panic_if(!PRDT, PANIC_TEXT("Failed to allocate PRDT"), PANIC_OUT_OF_MEMORY);
    MEMZERO(PRDT, sizeof(PRDT_ENTRY));

    DMA_BUFFER = KMALLOC_ALIGN(BUS_MASTER_LIMIT_PER_ENTRY, ATA_PIIX3_BUFFER_ALIGN);
    panic_if(!DMA_BUFFER, PANIC_TEXT("Failed to allocate DMA buffer"), PANIC_OUT_OF_MEMORY);
    MEMZERO(DMA_BUFFER, BUS_MASTER_LIMIT_PER_ENTRY);

    // Clear BM command + status
    if (BM_BASE_PRIMARY) {
        bm_write8(BM_BASE_PRIMARY, BM_PRIMARY_IS_IO, BM_COMMAND_OFFSET, 0);
        bm_write8(BM_BASE_PRIMARY, BM_PRIMARY_IS_IO, BM_STATUS_OFFSET, BM_STATUS_RESET);
    }
    if (BM_BASE_SECONDARY) {
        bm_write8(BM_BASE_SECONDARY, BM_SECONDARY_IS_IO, BM_COMMAND_OFFSET, 0);
        bm_write8(BM_BASE_SECONDARY, BM_SECONDARY_IS_IO, BM_STATUS_OFFSET, BM_STATUS_RESET);
    }

    return TRUE;
}

// Generic DMA sector read/write (polling mode)
BOOLEAN ATA_PIIX3_XFER(U8 device, U32 lba, U8 sectors, VOIDPTR buf, BOOLEAN write) {
    if (!PRDT || !DMA_BUFFER) return FALSE;

    U32 total_bytes = sectors * ATA_PIIX3_SECTOR_SIZE;
    U16 base = 0;
    U32 bm_base = 0;
    BOOL is_io = FALSE;

    switch (device) {
        case ATA_PRIMARY_MASTER:
        case ATA_PRIMARY_SLAVE:
            base = ATA_PRIMARY_BASE;
            bm_base = BM_BASE_PRIMARY;
            is_io = BM_PRIMARY_IS_IO;
            break;
        case ATA_SECONDARY_MASTER:
        case ATA_SECONDARY_SLAVE:
            base = ATA_SECONDARY_BASE;
            bm_base = BM_BASE_SECONDARY;
            is_io = BM_SECONDARY_IS_IO;
            break;
        default:
            return FALSE;
    }

    // Reset controller state
    bm_write8(bm_base, is_io, BM_COMMAND_OFFSET, 0);
    bm_write8(bm_base, is_io, BM_STATUS_OFFSET, BM_STATUS_RESET);

    // Setup PRDT + DMA buffer
    MEMZERO(DMA_BUFFER, total_bytes);
    PRDT[0].phys_addr = (U32)DMA_BUFFER;
    PRDT[0].byte_count = total_bytes - 1;
    PRDT[0].flags = END_OF_TABLE_FLAG;
    bm_write32(bm_base, is_io, BM_PRDT_ADDR_OFFSET, (U32)PRDT);

    if (write) MEMCPY_OPT(DMA_BUFFER, buf, total_bytes);

    // Program ATA registers
    _outb(base + ATA_SECCOUNT, sectors);
    _outb(base + ATA_LBA_LO,   (U8)(lba & 0xFF));
    _outb(base + ATA_LBA_MID,  (U8)((lba >> 8) & 0xFF));
    _outb(base + ATA_LBA_HI,   (U8)((lba >> 16) & 0xFF));
    _outb(base + ATA_DRIVE_HEAD, 0xE0 | ((device & 1) ? 0x10 : 0) | ((lba >> 24) & 0x0F));
    ata_io_wait(base);
    _outb(base + ATA_COMM_REG, write ? BM_CMD_WRITE28 : BM_CMD_READ28);

    // Start DMA
    U8 cmd = write ? BM_CMD_WRITE : BM_CMD_READ;
    cmd |= BM_CMD_START_STOP;
    bm_write8(bm_base, is_io, BM_COMMAND_OFFSET, cmd);

    // Polling loop
    U32 timeout = 0xFFFFFF;
    while (timeout--) {
        U8 status = bm_read8(bm_base, is_io, BM_STATUS_OFFSET);
        if (status & BM_STATUS_ERR) {
            return FALSE;
        }
        if (!(status & BM_CMD_START_STOP)) break; // DMA finished
        cpu_relax();
    }

    // Stop DMA
    bm_write8(bm_base, is_io, BM_COMMAND_OFFSET, 0);
    bm_write8(bm_base, is_io, BM_STATUS_OFFSET, BM_STATUS_RESET);

    if (!write) MEMCPY_OPT(buf, DMA_BUFFER, total_bytes);
    return TRUE;
}

BOOLEAN ATA_PIIX3_READ_SECTORS(U8 device, U32 lba, U8 sectors, VOIDPTR out) {
    return ATA_PIIX3_XFER(device, lba, sectors, out, FALSE);
}

BOOLEAN ATA_PIIX3_WRITE_SECTORS(U8 device, U32 lba, U8 sectors, VOIDPTR in) {
    return ATA_PIIX3_XFER(device, lba, sectors, in, TRUE);
}
