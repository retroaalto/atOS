#ifndef PCI_H
#define PCI_H
#include <STD/TYPEDEF.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC
#define PCI_MAX_BUS        256
#define PCI_MAX_DEVICES    256
#define PCI_MAX_FUNC       8

// PCI Class Codes and Subclass Codes
// PCI Class Codes and Subclass Codes

// 0x00 – Unclassified
#define PCI_CLASS_UNCLASSIFIED       0x00
#define PCI_SUBCLASS_NON_VGA         0x00
#define PCI_SUBCLASS_VGA_COMPATIBLE  0x01

// 0x01 – Mass Storage Controller
#define PCI_CLASS_MASS_STORAGE       0x01
#define PCI_SUBCLASS_SCSI            0x00
#define PCI_SUBCLASS_IDE             0x01
#define PCI_SUBCLASS_FLOPPY          0x02
#define PCI_SUBCLASS_IPI             0x03
#define PCI_SUBCLASS_RAID            0x04
#define PCI_SUBCLASS_ATA             0x05
#define PCI_SUBCLASS_SATA            0x06
#define PCI_SUBCLASS_SAS             0x07
#define PCI_SUBCLASS_NVME            0x08
#define PCI_SUBCLASS_OTHER_STORAGE   0x80

// 0x02 – Network Controller
#define PCI_CLASS_NETWORK            0x02
#define PCI_SUBCLASS_ETHERNET        0x00
#define PCI_SUBCLASS_TOKEN_RING      0x01
#define PCI_SUBCLASS_FDDI            0x02
#define PCI_SUBCLASS_ATM             0x03
#define PCI_SUBCLASS_ISDN            0x04
#define PCI_SUBCLASS_OTHER_NET       0x80
#define PCI_SUBCLASS_WIFI            0x80 // vendor-specific

// 0x03 – Display Controller
#define PCI_CLASS_DISPLAY            0x03
#define PCI_SUBCLASS_VGA             0x00
#define PCI_SUBCLASS_XGA             0x01
#define PCI_SUBCLASS_3D              0x02
#define PCI_SUBCLASS_OTHER_DISPLAY   0x80

// 0x04 – Multimedia Controller
#define PCI_CLASS_MULTIMEDIA         0x04
#define PCI_SUBCLASS_VIDEO           0x00
#define PCI_SUBCLASS_AUDIO           0x01
#define PCI_SUBCLASS_PHONE           0x03
#define PCI_SUBCLASS_SATELLITE       0x04
#define PCI_SUBCLASS_COPROCESSOR     0x05
#define PCI_SUBCLASS_OTHER_MULTIMEDIA 0x80

// 0x05 – Memory Controller
#define PCI_CLASS_MEMORY             0x05
#define PCI_SUBCLASS_RAM             0x00
#define PCI_SUBCLASS_FLASH           0x01
#define PCI_SUBCLASS_OTHER_MEMORY    0x80

// 0x06 – Bridge Device
#define PCI_CLASS_BRIDGE             0x06
#define PCI_SUBCLASS_HOST            0x00
#define PCI_SUBCLASS_ISA             0x01
#define PCI_SUBCLASS_EISA            0x02
#define PCI_SUBCLASS_MCA             0x03
#define PCI_SUBCLASS_PCI_TO_PCI      0x04
#define PCI_SUBCLASS_PCI_TO_USB      0x09
#define PCI_SUBCLASS_OTHER_BRIDGE    0x80

// 0x07 – Simple Communication Controller
#define PCI_CLASS_COMM               0x07
#define PCI_SUBCLASS_SERIAL          0x00
#define PCI_SUBCLASS_PARALLEL        0x01
#define PCI_SUBCLASS_MULTIPORT_SERIAL 0x02
#define PCI_SUBCLASS_MODEM           0x03
#define PCI_SUBCLASS_GPIB            0x04
#define PCI_SUBCLASS_SMART_CARD      0x05
#define PCI_SUBCLASS_OTHER_COMM      0x80

