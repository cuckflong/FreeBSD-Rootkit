#!/bin/sh

controller off
rm /sbin/controller
rm /sbin/priv_esc
rm /log.txt
rm /remote.py
kldunload ./rootkit.ko
rm /boot/kernel/rootkit.ko
