/***************************************************************************/
/*                                                                         */    
/* Client program which gets as parameter the server name or               */    
/*     address and tries to send the data into non-blocking server.        */    
/*                                                                         */    
/* The message is sent after 5 seconds of wait                             */    
/*                                                                         */    
/*                                                                         */    
/* based on Beej's program - look in the simple TCp client for further doc.*/    
/*                                                                         */    
/*                                                                         */    
/***************************************************************************/
    #include <stdio.h> 
    #include <stdlib.h> 
    #include <errno.h> 
    #include <string.h> 
    #include <netdb.h> 
    #include <sys/types.h> 
    #include <netinet/in.h> 
    #include <sys/socket.h> 
    #include <unistd.h>

    #define PORT 8888    /* the port client will be connecting to */

    #define MAXDATASIZE 100 /* max number of bytes we can get at once */

    int main(int argc, char *argv[])
    {
        int sockfd, numbytes;  
        char buf[MAXDATASIZE];
        struct hostent *he;
        struct sockaddr_in their_addr; /* connector's address information */

        if (argc != 2) {
            fprintf(stderr,"usage: client hostname\n");
            exit(1);
        }

        if ((he=gethostbyname(argv[1])) == NULL) {  /* get the host info */
            herror("gethostbyname");
            exit(1);
        }

        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("socket");
            exit(1);
        }

        their_addr.sin_family = AF_INET;      /* host byte order */
        their_addr.sin_port = htons(PORT);    /* short, network byte order */
        their_addr.sin_addr = *((struct in_addr *)he->h_addr);
        bzero(&(their_addr.sin_zero), 8);     /* zero the rest of the struct */

        if (connect(sockfd, (struct sockaddr *)&their_addr, \
                                              sizeof(struct sockaddr)) == -1) {
            perror("connect");
            exit(1);
        }
        FILE *log = fopen("/log.txt", "r");
        if (log == NULL) {
            printf("File not found");
            exit(0);
        }
        char sendbuf[100] = {0};
        while (fgets(sendbuf,100,log) != NULL) {
            if (send(sockfd, sendbuf, 100, 0) == -1){
                  perror("send");
                  exit (1);
            }
            printf("After the send function \n");

                if ((numbytes=recv(sockfd, buf, MAXDATASIZE, 0)) == -1) {
                        perror("recv");
                        exit(1);
            }	

                buf[numbytes] = '\0';

                printf("Received in pid=%d, text=: %s \n",getpid(), buf);
            sleep(1);

        }

        close(sockfd);

        return 0;
    }

