#include <STD/ASM.h>
#include <DRIVERS/PCI/PCI.h>

static PCI_DEVICE_ENTRY pci_devices[PCI_MAX_DEVICES];
static U32 pci_device_count = 0;


U8 PCI_READ8(U8 bus, U8 slot, U8 func, U8 offset) {
    U32 address = (U32)((bus << 16) | (slot << 11) | (func << 8) |
                        (offset & 0xFC) | 0x80000000);
    _outl(PCI_CONFIG_ADDRESS, address);
    U32 tmp = _inl(PCI_CONFIG_DATA);
    return (U8)((tmp >> ((offset & 3) * 8)) & 0xFF);
}
U16 PCI_READ16(U8 bus, U8 slot, U8 func, U8 offset) {
    U32 address = (U32)((bus << 16) | (slot << 11) | (func << 8) |
                        (offset & 0xFC) | 0x80000000);
    _outl(PCI_CONFIG_ADDRESS, address);
    U32 tmp = _inl(PCI_CONFIG_DATA);
    return (U16)((tmp >> ((offset & 2) * 8)) & 0xFFFF);
}
U32 PCI_READ32(U8 bus, U8 slot, U8 func, U8 offset) {
    U32 address = (U32)((bus << 16) | (slot << 11) | (func << 8) |
                        (offset & 0xFC) | 0x80000000);
    _outl(PCI_CONFIG_ADDRESS, address);
    return _inl(PCI_CONFIG_DATA);
}
void PCI_WRITE8(U8 bus, U8 slot, U8 func, U8 offset, U8 value) {
    U32 address = (U32)((bus << 16) | (slot << 11) | (func << 8) |
                        (offset & 0xFC) | 0x80000000);
    _outl(PCI_CONFIG_ADDRESS, address);
    U32 tmp = _inl(PCI_CONFIG_DATA);
    U32 mask = 0xFF << ((offset & 3) * 8);
    tmp = (tmp & ~mask) | ((value << ((offset & 3) * 8)) & mask);
    _outl(PCI_CONFIG_DATA, tmp);
}
void PCI_WRITE16(U8 bus, U8 slot, U8 func, U8 offset, U16 value) {
    U32 address = (U32)((bus << 16) | (slot << 11) | (func << 8) |
                        (offset & 0xFC) | 0x80000000);
    _outl(PCI_CONFIG_ADDRESS, address);
    U32 tmp = _inl(PCI_CONFIG_DATA);
    U32 mask = 0xFFFF << ((offset & 2) * 8);
    tmp = (tmp & ~mask) | ((value << ((offset & 2) * 8)) & mask);
    _outl(PCI_CONFIG_DATA, tmp);
}
void PCI_WRITE32(U8 bus, U8 slot, U8 func, U8 offset, U32 value) {
    U32 address = (U32)((bus << 16) | (slot << 11) | (func << 8) |
                        (offset & 0xFC) | 0x80000000);
    _outl(PCI_CONFIG_ADDRESS, address);
    _outl(PCI_CONFIG_DATA, value);
}

U8 PCI_GET_HEADER_TYPE(U8 bus, U8 slot, U8 func) {
    return PCI_READ8(bus, slot, func, PCI_HEADER_TYPE_OFFSET);
}
U8 PCI_GET_CLASS(U8 bus, U8 slot, U8 func) {
    return PCI_READ8(bus, slot, func, PCI_CLASS_CODE_OFFSET);
}
U8 PCI_GET_SUBCLASS(U8 bus, U8 slot, U8 func) {
    return PCI_READ8(bus, slot, func, PCI_SUBCLASS_OFFSET);
}
U8 PCI_GET_PROG_IF(U8 bus, U8 slot, U8 func) {
    return PCI_READ8(bus, slot, func, PCI_PROG_IF_OFFSET);
}
U8 PCI_GET_SECONDARY_BUS(U8 bus, U8 slot, U8 func) {
    return PCI_READ8(bus, slot, func, PCI_SECONDARY_BUS_OFFSET);
}
U16 PCI_GET_VENDOR_ID(U8 bus, U8 slot, U8 func) {
    return PCI_READ16(bus, slot, func, PCI_VENDOR_ID_OFFSET);
}
U16 PCI_GET_DEVICE_ID(U8 bus, U8 slot, U8 func) {
    return PCI_READ16(bus, slot, func, PCI_DEVICE_ID_OFFSET);
}
U32 PCI_GET_BAR(U8 bus, U8 slot, U8 func, U8 bar_num) {
    if(bar_num > 5) return 0;
    return PCI_READ32(bus, slot, func, 0x10 + (bar_num * 4));
}
U32 PCI_GET_BAR0(U8 bus, U8 slot, U8 func) { return PCI_GET_BAR(bus, slot, func, 0); }
U32 PCI_GET_BAR1(U8 bus, U8 slot, U8 func) { return PCI_GET_BAR(bus, slot, func, 1); }
U32 PCI_GET_BAR2(U8 bus, U8 slot, U8 func) { return PCI_GET_BAR(bus, slot, func, 2); }
U32 PCI_GET_BAR3(U8 bus, U8 slot, U8 func) { return PCI_GET_BAR(bus, slot, func, 3); }
U32 PCI_GET_BAR4(U8 bus, U8 slot, U8 func) { return PCI_GET_BAR(bus, slot, func, 4); }
U32 PCI_GET_BAR5(U8 bus, U8 slot, U8 func) { return PCI_GET_BAR(bus, slot, func, 5); }

