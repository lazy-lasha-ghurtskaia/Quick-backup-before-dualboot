#### send regular arp packet ####
# from scapy.all import srp1, Ether, ARP, conf
#
# conf.verb = 0
# target_ip = "192.168.100.4"
# interface = "wlo1"
# 
# resp = srp1(Ether(dst="ff:ff:ff:ff:ff:ff")/ARP(pdst=target_ip), timeout=2, iface=interface)
# 
# if resp:
#     print("MAC for", target_ip, "is", resp[ARP].hwsrc)
# else:
#     print("No answer (timeout or lost)")


### get all mac addresses ###
# from scapy.all import arping, conf
# 
# conf.verg = 1
# arping("192.168.100.0/24", iface="wlo1")



# arp spoof
# from scapy.all import sendp, Ether, ARP, conf
# 
# conf.verb = 1
# arp_reply = Ether(dst="f8:d1:11:af:b8:1b")/ARP(sp=2,
#                                                hwsrc="",
#                                                psrc="",
#                                                hwdst="f8:d1:11:af:b8:1b ", 
#                                                pdst="")
# 
# #28:fb:ae:3a:b9:d2  HuaweiTechno  192.168.100.1 
# #f8:d1:11:af:b8:1b  TpLinkTechno  192.168.100.20
# sendp(arp_reply, iface="wlo1", count=3)


# from scapy.all import IP, TCP, sr1, conf
# 
# conf.verb = 0
# dst = "192.168.100.4"
# dport = 80
# 
# syn = IP(dst=dst)/TCP(dport=dport, flags="S")
# synack = sr1(syn, timeout=2)
# 
# if synack and synack.haslayer(TCP):
#     flags = synack[TCP].flags
#     if flags & 0x12:
#         print(f"Port {dport} is open. received SYN-ACK.")
#     elif flags &0x14:
#         print(f"Port {dport} is closed. received RST.")
#     else:
#         print("Received TCP flags:", flags)
# else:
#     print("No response (filtered or timeout")


# from scapy.all import IP, TCP, sr1, send, conf

# conf.verb = 0
# gateway_ip = "192.168.100.1"
# sport = 12345

# syn = IP(dst=gateway_ip)/TCP(sport=sport, dport=80, flags="S", seq=1000)
# synack = sr1(syn, timeout=2)

# if synack and synack.haslayer(TCP):
#     flags = synack[TCP].flags
#     if flags & 0x12:
#         # acknowledged. what now
#         ack_pkt = IP(dst=gateway_ip)/TCP(sport=sport, dport=80, flags="A", seq=synack.ack, ack=synack.seq+1)
#         send(ack_pkt)
#         print("acknowledgement packet sent")


# arp spoof
# forward traffic
# if we catch a dns request poison it