## A FreeBSD kernel rootkit and detector written in C and Golang

# Features
- Reboot Persistence
- File and Kernel Stat Hiding
- Keylogging and File Exfiltration
- Reverse Shell 

# Methodology
A kernel module is loaded which hook the syscalls and make changes we need, it will communicate with a background program written in Golang via a block device to achieve certain features above. It will also look for ICMP packets with a magic sequence in order to perform actions to external network such as reverse shell and log file exfiltration.

# Report
For more details please read  
[Final Report](rootkit-writeup.pdf)