// 0x08 – Base System Peripheral
#define PCI_CLASS_BASE_SYSTEM        0x08
#define PCI_SUBCLASS_PIC             0x00
#define PCI_SUBCLASS_DMA             0x01
#define PCI_SUBCLASS_TIMER           0x02
#define PCI_SUBCLASS_RTC             0x03
#define PCI_SUBCLASS_PCI_HOTPLUG     0x04
#define PCI_SUBCLASS_OTHER_BASE      0x80

// 0x09 – Input Device Controller
#define PCI_CLASS_INPUT              0x09
#define PCI_SUBCLASS_KEYBOARD        0x00
#define PCI_SUBCLASS_DIGITIZER       0x01
#define PCI_SUBCLASS_MOUSE           0x02
#define PCI_SUBCLASS_SCANNER         0x03
#define PCI_SUBCLASS_GAMEPORT        0x04
#define PCI_SUBCLASS_OTHER_INPUT     0x80

// 0x0A – Docking Station
#define PCI_CLASS_DOCK               0x0A
#define PCI_SUBCLASS_GENERIC_DOCK    0x00
#define PCI_SUBCLASS_OTHER_DOCK      0x80

// 0x0B – Processor
#define PCI_CLASS_PROCESSOR          0x0B
#define PCI_SUBCLASS_386             0x00
#define PCI_SUBCLASS_486             0x01
#define PCI_SUBCLASS_PENTIUM         0x02
#define PCI_SUBCLASS_ALPHA           0x10
#define PCI_SUBCLASS_POWERPC         0x20
#define PCI_SUBCLASS_OTHER_PROC      0x80

// 0x0C – Serial Bus Controller
#define PCI_CLASS_SERIAL_BUS         0x0C
#define PCI_SUBCLASS_FIREWIRE        0x00
#define PCI_SUBCLASS_ACCESS_BUS      0x01
#define PCI_SUBCLASS_SSA             0x02
#define PCI_SUBCLASS_USB             0x03
#define PCI_SUBCLASS_FIBRE_CHANNEL   0x04
#define PCI_SUBCLASS_SMBUS           0x05
#define PCI_SUBCLASS_INFINIBAND      0x06
#define PCI_SUBCLASS_IPMI            0x07
#define PCI_SUBCLASS_SERCOS          0x08
#define PCI_SUBCLASS_CAN             0x09
#define PCI_SUBCLASS_OTHER_SERIAL    0x80


#define PCI_CLASS_SERIAL_BUS    0x0C
#define PCI_SUBCLASS_USB        0x03
#define PCI_SUBCLASS_SMBUS      0x05



#define PCI_HEADER_TYPE_OFFSET       0x0E
#define PCI_CLASS_CODE_OFFSET        0x0B
#define PCI_SUBCLASS_OFFSET          0x0A
#define PCI_PROG_IF_OFFSET           0x09
#define PCI_REVISION_ID_OFFSET       0x08
#define PCI_VENDOR_ID_OFFSET         0x00
#define PCI_DEVICE_ID_OFFSET         0x02
#define PCI_INTERRUPT_LINE_OFFSET    0x3C
#define PCI_INTERRUPT_PIN_OFFSET     0x3D
#define PCI_STATUS_OFFSET            0x06
#define PCI_COMMAND_OFFSET           0x04
#define PCI_CAPABILITIES_POINTER     0x34
#define PCI_SECONDARY_BUS_OFFSET     0x19
#define PCI_SUBSYSTEM_VENDOR_ID_OFFSET 0x2C
#define PCI_SUBSYSTEM_ID_OFFSET       0x2E

