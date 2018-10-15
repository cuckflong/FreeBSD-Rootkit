#include <stdio.h>
#include <fcntl.h>
#include <paths.h>
#include <string.h>
#include <sys/types.h>

#define CDEV_DEVICE "cd"
static char buf[100];

int
main(int argc, char *argv[]) {
	int kernel_fd;
	int len;

	/* Open cd_example. */
	if ((kernel_fd = open("/dev/" CDEV_DEVICE, O_RDWR)) == -1) {
		perror("/dev/" CDEV_DEVICE);
		exit(1);
	}

	/* Read from cd_example. */
	read(kernel_fd, buf, 100);
    //fread(buf, sizeof(buf), 1, kernel_fd);
    printf("%s\n", buf);

	/* Close cd_example. */
	if ((close(kernel_fd)) == -1) {
		perror("close()");
		exit(1);
	}

	exit(0);
}