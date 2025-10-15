#include <DRIVERS/RTL8139/RTL8139.h>
#include <MEMORY/HEAP/KHEAP.h>
#include <RTOSKRNL_INTERNAL.h>
#include <STD/ASM.h>
#include <STD/MEM.h>
#include <STD/STRING.h>
#include <DRIVERS/VIDEO/VBE.h>
#include <CPU/PIC/PIC.h>
#include <CPU/ISR/ISR.h>
#include <DRIVERS/PCI/PCI.h>

void RTL8139_HANDLER(U32 vec, U32 errno);

// Key register bit
#define CR_BUF_EMPTY  (1 << 0)
#define CR_TX_ENABLE  (1 << 2)
#define CR_RX_ENABLE  (1 << 3)
#define CR_RESET      (1 << 4)

// IntrMask and IntrStatus
#define INT_RX_OK     (1 << 0)
#define INT_RX_ERR    (1 << 1)
#define INT_TX_OK     (1 << 2)
#define INT_TX_ERR    (1 << 3)
#define INT_RX_OVER   (1 << 4)
#define INT_RX_UNDERRUN (1 << 5)
#define INT_FIFO_OVER (1 << 6)
#define INT_LINK_CHG  (1 << 13)
#define INT_RX_FIFO_OV (1 << 6)
#define INT_SYSTEM_ERR (1 << 15)

#define RTL8139_INTRS (INT_RX_OK | INT_TX_OK | INT_RX_ERR | INT_TX_ERR)

// RxConfig
#define RCR_AAP  (1 << 0)  // Accept all physical packets
#define RCR_APM  (1 << 1)  // Accept physical match
#define RCR_AM   (1 << 2)  // Accept multicast
#define RCR_AB   (1 << 3)  // Accept broadcast
#define RCR_WRAP (1 << 7)  // Wrap around buffer
#define RCR_MXDMA_UNLIMITED (7 << 8)
#define RCR_RXFTH_NONE (7 << 13)

#define RTL8139_RCR_CONFIG (RCR_AAP | RCR_AB | RCR_AM | RCR_APM | RCR_MXDMA_UNLIMITED | RCR_RXFTH_NONE | RCR_WRAP)
#define RTL8139_RCR_CONFIG (RCR_AB | RCR_AM | RCR_APM | RCR_MXDMA_UNLIMITED | RCR_RXFTH_NONE | RCR_WRAP)

// TxConfig
#define TCR_IFG96   (3 << 24)
#define TCR_MXDMA_UNLIMITED (7 << 8)
#define TCR_CLRABT  (1 << 0)
#define TCR_TXRR_ZERO (0 << 4)

#define RTL8139_TCR_CONFIG (TCR_MXDMA_UNLIMITED | TCR_IFG96)

#define RTL8139_CR_RE 0x08
#define RTL8139_CR_TE 0x04

// Cfg9346
#define Cfg9346_UNLOCK 0xC0
#define Cfg9346_LOCK   0x00

// Media status
#define MSR_LINKB (1 << 2)
#define MSR_SPEED10 (1 << 3)
#define MSR_TXFLOW (1 << 4)
#define MSR_RXFLOW (1 << 5)
#define MSR_DUPLEX (1 << 8)

// TX descriptors
// RTL8139 Register Offsets
#define RTL8139_IDR0          0x00    // MAC[0]
#define RTL8139_IDR1          0x01
#define RTL8139_IDR2          0x02
#define RTL8139_IDR3          0x03
#define RTL8139_IDR4          0x04
#define RTL8139_IDR5          0x05
#define RTL8139_RES0          0x06    // Reserved (2 bytes)
#define RTL8139_MAR0          0x08    // Multicast filter low 32 bits
#define RTL8139_MAR1          0x0C    // Multicast filter high 32 bits

