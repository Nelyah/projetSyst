#include "globalvar.h"
#include "typedef.h"
#include <stdlib.h>
#include <stdio.h>
#include "fonctionsGenerales.h"




unsigned char ecrireLenght1(int a){
    if(a<(256*256+256)){
        return 0;
    }else {
        return (a/(256*256));
    }
}

unsigned char ecrireLenght2(int a, unsigned char l1){
    if(a<(256)){
        return 0;
    }else {
        return ((a-(256*256*l1))/256);
    }
}


unsigned char ecrireLenght3(int a, unsigned char l1, unsigned char l2){
    if(a<(256)){
        return a;
    }else {
        return (a-(256*256*l1)-(256*l2));
    }
}

int lireLenght(unsigned char a, unsigned char b, unsigned char c){
    return(256*256*a+256*b+c);
}

tlv* newTlv(int type){
    tlv* new_msg=NULL;
    if((new_msg=malloc(sizeof(tlv)))==NULL){
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    new_msg->contenuImage=NULL;
    new_msg->nbTlv=0;
    new_msg->pathImage=NULL,
    new_msg->time=0;
    new_msg->lenght=0;
    new_msg->tlvList=NULL;
    new_msg->textOrPath=NULL;
    new_msg->type=type;
    return new_msg;
}

void freeTlv(tlv* tlv){
    int i;
    for(i=0;i<tlv->nbTlv;i++){
        if(tlv->tlvList[i]!=NULL){
            free(tlv->tlvList[i]);
        }
    }
    if(tlv->tlvList==NULL)
        free(tlv->tlvList);
    if(tlv->contenuImage!=NULL)
        free(tlv->contenuImage);
    if(tlv->pathImage!=NULL)
        free(tlv->pathImage);
    if(tlv->textOrPath!=NULL)
        free(tlv->textOrPath);
    free(tlv);
}
