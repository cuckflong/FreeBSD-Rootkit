#!/bin/sh

/sbin/controller off
crontab -r -f

RESULT=`pgrep daemon`

if [ "${RESULT:-null}" != null ]; then
    killall -9 daemon
fi

rm -f /sbin/controller /bin/priv_esc /log.txt /lib/daemon /etc/install elevate
kldunload /boot/kernel/rootkit.ko
rm -f /boot/kernel/rootkit.ko
rm uninstall.sh