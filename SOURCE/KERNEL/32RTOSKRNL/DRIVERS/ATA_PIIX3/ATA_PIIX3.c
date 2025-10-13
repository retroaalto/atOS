#include <DRIVERS/ATA_PIIX3/ATA_PIIX3.h>
#include <MEMORY/HEAP/KHEAP.h>
#include <RTOSKRNL_INTERNAL.h>
#include <STD/ASM.h>
#include <STD/MEM.h>
#include <STD/STRING.h>
#include <DRIVERS/VIDEO/VBE.h>

#define POLLING_TIME 0xFFFFF

typedef struct {
    U32 phys_addr;
    U16 byte_count;
    U16 flags; // bit 15 = end-of-table
} ATTRIB_PACKED PRDT_ENTRY;

static PRDT_ENTRY* PRDT ATTRIB_DATA = NULL;
static U8* DMA_BUFFER ATTRIB_DATA = NULL;
static U32 BM_BASE_PRIMARY ATTRIB_DATA = 0;
static U32 BM_BASE_SECONDARY ATTRIB_DATA = 0;
static U32 IDENTIFIER ATTRIB_DATA = 0;

static BOOL BM_PRIMARY_IS_IO ATTRIB_DATA = FALSE;
static BOOL BM_SECONDARY_IS_IO ATTRIB_DATA = FALSE;

typedef struct {
    U8 MW_DMA_0;
    U8 MW_DMA_1;
    U8 MW_DMA_2;
    U8 UDMA_0;
    U8 UDMA_1;
} MDA_MODES;

static MDA_MODES supported_dma_modes ATTRIB_DATA = { 0, 0, 0, 0, 0 };

static BOOL ATA_PIIX3_LOCATE_BUS_MASTER(void) {
    U32 pci_device_count = PCI_GET_DEVICE_COUNT();
    BM_BASE_PRIMARY = 0;
    BM_BASE_SECONDARY = 0;

    for (U32 i = 0; i < pci_device_count; i++) {
        PCI_DEVICE_ENTRY* dev = PCI_GET_DEVICE(i);
        if (dev->header.class_code == PCI_CLASS_MASS_STORAGE &&
            dev->header.subclass  == PCI_SUBCLASS_IDE) {

            U32 bm_bar = dev->header.bar4;
            BOOL bm_is_io = bm_bar & 0x01;
            U32 bm_base = bm_is_io ? (bm_bar & ~0x3) : (bm_bar & ~0xF);

            BM_BASE_PRIMARY = bm_base + 0x00;
            BM_BASE_SECONDARY = bm_base + 0x08;
            BM_PRIMARY_IS_IO = bm_is_io;
            BM_SECONDARY_IS_IO = bm_is_io;

            if(!PCI_ENABLE_BUS_MASTERING(dev->bus, dev->slot, dev->func)) return FALSE;
        }
    }
    return TRUE;
}


BOOLEAN ATA_DRIVE_EXISTS(U16 base_port, U8 drive) {
    U16 ident[256];
    return ata_read_identify_to_buffer(base_port, drive, ident) != 0;
}

// Identify first connected drive
U32 ATA_IDENTIFY(void) {
    if (BM_BASE_PRIMARY) {
        if (ATA_DRIVE_EXISTS(ATA_PRIMARY_BASE, ATA_MASTER))   return IDENTIFIER = ATA_PRIMARY_MASTER;
        if (ATA_DRIVE_EXISTS(ATA_PRIMARY_BASE, ATA_SLAVE))    return IDENTIFIER = ATA_PRIMARY_SLAVE;
    }

    if (BM_BASE_SECONDARY) {
        if (ATA_DRIVE_EXISTS(ATA_SECONDARY_BASE, ATA_MASTER)) return IDENTIFIER = ATA_SECONDARY_MASTER;
        if (ATA_DRIVE_EXISTS(ATA_SECONDARY_BASE, ATA_SLAVE))  return IDENTIFIER = ATA_SECONDARY_SLAVE;
    }

    return IDENTIFIER = ATA_FAILED;
}

U32 ATA_GET_IDENTIFIER(VOID) {
    if(!IDENTIFIER) return ATA_IDENTIFY();
    return IDENTIFIER;
}

