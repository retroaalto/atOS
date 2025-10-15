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
    U8 RTL8139_IRQ = PCI_READ8(dev->bus, dev->slot, dev->func, PCI_INTERRUPT_LINE_OFFSET);
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

BOOLEAN RTL8139_INIT() {
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
    // ENABLE_PHY_LOOPBACK();
    
    U8 test_packet[48] = {
        0xaa, 0xbb, 0xcc, 0x11, 0x22, 0x33, 0x77, 0x99,
        0xaa, 0xbb, 0xcc, 0x11, 0x22, 0x33, 0x77, 0x99,
        0xaa, 0xbb, 0xcc, 0x11, 0x22, 0x33, 0x77, 0x99,
        0xaa, 0xbb, 0xcc, 0x11, 0x22, 0x33, 0x77, 0x99,
        0xaa, 0xbb, 0xcc, 0x11, 0x22, 0x33, 0x77, 0x99,
        0xaa, 0xbb, 0xcc, 0x11, 0x22, 0x33, 0x77, 0x99,
    }; // minimal Ethernet frame
    while(1) {
        // SEND_RTL8139_PACKET(test_packet, sizeof(test_packet));
        U16 status = _inw(RTL8139_IO_BASE + RTL8139_ISR);
        if (status) {
            RTL8139_HANDLER(0, 0);
        }
    }
    
    return TRUE;
}

BOOLEAN RTL8139_STOP() {
    if(RTL8139_RX_BUF) KFREE_ALIGN(RTL8139_RX_BUF);
    for(U8 i = 0; i < 4; i++) {
        if(RTLX8139_TX_BUFS[i]) KFREE_ALIGN(RTLX8139_TX_BUFS[i]);
    }

    return FALSE;
}

void SEND_RTL8139_PACKET(U8 *packet, U32 len) {
    // Check if current TX buffer is free
    if (!(_inl(RTL8139_IO_BASE + RTL8139_TXSTATUS0 + (RTL8139_TX_CUR * 4)) & CR_BUF_EMPTY)) {
        // Buffer busy, cannot send
        return;
    }

    MEMCPY_OPT(RTLX8139_TX_BUFS[RTL8139_TX_CUR], packet, len);
    _outl(RTL8139_IO_BASE + RTL8139_TXADDR0 + (RTL8139_TX_CUR * 4), (U32)RTLX8139_TX_BUFS[RTL8139_TX_CUR]);
    _outl(RTL8139_IO_BASE + RTL8139_TXSTATUS0 + (RTL8139_TX_CUR * 4), len & 0x1FFF); // 13-bit length
    RTL8139_TX_CUR = (RTL8139_TX_CUR + 1) % 4;
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

void RTL8139_HANDLE_RX() {
    while (!(_inb(RTL8139_IO_BASE + RTL8139_CR) & CR_BUF_EMPTY)) {
        U16 *packet_header = (U16*)(RTL8139_RX_BUF + RTL8139_RX_OFFSET);
        U16 status = packet_header[0];
        U16 length = packet_header[1];
        U8 *packet = (U8 *)(RTL8139_RX_BUF + RTL8139_RX_OFFSET + 4);

        if (status & ROK) {
            PROCESS_PACKET(packet, length);
        } else {
            DUMP_ERRCODE(status); // debug errors
        }

        // Move to next packet, align to 4 bytes
        RTL8139_RX_OFFSET = (RTL8139_RX_OFFSET + length + 4 + 3) & ~3;
        if (RTL8139_RX_OFFSET >= RX_BUFFER_SIZE)
            RTL8139_RX_OFFSET -= RX_BUFFER_SIZE;

        // Tell NIC we consumed the packet
        _outw(RTL8139_IO_BASE + RTL8139_CAPR, RTL8139_RX_OFFSET - 16);
    }
}


void RTL8139_HANDLE_TX(){
    // handle transmit done
}
void RTL8139_HANDLE_TXERR(){

}
void RTL8139_HANDLE_TXOVERFLOW(){}
void RTL8139_HANDLE_LINK_CHANGED(){}

void RTL8139_HANDLER(U32 vec, U32 errno) {
    U16 status = _inw(RTL8139_IO_BASE + RTL8139_ISR);
    if(!status) goto end;
    _outw(RTL8139_IO_BASE + RTL8139_ISR, 0x05);// ack
    if (status & INT_RX_OK) {
        RTL8139_HANDLE_RX();
    }
    if (status & INT_TX_OK) {
        RTL8139_HANDLE_TX();
    }
    if (status & INT_TX_ERR) {
        RTL8139_HANDLE_TXERR();
    }
    if (status & INT_RX_OVER) {
        RTL8139_HANDLE_TXOVERFLOW();
    }
    if (status & INT_LINK_CHG) {
        RTL8139_HANDLE_LINK_CHANGED();
    }
    if (status & (INT_RX_ERR | INT_TX_ERR | INT_RX_OVER | INT_RX_UNDERRUN | INT_SYSTEM_ERR)) {
        DUMP_ERRCODE(0xFF);
        RESET_RTL8139();
        HLT;
    }
    end:
    pic_send_eoi(RTL8139_IRQ);
}