// PCI Configuration Space Offsets
typedef struct {
    U16 vendor_id;      // 0x00
    U16 device_id;      // 0x02
    U16 command;        // 0x04
    U16 status;         // 0x06
    U8 revision_id;     // 0x08
    U8 prog_if;         // 0x09
    U8 subclass;        // 0x0A
    U8 class_code;      // 0x0B
    U8 cache_line_size; // 0x0C
    U8 latency_timer;   // 0x0D
    U8 header_type;     // 0x0E
    U8 bist;            // 0x0F
    U32 bar0;           // 0x10
    U32 bar1;           // 0x14
    U32 bar2;           // 0x18
    U32 bar3;           // 0x1C
    U32 bar4;           // 0x20
    U32 bar5;           // 0x24
    U32 cardbus_cis_pointer; // 0x28 (optional)
    U16 subsystem_vendor_id; // 0x2C
    U16 subsystem_id;        // 0x2E
    U32 expansion_rom_base;  // 0x30
    U8 capabilities_pointer; // 0x34
    U8 reserved1[7];         // 0x35–0x3B
    U8 interrupt_line;       // 0x3C
    U8 interrupt_pin;        // 0x3D
    U8 min_grant;            // 0x3E
    U8 max_latency;          // 0x3F
} ATTRIB_PACKED PCI_TYPE0_HEADER;

typedef struct {
    U8 bus;
    U8 slot;
    U8 func;
    PCI_TYPE0_HEADER header;
} ATTRIB_PACKED PCI_DEVICE_ENTRY;

BOOLEAN PCI_ENABLE_BUS_MASTER_FOR_PIXX3(void);
U8 PCI_READ8(U8 bus, U8 slot, U8 func, U8 offset);
U16 PCI_READ16(U8 bus, U8 slot, U8 func, U8 offset);
U32 PCI_READ32(U8 bus, U8 slot, U8 func, U8 offset);
void PCI_WRITE8(U8 bus, U8 slot, U8 func, U8 offset, U8 value);
void PCI_WRITE16(U8 bus, U8 slot, U8 func, U8 offset, U16 value);
void PCI_WRITE32(U8 bus, U8 slot, U8 func, U8 offset, U32 value);
U8 PCI_GET_HEADER_TYPE(U8 bus, U8 slot, U8 func);
U8 PCI_GET_CLASS(U8 bus, U8 slot, U8 func);
U8 PCI_GET_SUBCLASS(U8 bus, U8 slot, U8 func);
U8 PCI_GET_PROG_IF(U8 bus, U8 slot, U8 func);
U16 PCI_GET_VENDOR_ID(U8 bus, U8 slot, U8 func);
U16 PCI_GET_DEVICE_ID(U8 bus, U8 slot, U8 func);
U8 PCI_GET_SECONDARY_BUS(U8 bus, U8 slot, U8 func);
U8 PCI_DEVICE_HAS_CAPABILITY(U8 bus, U8 slot, U8 func, U8 cap_id);
U8 PCI_FIND_CAPABILITY(U8 bus, U8 slot, U8 func, U8 cap_id);
U32 PCI_GET_BAR0(U8 bus, U8 slot, U8 func);
U32 PCI_GET_BAR1(U8 bus, U8 slot, U8 func);
U32 PCI_GET_BAR2(U8 bus, U8 slot, U8 func);
U32 PCI_GET_BAR3(U8 bus, U8 slot, U8 func);
U32 PCI_GET_BAR4(U8 bus, U8 slot, U8 func);
U32 PCI_GET_BAR5(U8 bus, U8 slot, U8 func);
U32 PCI_GET_BAR(U8 bus, U8 slot, U8 func, U8 bar_num);
U32 PCI_GET_INTERRUPT_LINE(U8 bus, U8 slot, U8 func);
U32 PCI_GET_INTERRUPT_PIN(U8 bus, U8 slot, U8 func);
U32 PCI_GET_CAPABILITIES_POINTER(U8 bus, U8 slot, U8 func);
U32 PCI_GET_STATUS(U8 bus, U8 slot, U8 func);
U32 PCI_GET_COMMAND(U8 bus, U8 slot, U8 func);
U16 PCI_GET_SUBSYSTEM_VENDOR_ID(U8 bus, U8 slot, U8 func);
U16 PCI_GET_SUBSYSTEM_ID(U8 bus, U8 slot, U8 func);
U8 PCI_IS_MULTI_FUNCTION_DEVICE(U8 bus, U8 slot);

void PCI_INITIALIZE(void);
U32 PCI_GET_DEVICE_COUNT(void);
PCI_DEVICE_ENTRY* PCI_GET_DEVICE(U32 index);
PCI_DEVICE_ENTRY* PCI_GET_LIST(void);
#endif // PCI_H