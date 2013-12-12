typedef struct tlv{
    unsigned int lenght;
    unsigned char type;
    char *textOrPath;
    unsigned char* contenuImage;
    char* pathImage;
    int nbTlv;
    struct tlv **tlvList;
    long time;
} tlv;