U32 PCI_GET_INTERRUPT_LINE(U8 bus, U8 slot, U8 func) {
    return PCI_READ8(bus, slot, func, PCI_INTERRUPT_LINE_OFFSET);
}
U32 PCI_GET_INTERRUPT_PIN(U8 bus, U8 slot, U8 func) {
    return PCI_READ8(bus, slot, func, PCI_INTERRUPT_PIN_OFFSET);
}
U32 PCI_GET_STATUS(U8 bus, U8 slot, U8 func) {
    return PCI_READ16(bus, slot, func, PCI_STATUS_OFFSET);
}
U32 PCI_GET_COMMAND(U8 bus, U8 slot, U8 func) {
    return PCI_READ16(bus, slot, func, PCI_COMMAND_OFFSET);
}
U32 PCI_GET_CAPABILITIES_POINTER(U8 bus, U8 slot, U8 func) {
    return PCI_READ8(bus, slot, func, PCI_CAPABILITIES_POINTER);
}
U16 PCI_GET_SUBSYSTEM_VENDOR_ID(U8 bus, U8 slot, U8 func) {
    return PCI_READ16(bus, slot, func, PCI_SUBSYSTEM_VENDOR_ID_OFFSET);
}
U16 PCI_GET_SUBSYSTEM_ID(U8 bus, U8 slot, U8 func) {
    return PCI_READ16(bus, slot, func, PCI_SUBSYSTEM_ID_OFFSET);
}
U8 PCI_IS_MULTI_FUNCTION_DEVICE(U8 bus, U8 slot) {
    return (PCI_READ8(bus, slot, 0, PCI_HEADER_TYPE_OFFSET) & 0x80) != 0;
}

// Simple function to check if a device supports a capability
U8 PCI_DEVICE_HAS_CAPABILITY(U8 bus, U8 slot, U8 func, U8 cap_id) {
    U32 ptr = PCI_GET_CAPABILITIES_POINTER(bus, slot, func);
    while(ptr) {
        if(PCI_READ8(bus, slot, func, ptr) == cap_id) return 1;
        ptr = PCI_READ8(bus, slot, func, ptr + 1);
    }
    return 0;
}

U8 PCI_FIND_CAPABILITY(U8 bus, U8 slot, U8 func, U8 cap_id) {
    U32 ptr = PCI_GET_CAPABILITIES_POINTER(bus, slot, func);
    while(ptr) {
        if(PCI_READ8(bus, slot, func, ptr) == cap_id) return ptr;
        ptr = PCI_READ8(bus, slot, func, ptr + 1);
    }
    return 0;
}

