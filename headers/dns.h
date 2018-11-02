#include <stdint.h>

struct dnshdr_k {
    uint16_t t_id:16;
    
    unsigned char rd:1;
    unsigned char tc:1;
    unsigned char aa:1;
    unsigned char opcode:4;
    unsigned char qr:1;
    unsigned char rcode:4;
    unsigned char cd:1;
    unsigned char ad:1;
    unsigned char z:1;
    unsigned char ra:1;

    //uint16_t flags:16;

    uint16_t qd_count;
    uint16_t an_count;
    uint16_t ns_count;
    uint16_t ad_count;
};