#define RTL8139_TXSTATUS0     0x10    // Transmit status 0
#define RTL8139_TXSTATUS1     0x14    // Transmit status 1
#define RTL8139_TXSTATUS2     0x18    // Transmit status 2
#define RTL8139_TXSTATUS3     0x1C    // Transmit status 3

#define RTL8139_TXADDR0       0x20    // Transmit address 0
#define RTL8139_TXADDR1       0x24    // Transmit address 1
#define RTL8139_TXADDR2       0x28    // Transmit address 2
#define RTL8139_TXADDR3       0x2C    // Transmit address 3

#define RTL8139_RBSTART       0x30    // Receive (RX) buffer start address
#define RTL8139_ERBCR         0x34    // Early RX byte count register
#define RTL8139_ERSR          0x36    // Early RX status register
#define RTL8139_CR            0x37    // Command register
#define RTL8139_CAPR          0x38    // Current address of packet read (RX)
#define RTL8139_CBR           0x3A    // Current RX buffer address
#define RTL8139_IMR           0x3C    // Interrupt mask
#define RTL8139_ISR           0x3E    // Interrupt status
#define RTL8139_TCR           0x40    // Transmit configuration
#define RTL8139_RCR           0x44    // Receive configuration
#define RTL8139_TCTR          0x48    // Timer count register
#define RTL8139_MPC           0x4C    // Missed packet counter
#define RTL8139_CFG9346       0x50    // 93C46 (EEPROM) command register
#define RTL8139_CONFIG0       0x51    // Configuration register 0
#define RTL8139_CONFIG1       0x52    // Configuration register 1
#define RTL8139_TIMERINT      0x54    // Timer interrupt register
#define RTL8139_MSR           0x58    // Media status register
#define RTL8139_CONFIG3       0x59    // Configuration register 3 (optional)
#define RTL8139_CONFIG4       0x5A    // Configuration register 4 (optional)
#define RTL8139_MULINT        0x5C    // Multiple interrupt select
#define RTL8139_RERID         0x5E    // PCI revision ID (optional)
#define RTL8139_TSAD          0x60    // Transmit status summary
#define RTL8139_BMCR          0x62    // Basic Mode Control (PHY)
#define RTL8139_BMSR          0x64    // Basic Mode Status (PHY)

#define RTL8139_VENDOR_ID    0x10EC
#define RTLS8139_DEVICE_ID   0x8139

#define RTL8139_CMD_RESET    0x10

#define RX_BUFFER_SIZE       (8192 + 16 + 1500)
#define DMA_BUFFER_ALIGN     0x100
#define BMCR_LOOPBACK (1 << 14)   // PHY loopback bit

#define ROK     (1 << 0)
#define FAE     (1 << 1)
#define CRC     (1 << 2)
#define LONG    (1 << 3)
#define RUNT    (1 << 4)
#define ISE     (1 << 5)
#define BAR     (1 << 13)
#define PAM     (1 << 14)
#define MAR     (1 << 15)

static U32 RTL8139_IO_BASE ATTRIB_DATA = 0;
static U32 RTL8139_RX_OFFSET ATTRIB_DATA = 0;
static VOIDPTR RTL8139_RX_BUF ATTRIB_DATA = NULL;
static U32 RTL8139_TX_CUR ATTRIB_DATA = 0;
static VOIDPTR RTLX8139_TX_BUFS[4] ATTRIB_DATA = {NULL, NULL, NULL, NULL};
static U8 MAC[6] ATTRIB_DATA = { 0 };
static PCI_DEVICE_ENTRY *dev = NULLPTR;
static U8 RTL8139_IRQ ATTRIB_DATA = 0;