// Scan all buses/devices/functions to find first IDE controller
BOOLEAN PCI_FIND_IDE_CONTROLLER(U8* out_bus, U8* out_slot, U8* out_func) {
    for(U8 bus = 0; bus < PCI_MAX_BUS; bus++) {
        for(U8 slot = 0; slot < PCI_MAX_DEVICES; slot++) {
            U8 funcs = PCI_IS_MULTI_FUNCTION_DEVICE(bus, slot) ? 8 : 1;
            for(U8 func = 0; func < funcs; func++) {
                if(PCI_GET_CLASS(bus, slot, func) == PCI_CLASS_MASS_STORAGE &&
                   PCI_GET_SUBCLASS(bus, slot, func) == PCI_SUBCLASS_IDE) {
                    *out_bus = bus;
                    *out_slot = slot;
                    *out_func = func;
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

BOOLEAN PCI_ENABLE_BUS_MASTER_FOR_PIXX3(void) {
    U8 bus, slot, func;
    if(!PCI_FIND_IDE_CONTROLLER(&bus, &slot, &func)) return FALSE;

    U16 command = PCI_GET_COMMAND(bus, slot, func);
    command |= 0x04; // Set Bus Master Enable bit
    PCI_WRITE16(bus, slot, func, PCI_COMMAND_OFFSET, command);

    // Verify
    command = PCI_GET_COMMAND(bus, slot, func);
    return (command & 0x04) != 0;
}

// Enumerate all PCI devices and fill pci_devices[]
void PCI_ENUMERATE(void) {
    pci_device_count = 0;

    for (U8 bus = 0; bus < PCI_MAX_BUS; bus++) {
        for (U8 slot = 0; slot < PCI_MAX_DEVICES; slot++) {
            // Check if this slot is multi-function
            U8 funcs = PCI_IS_MULTI_FUNCTION_DEVICE(bus, slot) ? PCI_MAX_FUNC : 1;

            for (U8 func = 0; func < funcs; func++) {
                U16 vendor_id = PCI_GET_VENDOR_ID(bus, slot, func);
                if (vendor_id == 0xFFFF) continue; // No device here

                if (pci_device_count >= PCI_MAX_DEVICES) return; // Array full

                PCI_DEVICE_ENTRY* dev = &pci_devices[pci_device_count++];
                dev->bus  = bus;
                dev->slot = slot;
                dev->func = func;

                // Fill Type0 header fields
                PCI_TYPE0_HEADER* h = &dev->header;
                h->vendor_id       = vendor_id;
                h->device_id       = PCI_GET_DEVICE_ID(bus, slot, func);
                h->command         = PCI_GET_COMMAND(bus, slot, func);
                h->status          = PCI_GET_STATUS(bus, slot, func);
                h->revision_id     = PCI_READ8(bus, slot, func, PCI_REVISION_ID_OFFSET);
                h->prog_if         = PCI_GET_PROG_IF(bus, slot, func);
                h->subclass        = PCI_GET_SUBCLASS(bus, slot, func);
                h->class_code      = PCI_GET_CLASS(bus, slot, func);
                h->cache_line_size = PCI_READ8(bus, slot, func, 0x0C);
                h->latency_timer   = PCI_READ8(bus, slot, func, 0x0D);
                h->header_type     = PCI_GET_HEADER_TYPE(bus, slot, func);
                h->bist            = PCI_READ8(bus, slot, func, 0x0F);

                // Read BARs
                h->bar0 = PCI_GET_BAR0(bus, slot, func);
                h->bar1 = PCI_GET_BAR1(bus, slot, func);
                h->bar2 = PCI_GET_BAR2(bus, slot, func);
                h->bar3 = PCI_GET_BAR3(bus, slot, func);
                h->bar4 = PCI_GET_BAR4(bus, slot, func);
                h->bar5 = PCI_GET_BAR5(bus, slot, func);

                // Other fields
                h->subsystem_vendor_id = PCI_GET_SUBSYSTEM_VENDOR_ID(bus, slot, func);
                h->subsystem_id        = PCI_GET_SUBSYSTEM_ID(bus, slot, func);
                h->capabilities_pointer = PCI_GET_CAPABILITIES_POINTER(bus, slot, func);
                h->interrupt_line      = PCI_GET_INTERRUPT_LINE(bus, slot, func);
                h->interrupt_pin       = PCI_GET_INTERRUPT_PIN(bus, slot, func);

                // Optional: you could fill expansion_rom_base, min_grant, max_latency, etc.
            }
        }
    }
}

U32 PCI_GET_DEVICE_COUNT(void) {
    return pci_device_count;
}
PCI_DEVICE_ENTRY* PCI_GET_DEVICE(U32 index) {
    if(index >= pci_device_count) return NULL;
    return &pci_devices[index];
}
PCI_DEVICE_ENTRY* PCI_GET_LIST(void) {
    return pci_devices;
}
void PCI_INITIALIZE(void) {
    PCI_ENUMERATE();
}