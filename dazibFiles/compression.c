#include <stdlib.h>
#include <sys/mman.h>
#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>
#include <signal.h>
#include "globalvar.h"
#include "fonctionsGenerales.h"
#include "compression.h"


void compression(){
/* Cette fonction a pour but de réduire la taille du fichier
 * Elle le fait en supprimant les tlv pad 1 ou Pad N
 * Elle va se baser sur le fait que tous les messages non Pad1 ou padN
 * ont eu leur position dans le fichier notée dans le tableau posM.
 * Cette fonction va juste réécrire chacun des messages répertoriés dans posM
 * en partant du début du fichier (après les 4 premiers octets) en écrasant
 * donc les pad1 et pad N puis va tronquer le fichier une fois que 
 * tous les messages ont été écris.
*/
    long tailleParc=4;
    int i;
    unsigned char type=0, l1=0,l2=0,l3=0;
    unsigned int lenght=0;
    unsigned char* contenu=NULL;
    int fd;
    if((fd=open(pathToDazibao,O_RDWR))==-1){
        perror("open");
        exit(EXIT_FAILURE);
    }
    // On pose un verrou sur le fichier
    if(flock(fd,LOCK_EX)==-1){
        perror("flock");
        exit(EXIT_FAILURE);
    }

    // Pour chaque tlv enregistré
    for(i=1;i<=num_msg;i++) {
    // On se place à sa position
    // Et on stock ses caractéristiques
        lseek(fd,posM[i],SEEK_SET);
        if(read(fd,&type,1)==0){
            perror("read");
            exit(EXIT_FAILURE);
        }
        if(read(fd,&l1,1)==0){
            perror("read");
            exit(EXIT_FAILURE);
        }
        if(read(fd,&l2,1)==0){
            perror("read");
            exit(EXIT_FAILURE);
        }
        if(read(fd,&l3,1)==0){
            perror("read");
            exit(EXIT_FAILURE);
        }
        lenght=lireLenght(l1,l2,l3);
        if((contenu=malloc((lenght+1)*sizeof(unsigned char)))==NULL){
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        if(read(fd,contenu,lenght)==-1){
            perror("read");
            exit(EXIT_FAILURE);
        }
        // On se place maintenant à la taille qu'on a parcourue jusqu'à présent.
        lseek(fd,tailleParc,SEEK_SET);
        // On réécrit ses caractéristiques
        if(write(fd,&type,1)==0){
            perror("write");
            exit(EXIT_FAILURE);
        }
        if(write(fd,&l1,1)==0){
            perror("write");
            exit(EXIT_FAILURE);
        }
        if(write(fd,&l2,1)==0){
            perror("write");
            exit(EXIT_FAILURE);
        }
        if(write(fd,&l3,1)==0){
            perror("write");
            exit(EXIT_FAILURE);
        }
        if(write(fd,contenu,lenght)==0){
            perror("write");
            exit(EXIT_FAILURE);
        }
        // On remet à jour sa nouvelle position
        posM[i]=tailleParc;
        // On augmente la taille parcourue 
        tailleParc+=4+lenght;
        free(contenu);
    }
    // Un fois que tout est réécrit, on tronque le ficher
    if(truncate(pathToDazibao,tailleParc)==-1){
        perror("truncate");
        exit(EXIT_FAILURE);
    }

    if(flock(fd,LOCK_UN)==-1){
        perror("flock");
        exit(EXIT_FAILURE);
    }
    close(fd);

}