BOOL RTL8139_ENABLE_BUS_MASTERING(){
    U32 pci_device_count = PCI_GET_DEVICE_COUNT();
    U8 some_set = FALSE;
    RTL8139_IO_BASE = 0;
    for (U32 i = 0; i < pci_device_count; i++) {
        PCI_DEVICE_ENTRY* _dev = PCI_GET_DEVICE(i);
        if (_dev->header.class_code == PCI_CLASS_NETWORK &&
            _dev->header.subclass  == PCI_SUBCLASS_ETHERNET) 
        {
            if(_dev->header.vendor_id == RTL8139_VENDOR_ID &&
                _dev->header.device_id == RTLS8139_DEVICE_ID) 
            {
                if(!PCI_ENABLE_BUS_MASTERING(_dev->bus, _dev->slot, _dev->func)) return FALSE;
                RTL8139_IO_BASE = _dev->header.bar0 & ~0x3;
                some_set = TRUE;
                dev = _dev;
                return TRUE;
            }
        }
    }

    return some_set;
}

void TURN_ON_RTL8139() {
    _outb(RTL8139_IO_BASE + RTL8139_CONFIG1, 0x0);
}
void RESET_RTL8139(){
    _outb(RTL8139_IO_BASE + RTL8139_CR, RTL8139_CMD_RESET);
    while (_inb(RTL8139_IO_BASE + RTL8139_CR) & RTL8139_CMD_RESET);
}
BOOL INIT_RX_BUFFER() {
    U8 *rx_buffer = KMALLOC_ALIGN(RX_BUFFER_SIZE, DMA_BUFFER_ALIGN);
    if(!rx_buffer) return FALSE;
    MEMZERO(rx_buffer, RX_BUFFER_SIZE);
    RTL8139_RX_BUF = (VOIDPTR)rx_buffer;
    _outl(RTL8139_IO_BASE + RTL8139_RBSTART, RTL8139_RX_BUF);

    RTL8139_RX_OFFSET = 0;
    _outw(RTL8139_IO_BASE + RTL8139_CAPR, 0);
    return TRUE;
}
BOOL INIT_TX_BUFFERS() {
    for(U8 i = 0; i < 4; i++) {
        RTLX8139_TX_BUFS[i] = KMALLOC_ALIGN(2048, DMA_BUFFER_ALIGN);
        if(!RTLX8139_TX_BUFS[i]) return FALSE;
        MEMZERO(RTLX8139_TX_BUFS[i], 2048);
        _outl(RTL8139_IO_BASE + RTL8139_TXADDR0 + (i * 4), RTLX8139_TX_BUFS[i]);
        _outl(RTL8139_IO_BASE + RTL8139_TXSTATUS0 + (i * 4), 0);
    }
    return TRUE;
}
void CONFIG_RX_TX() {
    _outl(RTL8139_IO_BASE + RTL8139_RCR, RTL8139_RCR_CONFIG);
    _outl(RTL8139_IO_BASE + RTL8139_TCR, RTL8139_TCR_CONFIG);
    _outl(RTL8139_IO_BASE + RTL8139_ERBCR, RX_BUFFER_SIZE);
}
void ENABLE_RX_TX() {
    _outb(RTL8139_IO_BASE + RTL8139_CR, RTL8139_CR_RE | RTL8139_CR_TE);
}
void READ_RTL8139_MAC() {
    for (int i = 0; i < 6; i++) {
        MAC[i] = _inb(RTL8139_IO_BASE + i);
    }
}
BOOLEAN VERIFY_RTL8139_MAC() {
    U8 bytes = 0;
    for (int i = 0; i < 6; i++) {
        if(MAC[i] == 0) bytes++;
    }
    return !(bytes == 6);
}
U8 SEARCH_RTL8139_IRQ() {
    RTL8139_IRQ = PCI_READ8(dev->bus, dev->slot, dev->func, PCI_INTERRUPT_LINE_OFFSET);
    return RTL8139_IRQ;
}
void INSTALL_RTL8139_IRQ() {
    ISR_REGISTER_HANDLER(PIC_REMAP_OFFSET + RTL8139_IRQ, RTL8139_HANDLER);
    PIC_Unmask(RTL8139_IRQ);
}

