#!/bin/sh
ip link set wlo1 promisc off
ip addr show wlo1 | grep UP
