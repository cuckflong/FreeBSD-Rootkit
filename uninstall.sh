#!/bin/sh

controller off
rm /sbin/controller
rm /sbin/priv_esc
rm /log.txt
rm /remote.py
rm /sbin/daemon
rm /etc/rc.d/login
kldunload ./rootkit.ko
rm /boot/kernel/rootkit.ko
