**FreeBSD Rootkit Documentation**

**How the rootkit installs itself**

The main functions of the rootkit are inside the kernel module rootkit.ko and a system call is created to activate the different functions implemented, then a controller binary is installed in /sbin which will do the system call to rootkit with the proper arguments. The controller currently only have two options on or off but it is designed so that in future we can specify a process, file or port to hide just by calling it without rewriting the rootkit kernel module.

**How the rootkit provide**  **the privilege escalation functionality**

During installation, a setuid binary is installed in /sbin and is hidden by the rootkit so it won&#39;t be listed with ls and find. At first we were thinking of hooking the execve system call so that we can do a execution redirection from utility functions such as /usr/bin/true to the setuid binary but we realised it is actually the same thing as just hiding it and is pointless to do the extra step so to elevate, we will just call it with it&#39;s absolute path /sbin/priv\_esc.

**How the rootkit attempts to hide itself**

For the current rootkit, since we don&#39;t have any running process or port, we only hide the files and binaries we used from being listed in ls and find, we also prevented the kernel module from listing in kldstat.

To hide the files and binaries from ls and find, we hook the system call getdirentries with our own function which will call getdirentries first but instead of returning right after that, we will go through the directory entries it returned and check if there were any files or binaries we don&#39;t want to show. If any of them match, we&#39;ll cut it out by overwriting it with the next entry and finally changing the transferred size as we removed the entries

To hide the kernel module from kldstat, it is done by remove the linker\_file and module struct from the linker\_files and modules lists, we used the different macro functions to go through the list and remove them when a match is found.

**Extra Functionality**

**Other than reboot persistence, the remaining functionalities are partly implemented but not included yet since they have bugs yet to be fixed.**



**Key Logging**

The system call &#39;read&#39; is hooked within the kernel module and every character that is inputted from stdin is checked. If the character is a keystroke, the single character is then copied over from kernel space into user space.

The file &#39;log.txt&#39; located at /root is used to store these keystrokes. The file is opened with kern\_openat (opened to the current thread), with read/write, create and append flags with permissions 0666. The character is first stored inside a local array, and using the structs of IOVEC (contains the starting address of buffer, number of bytes to transfer) and UIO (contains information on where to write to from within kernel space). The character is then written with the use of  kern\_writev, writing from the UIO struct, to the file descriptor (which was set to the current thread by kern\_openat).

This functionality can be turned on with the use of &#39;controller on&#39; and &#39;controller off&#39; after the rootkit is installed and can log input from a remote ssh as well.

Known bugs:

- If the first time running &#39;controller on&#39; is as a root user, the log.txt file has permission errors later on when switching over to a normal privilege user. It will return permission errors when trying to open &quot;log.txt&quot; whenever an input is given to stdin

**Reboot Persistence**

To make the rootkit boot persistence, the rootkit.ko is copied to /boot/kernel and &quot;rootkit\_load=&quot;YES&quot;&quot; is added to the /boot/loader.conf so that upon boot, the rootkit kernel module will be loaded automatically.

**Reverse Shell and Exfiltrating The Log File To External Network**

To achieve these two functionalities, we planned to use an ICMP packet with a magic sequence in the payload as a trigger by hooking the icmp\_input, then we will write the corresponding option and command to a character device. Then have another running process constantly checking it and make a reverse shell or send the file out depending on what the option and command are.

It is not included yet since there is a fatal error in using the character device and is yet to be fixed.