#!/bin/sh

pkg install lang/gcc
make
kldload ./rootkit.ko
gcc controller.c -o /sbin/controller

