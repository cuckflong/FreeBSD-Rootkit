#!/bin/sh

controller off
killall -9 daemon
rm /sbin/controller
rm /bin/priv_esc
rm /log.txt
rm /sbin/daemon
rm /etc/rc.d/login
kldunload ./rootkit.ko
rm /boot/kernel/rootkit.ko