// Initialize DMA (polling only)
BOOLEAN ATA_PIIX3_INIT(VOID) {
    if (PRDT && DMA_BUFFER) return TRUE;

    // Locate the bus master base addresses
    panic_if(!ATA_PIIX3_LOCATE_BUS_MASTER(), PANIC_TEXT("Failed to initialize bus master"), PANIC_INITIALIZATION_FAILED);
    if (!BM_BASE_PRIMARY && !BM_BASE_SECONDARY) return FALSE;

    // Identify first available drive
    U32 identification = ATA_GET_IDENTIFIER();
    if (identification == ATA_FAILED) return FALSE;

    PRDT = (PRDT_ENTRY*)KMALLOC_ALIGN(sizeof(PRDT_ENTRY), ATA_PIIX3_PRDT_ALIGN);
    panic_if(!PRDT, PANIC_TEXT("Failed to allocate PRDT"), PANIC_OUT_OF_MEMORY);
    MEMZERO(PRDT, sizeof(PRDT_ENTRY));

    DMA_BUFFER = KMALLOC_ALIGN(BUS_MASTER_LIMIT_PER_ENTRY, ATA_PIIX3_BUFFER_ALIGN);
    panic_if(!DMA_BUFFER, PANIC_TEXT("Failed to allocate DMA buffer"), PANIC_OUT_OF_MEMORY);
    MEMZERO(DMA_BUFFER, BUS_MASTER_LIMIT_PER_ENTRY);

    // Clear BM command + status registers
    if (BM_BASE_PRIMARY) {
        bm_write8(BM_BASE_PRIMARY, BM_PRIMARY_IS_IO, BM_COMMAND_OFFSET, 0);
        bm_write8(BM_BASE_PRIMARY, BM_PRIMARY_IS_IO, BM_STATUS_OFFSET, BM_STATUS_ACK);
    }
    if (BM_BASE_SECONDARY) {
        bm_write8(BM_BASE_SECONDARY, BM_SECONDARY_IS_IO, BM_COMMAND_OFFSET, 0);
        bm_write8(BM_BASE_SECONDARY, BM_SECONDARY_IS_IO, BM_STATUS_OFFSET, BM_STATUS_ACK);
    }

    // Detect supported DMA modes on connected drives
    U16 ident[256];
    if (identification == ATA_PRIMARY_MASTER || identification == ATA_PRIMARY_SLAVE) {
        if (ata_read_identify_to_buffer(ATA_PRIMARY_BASE, identification & 1, ident)) {
            U16 w63 = ident[63];
            U16 w88 = ident[88];
            supported_dma_modes.MW_DMA_0 = (w63 & (1 << 0)) != 0;
            supported_dma_modes.MW_DMA_1 = (w63 & (1 << 1)) != 0;
            supported_dma_modes.MW_DMA_2 = (w63 & (1 << 2)) != 0;
            supported_dma_modes.UDMA_0    = (w88 & (1 << 0)) != 0;
            supported_dma_modes.UDMA_1    = (w88 & (1 << 1)) != 0;
        }
    }
    if (identification == ATA_SECONDARY_MASTER || identification == ATA_SECONDARY_SLAVE) {
        if (ata_read_identify_to_buffer(ATA_SECONDARY_BASE, identification & 1, ident)) {
            U16 w63 = ident[63];
            U16 w88 = ident[88];
            supported_dma_modes.MW_DMA_0 |= (w63 & (1 << 0)) != 0;
            supported_dma_modes.MW_DMA_1 |= (w63 & (1 << 1)) != 0;
            supported_dma_modes.MW_DMA_2 |= (w63 & (1 << 2)) != 0;
            supported_dma_modes.UDMA_0    |= (w88 & (1 << 0)) != 0;
            supported_dma_modes.UDMA_1    |= (w88 & (1 << 1)) != 0;
        }
    }
    return TRUE;
}


