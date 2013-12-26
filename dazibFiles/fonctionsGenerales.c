#include "globalvar.h"
#include "typedef.h"
#include <stdlib.h>
#include <stdio.h>
#include "fonctionsGenerales.h"


/* Ces fonctions ont une utilité globale dans l'ensemble du programme.
 * Elles permettent certaines petites opérations qu'il serait lourd
 * d'avoir dans le code.
*/


/* On retrouve ici les fonctions pour écrire la longueur (d'un tlv).
 * il en faut trois différentes, car elles se basent sur les valeurs précédentes 
 * pour calculer leur valeur de retour
*/
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


/* La fonction de lire lenght permettre de renvoyer un int contenant 
 * la longueur calculée à partir des paramètres a, b et c.
 * Cette fonction est fait pour le système gros boutiste
*/

int lireLenght(unsigned char a, unsigned char b, unsigned char c){
    return(256*256*a+256*b+c);
}

/* Cette fonction permet de déclarer un tlv, en initialisant ses champs
 * à 0 ou NULL, ainsi qu'en malloc-ant un pointeur que la fonction va pouvoir
 * renvoyer. L'argument permet de fixer le type du tlv.
*/

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

/* Cette fonction va permettre de free un tlv, ainsi que tous ses champs.
 * Elle va vérifier pour chaque champ à free s'il n'est pas déjà NULL, et 
 * va le free. 
 * Note : Dans ce programme, il est fait en sorte que les champs du tlv qui 
 * ne sont pas NULL sont forcement malloc-és. Un champ NULL est forcement donc
 * non malloc-é
*/
void freeTlv(tlv* tlv){
    if(tlv->tlvList!=NULL)
        free(tlv->tlvList);
    if(tlv->contenuImage!=NULL)
        free(tlv->contenuImage);
    if(tlv->pathImage!=NULL)
        free(tlv->pathImage);
    if(tlv->textOrPath!=NULL)
        free(tlv->textOrPath);
    free(tlv);
}