void ENABLE_RTL8139_INTERRUPTS() {
    _outw(RTL8139_IO_BASE + RTL8139_ISR, 0xFFFF);  // clear any pending interrupts
    _outw(RTL8139_IO_BASE + RTL8139_IMR, 0x0005); 
}

void ENABLE_PHY_LOOPBACK() {
    // Read BMCR
    U16 bmcr = _inw(RTL8139_IO_BASE + RTL8139_BMCR);
    bmcr |= BMCR_LOOPBACK;
    _outw(RTL8139_IO_BASE + RTL8139_BMCR, bmcr);
}

BOOLEAN RTL8139_STATUS() {
    if(!RTL8139_IO_BASE || !dev) return FALSE;
    return TRUE;
}

BOOLEAN RTL8139_START() {
    if(!RTL8139_ENABLE_BUS_MASTERING()) return FALSE;
    if(!RTL8139_IO_BASE || !dev) return FALSE;
    
    TURN_ON_RTL8139();
    RESET_RTL8139();
    if(!INIT_RX_BUFFER()) return RTL8139_STOP();
    if(!INIT_TX_BUFFERS()) return RTL8139_STOP();
    CONFIG_RX_TX();
    ENABLE_RX_TX();
    READ_RTL8139_MAC();
    if(!VERIFY_RTL8139_MAC()) return RTL8139_STOP();
    if(!SEARCH_RTL8139_IRQ()) return RTL8139_STOP();
    CLI;
    INSTALL_RTL8139_IRQ();
    ENABLE_RTL8139_INTERRUPTS();
    STI;
    
    return TRUE;
}

BOOLEAN RTL8139_STOP() {
    // Mask interrupts at device
    if (RTL8139_IO_BASE) {
        _outw(RTL8139_IO_BASE + RTL8139_IMR, 0x0000);    // disable all RTL interrupts
        _outw(RTL8139_IO_BASE + RTL8139_ISR, 0xFFFF);   // clear pending

        // Disable RX/TX in Command register
        U8 cr = _inb(RTL8139_IO_BASE + RTL8139_CR);
        cr &= ~(CR_RX_ENABLE | CR_TX_ENABLE);
        _outb(RTL8139_IO_BASE + RTL8139_CR, cr);

        // Optionally perform a reset to put card in known state
        _outb(RTL8139_IO_BASE + RTL8139_CR, RTL8139_CMD_RESET);
        while (_inb(RTL8139_IO_BASE + RTL8139_CR) & RTL8139_CMD_RESET) { /* wait */ }
    }

    // Unregister/mask IRQ at PIC level
    if (RTL8139_IRQ) {
        PIC_Mask(RTL8139_IRQ);   // mask at PIC (you used PIC_Unmask earlier)
        // If you have an ISR unregister function, call it here (pseudo):
        // ISR_UNREGISTER_HANDLER(PIC_REMAP_OFFSET + RTL8139_IRQ, RTL8139_HANDLER);
    }

    // Free DMA buffers
    if (RTL8139_RX_BUF) {
        KFREE_ALIGN(RTL8139_RX_BUF);
        RTL8139_RX_BUF = NULL;
    }
    for (U8 i = 0; i < 4; i++) {
        if (RTLX8139_TX_BUFS[i]) {
            KFREE_ALIGN(RTLX8139_TX_BUFS[i]);
            RTLX8139_TX_BUFS[i] = NULL;
        }
    }

    // Reset runtime state
    RTL8139_RX_OFFSET = 0;
    RTL8139_TX_CUR = 0;
    RTL8139_IO_BASE = 0;
    dev = NULLPTR;
    RTL8139_IRQ = 0;
    MEMZERO(MAC, sizeof(MAC));

    return FALSE; // keep existing behaviour (caller used this to indicate failure)
}