// Generic DMA sector read/write (polling mode)
BOOLEAN ATA_PIIX3_XFER(U8 device, U32 lba, U8 sectors, VOIDPTR buf, BOOLEAN write) {
    if (!PRDT || !DMA_BUFFER) return FALSE;

    U32 total_bytes = sectors * ATA_PIIX3_SECTOR_SIZE;
    if (total_bytes > 0x10000) return FALSE; // PIIX3 max 64 KiB per PRDT entry

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

    // Reset BM controller state
    bm_write8(bm_base, is_io, BM_COMMAND_OFFSET, 0);
    bm_write8(bm_base, is_io, BM_STATUS_OFFSET, BM_STATUS_ERROR | BM_STATUS_IRQ);

    // Setup PRDT + DMA buffer
    MEMZERO(DMA_BUFFER, total_bytes);
    PRDT[0].phys_addr = (U32)DMA_BUFFER;
    PRDT[0].byte_count = total_bytes - 1;
    PRDT[0].flags = END_OF_TABLE_FLAG;
    bm_write32(bm_base, is_io, BM_PRDT_ADDR_OFFSET, (U32)PRDT);

    panic_if(((U32)PRDT) & 0x3, "PRDT not DWORD aligned", PRDT);
    panic_if(((U32)DMA_BUFFER) & 0x3, "DMA buffer not DWORD aligned", DMA_BUFFER);

    if (write) MEMCPY_OPT(DMA_BUFFER, buf, total_bytes);

    // Start DMA engine
    bm_write8(bm_base, is_io, BM_STATUS_OFFSET, BM_STATUS_ERROR | BM_STATUS_IRQ); // clear previous errors/IRQ
    bm_write8(bm_base, is_io, BM_COMMAND_OFFSET, (write ? BM_CMD_WRITE : BM_CMD_READ) | BM_CMD_START_STOP);

    // Program ATA registers for LBA28
    _outb(base + ATA_DRIVE_HEAD, 0xE0 | ((device & 1) ? 0x10 : 0) | ((lba >> 24) & 0x0F));
    ata_io_wait(base);
    _outb(base + ATA_SECCOUNT, sectors);
    _outb(base + ATA_LBA_LO,   (U8)(lba & 0xFF));
    _outb(base + ATA_LBA_MID,  (U8)((lba >> 8) & 0xFF));
    _outb(base + ATA_LBA_HI,   (U8)((lba >> 16) & 0xFF));
    _outb(base + ATA_COMM_REG, write ? ATA_MDA_CMD_WRITE28 : ATA_MDA_CMD_READ28);
    ata_io_wait(base);
        
    // Poll for DMA completion
    U32 timeout = POLLING_TIME;
    while (timeout--) {
        U8 bm_stat  = bm_read8(bm_base, is_io, BM_STATUS_OFFSET);
        U8 ata_stat = _inb(base + ATA_COMM_REG);

        if (bm_stat & BM_STATUS_ERROR || ata_stat & STAT_ERR) {
            // Stop DMA & acknowledge
            bm_write8(bm_base, is_io, BM_COMMAND_OFFSET, BM_STATUS_STOP);
            bm_write8(bm_base, is_io, BM_STATUS_OFFSET, BM_STATUS_ACK);
            return FALSE;
        }

        // DMA done when BM not active AND ATA not busy
        if (!(bm_stat & BM_STATUS_ACTIVE) && !(ata_stat & STAT_BSY)) break;

        cpu_relax();
        ata_io_wait(base);
    }

    if (timeout == 0) {
        // Stop DMA & acknowledge
        bm_write8(bm_base, is_io, BM_COMMAND_OFFSET, BM_STATUS_STOP);
        bm_write8(bm_base, is_io, BM_STATUS_OFFSET, BM_STATUS_ACK);
        return FALSE;
    }

    // Stop DMA engine cleanly
    bm_write8(bm_base, is_io, BM_COMMAND_OFFSET, BM_STATUS_STOP);
    bm_write8(bm_base, is_io, BM_STATUS_OFFSET, BM_STATUS_ACK);

    // Final ATA status check
    timeout = POLLING_TIME;
    while (timeout--) {
        U8 status = _inb(base + ATA_COMM_REG);
        if (status & STAT_ERR) return FALSE;
        if (!(status & STAT_BSY) && (status & STAT_DRQ)) break;
        ata_io_wait(base);
    }

    if (!write) MEMCPY_OPT(buf, DMA_BUFFER, total_bytes);

    return TRUE;
}

BOOLEAN ATA_PIIX3_READ_SECTORS(U8 device, U32 lba, U8 sectors, VOIDPTR out) {
    return ATA_PIIX3_XFER(device, lba, sectors, out, FALSE);
}

BOOLEAN ATA_PIIX3_WRITE_SECTORS(U8 device, U32 lba, U8 sectors, VOIDPTR in) {
    return ATA_PIIX3_XFER(device, lba, sectors, in, TRUE);
}
