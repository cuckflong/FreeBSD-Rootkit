#!/bin/sh

controller off
rm /sbin/controller
kldunload ./rootkit.ko
