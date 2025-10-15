#ifndef RTL8139_DRIVER_H
#define RTL8139_DRIVER_H
#include <STD/TYPEDEF.h>

BOOLEAN RTL8139_START();
BOOLEAN RTL8139_STATUS();
BOOLEAN RTL8139_STOP(); // NOTE: This will ALWAYS return FALSE

U32 BUILD_ETH_FRAME(U8 *buf, const U8 *dst, const U8 *src, U16 ether_type, const U8 *payload, U32 payload_len);

/*
U8 frame_buf[1520];
    U8 dst[6] = {0x52,0x54,0x00,0x12,0x34,0x56};
    U8 src[6];
    MEMCPY_OPT(src, MAC, 6); // read earlier with READ_RTL8139_MAC()
    const char *msg = "Hello RTL8139!";

    // EtherType IPv4 or choose 0x888E/0x1234
    U32 frame_len = BUILD_ETH_FRAME(frame_buf, dst, src, 0x0800 , (const U8*)msg, (U32)strlen(msg));
    if (!SEND_RTL8139_PACKET(frame_buf, frame_len)) {
        // no free transmit descriptors — retry later or return error
    }
*/
BOOL SEND_RTL8139_PACKET(U8 *packet, U32 len);
void RTL8139_HANDLER(U32 vec, U32 errno);

#endif // RTL8139_DRIVER_H