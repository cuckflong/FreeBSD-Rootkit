#include <fcntl.h>
#include <kvm.h>
#include <limits.h>
#include <nlist.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/sysent.h>
#include "sys_list_file.h"

int checkCall(char * callname, int callnum);
void usage();

int
main(int argc, char *argv[])
{
    int count = 0;
    for (int i = 0; i < 393; i++) {
        int ret = checkCall(syscall_list[i].callName, syscall_list[i].callNumber);
        if (ret == 1 || ret == 2) {
			count++;
		}
    }
    if (count > 0) {
        return 1;
    }
    printf("count = %d\n", count);
	return 0;
}

int checkCall( char * callname, int callnum){
	char errbuf[_POSIX2_LINE_MAX];
	kvm_t *kd;
	kd = kvm_openfiles(NULL, NULL, NULL, O_RDWR, errbuf);
	if (!kd) {
		fprintf(stderr, "ERROR: %s\n", errbuf);
		exit(-1);
	}

	struct nlist nl[] = { { NULL }, { NULL }, { NULL }, };

	unsigned long addr;
	struct sysent call;
	nl[0].n_name = "sysent";
	nl[1].n_name = callname;

	/* Find the address of sysent[] and callname. */
	if (kvm_nlist(kd, nl) < 0) {
		fprintf(stderr, "ERROR: %s\n", kvm_geterr(kd));
		exit(-1);
	}

    // Address of sysent not found - system error
	if (!nl[0].n_value){
		exit(-1);
	}

	if (!nl[1].n_value) {
        exit(-1);
	}

	/* Determine the address of sysent[callnum]. */
	addr = nl[0].n_value + callnum * sizeof(struct sysent);

	/* Copy sysent[callnum]. */
	if (kvm_read(kd, addr, &call, sizeof(struct sysent)) < 0) {
        printf("%d %s\n",callnum,callname);
        return -1;
	}

	/* Check if that's correct. */
	if ((uintptr_t)call.sy_call != nl[1].n_value) {
		printf("ALERT! %d: %s should point to 0x%lx instead of 0x%lx\n", callnum, callname, nl[1].n_value, (uintptr_t)call.sy_call);
        return 2;
	}

	if (kvm_close(kd) < 0) {
		fprintf(stderr, "ERROR: %s\n", kvm_geterr(kd));
		exit(-1);
	}
    return 0;

}
