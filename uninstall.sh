#!/bin/sh

controller off
rm /sbin/controller
rm /sbin/priv_esc
kldunload ./rootkit.ko
rm /boot/kernel/rootkit.ko