// helper to construct a simple Ethernet frame
// dst and src are 6-byte MACs, ether_type is 2 bytes (network order)
U32 BUILD_ETH_FRAME(U8 *buf, const U8 *dst, const U8 *src, U16 ether_type, const U8 *payload, U32 payload_len) {
    U32 offset = 0;
    MEMCPY_OPT(buf + offset, dst, 6); offset += 6;
    MEMCPY_OPT(buf + offset, src, 6); offset += 6;
    // store ether_type in network byte order
    buf[offset++] = (U8)((ether_type >> 8) & 0xFF);
    buf[offset++] = (U8)(ether_type & 0xFF);

    MEMCPY_OPT(buf + offset, payload, payload_len);
    offset += payload_len;

    // Enforce minimum Ethernet frame size (including 14 bytes header). Minimum is 60 bytes
    if (offset < 60) {
        MEMZERO(buf + offset, (60 - offset));
        offset = 60;
    }
    return offset;
}

// Try to send a single frame. Returns TRUE on queued, FALSE if no free descriptor.
BOOL SEND_RTL8139_PACKET(U8 *packet, U32 len) {
    // len must be >= 60 (Ethernet minimum payload with headers) or NIC will pad,
    // and <= 0x1FFF (13-bit length register).
    if (len == 0 || len > 0x1FFF) return FALSE;

    // Find a free TX descriptor (try all 4)
    for (U8 i = 0; i < 4; i++) {
        U32 txstatus_reg = RTL8139_IO_BASE + RTL8139_TXSTATUS0 + (i * 4);
        U32 txstatus = _inl(txstatus_reg);

        // Common approach: if NIC sets "OWN/ACTIVE" bit when busy, test that bit.
        // Many implementations check bit 13 (0x2000) to see if it's busy; if bit is 0 it's free.
        // If uncertain, check the RTL8139 datasheet for the exact "active" bit in TX status.
        if (txstatus & 0x2000) {
            // descriptor busy, try next
            continue;
        }

        // Descriptor is free — copy the frame into the DMA buffer
        MEMCPY_OPT(RTLX8139_TX_BUFS[i], packet, len);

        // Write physical address (driver already stored physical pointer at init)
        _outl(RTL8139_IO_BASE + RTL8139_TXADDR0 + (i * 4),
              (U32)RTLX8139_TX_BUFS[i]);

        // Kick transmission by writing frame length (lower 13 bits)
        // According to typical RTL8139 usage, writing length (len & 0x1FFF) starts TX
        _outl(RTL8139_IO_BASE + RTL8139_TXSTATUS0 + (i * 4), (len & 0x1FFF));

        // Optionally update a ring pointer if you use one for bookkeeping:
        // RTL8139_TX_CUR = (i + 1) % 4; // not strictly required if you search each time

        return TRUE; // queued successfully
    }

    // No free descriptors
    return FALSE;
}


void PROCESS_PACKET(U8 *packet, U32 len) {
    VBE_CLEAR_SCREEN(VBE_BLACK);
    U8 receiver[6];
    U8 sender[6];
    U16 ether_type;
    MEMCPY(receiver, packet, 6);
    MEMCPY(sender, packet+6, 6);
    MEMCPY(ether_type, packet+12, 2);
    U8 *payload = packet+14;
    DUMP_STRING("Receiver MAC");
    DUMP_MEMORY(receiver, 6);
    DUMP_STRING("Sender MAC");
    DUMP_MEMORY(sender, 6);
    DUMP_STRING("EtherType");
    DUMP_MEMORY(ether_type, 6);
    DUMP_STRING("Payload");
    DUMP_STRING(payload);
    set_rki_row(U32_MAX);

    DUMP_MEMORY(packet, len);
    set_rki_row(0);
}

static inline U32 rtl_next_rx_offset(U32 cur, U32 added) {
    U32 next = cur + added;
    if (next >= RX_BUFFER_SIZE) next -= RX_BUFFER_SIZE;
    return next;
}

