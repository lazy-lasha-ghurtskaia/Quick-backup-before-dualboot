#!/bin/sh
ip link set wlo1 promisc on
ip addr show wlo1 | grep PROMISC
