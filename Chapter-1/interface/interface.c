#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/module.h>

int main(int argc, char *argv[]) {
	int syscall_num;
	struct module_stat stat;

	if (argc != 2) {
		printf("Usage:\n%s <string>\n", argv[0]);
		exit(0);
	}

	stat.version = sizeof(stat);
	printf("Sys call num: %d\n", modfind("via.ko"));
	modstat(210, &stat);
	syscall_num = stat.data.intval;
	printf("Calling %d\n", syscall_num);
	return(syscall(210, argv[1]));
}
