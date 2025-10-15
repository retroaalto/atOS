#include <DRIVERS/RTL8139/RTL8139.h>
#include <MEMORY/HEAP/KHEAP.h>
#include <RTOSKRNL_INTERNAL.h>
#include <STD/ASM.h>
#include <STD/MEM.h>
#include <STD/STRING.h>
#include <DRIVERS/VIDEO/VBE.h>
#include <CPU/PIC/PIC.h>
#include <DRIVERS/PCI/PCI.h>

typedef struct {
    U8 MAC[6];      // Read only mac address
    U16 RES0;
    U32 MAR0;       // Multicast filter low 32
    U32 MAR1;       // Multicast filter high 32
    U32 TXSTATUS0;  // Transmit status
    U32 TXSTATUS1;  // Transmit status
    U32 TXSTATUS2;  // Transmit status
    U32 TXSTATUS3;  // Transmit status
    U32 TXADDR0;    // Transmit buffer
    U32 TXADDR1;    // Transmit buffer
    U32 TXADDR2;    // Transmit buffer
    U32 TXADDR3;    // Transmit buffer
    U32 RXBUF;      // Receive buffer
    U8 CMD;         // Command register
    U16 RXBUFPTR;   // Current address of receive buffer
    U16 RXBUFADDR;  // Current receive address 
    U16 INTRMASK;   // Interrupt mask register
    U16 INTRSTATUS; // Interrupt mask status
    U32	TxConfig;	// Transmit configuration
    U32	RxConfig;   // Receive configuration
    U32	Timer;      // Timer count register
    U16	MissedPkt;  // Missed packet counter
    U8	Cfg9346;    // 93C46 (EEPROM) Command Register
    U8	Config0;    // Configuration register 0
    U8	Config1;    // Configuration register 1
    U32	TimerInt;   // Timer interrupt register
    U8	MediaStatus;// Media status register
    U16	MultiIntr;  // Multiple interrupt select
    U16	TxSummary;  // Transmit status summary
    U16	BasicModeCtrl;      // PHY control register
    U16	BasicModeStatus;    // PHY status register
} RTL8139_REGS;

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

#define RTL8139_RCR_CONFIG (RCR_AB | RCR_AM | RCR_APM | RCR_AAP | RCR_MXDMA_UNLIMITED | RCR_RXFTH_NONE)

// TxConfig
#define TCR_IFG96   (3 << 24)
#define TCR_MXDMA_UNLIMITED (7 << 8)
#define TCR_CLRABT  (1 << 0)
#define TCR_TXRR_ZERO (0 << 4)

#define RTL8139_TCR_CONFIG (TCR_MXDMA_UNLIMITED | TCR_IFG96)

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
#define TX_STATUS0 0x10
#define TX_STATUS1 0x14
#define TX_STATUS2 0x18
#define TX_STATUS3 0x1C
#define TX_ADDR0   0x20
#define TX_ADDR1   0x24
#define TX_ADDR2   0x28
#define TX_ADDR3   0x2C

#define RTL8139_CR           0x37
#define RTL8139_IMR          0x3C
#define RTL8139_ISR          0x3E
#define RTL8139_RBSTART      0x30
#define RTL8139_RCR          0x44
#define RTL8139_TCR          0x40
#define RTL8139_TXSTATUS0    0x10
#define RTL8139_TXADDR0      0x20
#define RTL8139_CFG9346      0x50
#define RTL8139_CONFIG1      0x52
#define RTL8139_MSR          0x58

#define RTL8139_VENDOR_ID    0x10EC
#define RTLS8139_DEVICE_ID   0x8139

static U32 RTL8139_BASE ATTRIB_DATA = 0;

BOOL RTL8139_ENABLE_BUS_MASTERING(){
    U32 pci_device_count = PCI_GET_DEVICE_COUNT();
    U8 some_set = FALSE;
    for (U32 i = 0; i < pci_device_count; i++) {
        PCI_DEVICE_ENTRY* dev = PCI_GET_DEVICE(i);
        if (dev->header.class_code == PCI_CLASS_NETWORK &&
            dev->header.subclass  == PCI_SUBCLASS_ETHERNET) 
        {
            if(dev->header.vendor_id == RTL8139_VENDOR_ID &&
                dev->header.device_id == RTLS8139_DEVICE_ID) 
            {
                if(!PCI_ENABLE_BUS_MASTERING(dev->bus, dev->slot, dev->func)) return FALSE;
                RTL8139_BASE = dev->header.bar0;
                some_set = TRUE;
            }
        }
    }
    return some_set;
}

void TURN_ON_RTL8139() {
    _outb(RTL8139_BASE + 0x52, 0x0);
}
void RESET_RTL8139(){
    _outb(RTL8139_BASE + 0x37, CR_RESET);
    while (_inb(RTL8139_BASE + 0x37) & CR_RESET);

}
BOOLEAN RTL8139_INIT() {
    if(!RTL8139_ENABLE_BUS_MASTERING()) return FALSE;
    TURN_ON_RTL8139();
    RESET_RTL8139();

}