void NET_HANDLE_PACKET(U8 *pkt, U16 len) {
    PROCESS_PACKET(pkt, len);
}

void RTL8139_HANDLE_RX() {
    // loop processing available packets (device raises RX OK for batches)
    // we must read CBR (current buffer read pointer) or track our offset to know when to stop.
    while (1) {
        // read the NIC's current buffer pointer (CBR). This points to the next byte
        // the NIC will write; data between our RTL8139_RX_OFFSET and CBR are ready.
        U16 cbr = _inw(RTL8139_IO_BASE + RTL8139_CBR);

        // if equal, no more packets
        if (RTL8139_RX_OFFSET == cbr) break;

        // Ensure we have a valid RX buffer
        if (!RTL8139_RX_BUF) break;

        U8 *rx = (U8*)RTL8139_RX_BUF;

        // Read packet header that starts at RTL8139_RX_OFFSET
        // The card writes a 4-byte header: [status (2 bytes), length (2 bytes)] (little-endian).
        // NOTE: some implementations swap the order; adjust if you observe garbage.
        U32 offset = RTL8139_RX_OFFSET;
        if (offset + 4 > RX_BUFFER_SIZE) {
            // header straddles buffer end — copy header bytes out into local array
            U8 header[4];
            for (int i = 0; i < 4; i++) header[i] = rx[(offset + i) % RX_BUFFER_SIZE];
            U16 pkt_status = (U16)(header[0] | (header[1] << 8));
            U16 pkt_len    = (U16)(header[2] | (header[3] << 8));
            // Move offset past header
            offset = rtl_next_rx_offset(offset, 4);
            // Now copy payload (pkt_len bytes) into contiguous buffer below
            U8 *pkt = KMALLOC_ALIGN(pkt_len, 4);
            if (!pkt) {
                // if allocation fails, drop packet and advance offset
                RTL8139_RX_OFFSET = rtl_next_rx_offset(RTL8139_RX_OFFSET, 4 + pkt_len);
                // update CAPR: tell NIC we've consumed up to (RTL8139_RX_OFFSET - 16)
                U16 capr = (U16)((RTL8139_RX_OFFSET - 16) & 0xFFFF);
                _outw(RTL8139_IO_BASE + RTL8139_CAPR, capr);
                continue;
            }
            for (U32 i = 0; i < pkt_len; i++) pkt[i] = rx[(offset + i) % RX_BUFFER_SIZE];
            // Advance RTL8139_RX_OFFSET past this packet
            RTL8139_RX_OFFSET = rtl_next_rx_offset(RTL8139_RX_OFFSET, 4 + pkt_len);
            // Tell NIC we've consumed packets by writing CAPR (see note below)
            U16 capr = (U16)((RTL8139_RX_OFFSET - 16) & 0xFFFF);
            _outw(RTL8139_IO_BASE + RTL8139_CAPR, capr);
            // Deliver to upper layer if status indicates OK
            if (!(pkt_status & (FAE | CRC | LONG | RUNT | ISE))) {
                NET_HANDLE_PACKET(pkt, pkt_len);
            } else {
                KFREE_ALIGN(pkt); // drop
            }
            continue;
        }

        // Header contiguous — read directly
        U16 pkt_status = *((U16*)(rx + offset));
        U16 pkt_len    = *((U16*)(rx + offset + 2));
        // move offset past header
        offset = rtl_next_rx_offset(offset, 4);

        // Basic sanity checks
        if (pkt_len > 2048) {
            // bogus length — advance past header and continue
            RTL8139_RX_OFFSET = rtl_next_rx_offset(RTL8139_RX_OFFSET, 4);
            U16 capr = (U16)((RTL8139_RX_OFFSET - 16) & 0xFFFF);
            _outw(RTL8139_IO_BASE + RTL8139_CAPR, capr);
            continue;
        }

        // Allocate a contiguous buffer for the packet payload
        U8 *pkt = KMALLOC_ALIGN(pkt_len, 4);
        if (!pkt) {
            // cannot allocate -> drop and advance
            RTL8139_RX_OFFSET = rtl_next_rx_offset(RTL8139_RX_OFFSET, 4 + pkt_len);
            U16 capr = (U16)((RTL8139_RX_OFFSET - 16) & 0xFFFF);
            _outw(RTL8139_IO_BASE + RTL8139_CAPR, capr);
            continue;
        }

        // Copy payload; handle possible wrap
        if (offset + pkt_len <= RX_BUFFER_SIZE) {
            MEMCPY(pkt, rx + offset, pkt_len);
        } else {
            // wrapped payload
            U32 first = RX_BUFFER_SIZE - offset;
            MEMCPY(pkt, rx + offset, first);
            MEMCPY(pkt + first, rx, pkt_len - first);
        }

        // Advance RTL8139_RX_OFFSET past header + payload
        RTL8139_RX_OFFSET = rtl_next_rx_offset(RTL8139_RX_OFFSET, 4 + pkt_len);

        // Update CAPR to indicate consumed bytes. Many RTL8139 drivers write (offset - 16).
        // This -16 is required by the chip to allow internal read pointer space (chip quirk).
        // If you see missed packets or lockups, you may need to adjust this value.
        U16 capr = (U16)((RTL8139_RX_OFFSET - 16) & 0xFFFF);
        _outw(RTL8139_IO_BASE + RTL8139_CAPR, capr);

        // If status indicates OK, hand packet to net stack, otherwise free
        if (pkt_status & ROK) {
            NET_HANDLE_PACKET(pkt, pkt_len);
        } else {
            KFREE_ALIGN(pkt);
        }
    } // end while packets available
}


