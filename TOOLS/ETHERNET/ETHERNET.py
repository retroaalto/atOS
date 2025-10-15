from scapy.all import *

dst_mac = "52:54:00:12:34:56"
src_mac = get_if_hwaddr("tap0")

msg = "Hello RTL8139!"
pkt = Ether(dst=dst_mac, src=src_mac)/msg.encode()
sendp(pkt, iface="tap0")
