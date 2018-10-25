struct dnshdr {
    u_int16_t t_id;
    
    u_int16_t qr:1;
    u_int16_t opcode:4;
    u_int16_t aa:1;
    u_int16_t tc:1;
    u_int16_t rd:1;
    u_int16_t ra:1;
    u_int16_t z:1;
    u_int16_t ad:1;
    u_int16_t cd:1;
    u_int16_t rcode:4;

    u_int16_t qd_count;
    u_int16_t an_count;
    u_int16_t ns_count;
    u_int16_t ad_count;
};