void RTL8139_HANDLE_TX(){
    // handle transmit done
}
void RTL8139_HANDLE_TXERR(){

}
void RTL8139_HANDLE_TXOVERFLOW(){}
void RTL8139_HANDLE_LINK_CHANGED(){}




void RTL8139_HANDLER(U32 vec, U32 errno) {
    // Read interrupt status
    U16 status = _inw(RTL8139_IO_BASE + RTL8139_ISR);
    if (!status) goto end; // spurious

    // Acknowledge handled interrupts by writing back the status bits we read.
    // Important: write 1s to clear (device-specific).
    _outw(RTL8139_IO_BASE + RTL8139_ISR, status);

    // TX completed
    if (status & INT_TX_OK) {
        // you can inspect TX status registers to know which queue completed.
        // For now we just reset current TX index if the NIC cleared it.
        // If you track per-TX-buffer ownership, check TXSTATUSn registers here.
        // Example (simple): clear TX cur when card signals completion for our index:
        // (real logic may need checking the TXSTATUSn bits)
        // U32 txstatus = _inl(RTL8139_IO_BASE + RTL8139_TXSTATUS0 + (RTL8139_TX_CUR * 4));
        // if (txstatus & (1<<15)) { /* success */ }
        // (left as TODO for precise bookkeeping)
    }

    if (status & INT_RX_OK) {
        RTL8139_HANDLE_RX();
    }

    // RX error / RX overflow / system errors
    if (status & (INT_RX_ERR | INT_RX_OVER | INT_FIFO_OVER | INT_SYSTEM_ERR)) {
        // Basic recovery: clear errors, optionally reset the device if severe
        // You may want to log these conditions via VBE or kernel logging.
        // Small errors can be cleared by writing the ISR (done above). For severe errors,
        // consider reinitialising the device in a safe way.
        // TODO: add logging and recovery policy here.
    }

    // Link change
    if (status & INT_LINK_CHG) {
        // Query MSR and adjust link state if necessary
        U16 msr = _inw(RTL8139_IO_BASE + RTL8139_MSR);
        // TODO: signal link state to upper layers
    }
    end:
    pic_send_eoi(RTL8139_IRQ);
}