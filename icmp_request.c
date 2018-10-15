#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <netinet/ip_icmp.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>

unsigned short checksum(void *b, int len);
void ping(struct sockaddr_in *addr);

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("usage: %s destination_ip\n", argv[0]);
        return 1;
    }

    struct sockaddr_in addr;
    struct in_addr dst;

    if (inet_aton(argv[1], &dst) == 0) {

        perror("inet_aton");
        printf("%s isn't a valid IP address\n", argv[1]);
        return 1;
    }


    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_addr = dst;

    ping(&addr);
    return 0;
}

unsigned short checksum(void *b, int len)
{   unsigned short *buf = b;
    unsigned int sum=0;
    unsigned short result;

    for ( sum = 0; len > 1; len -= 2 )
        sum += *buf++;
    if ( len == 1 )
        sum += *(unsigned char*)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

void ping(struct sockaddr_in *addr) {
    int sd;
    const int val=255;
    struct sockaddr_in r_addr;
    sd = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP);
    if ( sd < 0 ) {
        perror("socket");
        return;
    }
    if ( setsockopt(sd, SOL_IP, IP_TTL, &val, sizeof(val)) != 0)
        perror("Set TTL option");
    if ( fcntl(sd, F_SETFL, O_NONBLOCK) != 0 )
        perror("Request nonblocking I/O");
    socklen_t len=sizeof(r_addr);
    struct icmp *icmp_send, *icmp_recv;
    printf("lol\n");
    icmp_send->icmp_type = ICMP_ECHO;
    icmp_send->icmp_code = 0;
    icmp_send->icmp_id = getpid();
    icmp_send->icmp_seq = htons(1);
    unsigned char buff[2000];
    unsigned char* p = buff;
    p += sizeof(icmp_send);
    printf("wtf\n");
    strcpy(p, "1337");

    icmp_send->icmp_cksum = 0;
    memcpy(buff, icmp_send, sizeof(icmp_send)) ;

    icmp_send->icmp_cksum = checksum(buff, sizeof(icmp_send) + 1);
    memcpy(buff, icmp_send, sizeof(icmp_send)) ;

    if ( sendto(sd, (unsigned char*)buff, sizeof(icmp_send) + 1, 0, (struct sockaddr*)addr, sizeof(*addr)) <= 0 ) {
                printf("Send err.\n");
    }
}
