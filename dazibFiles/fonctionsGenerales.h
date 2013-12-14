#ifndef DEF_TYPEDEF
#define DEF_TYPEDEF
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

#endif
unsigned char ecrireLenght1(int a);
unsigned char ecrireLenght2(int a, unsigned char l1);
unsigned char ecrireLenght3(int a, unsigned char l1, unsigned char l2);
int lireLenght(unsigned char a, unsigned char b, unsigned char c);
tlv* newTlv(int type);
void freeTlv(tlv* tlv);
