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
    long tailleParc=4;
    int i;
    unsigned char type=0, l1=0,l2=0,l3=0;
    unsigned int lenght=0;
    unsigned char* contenu=NULL;
    FILE* f=NULL;
    if((f=fopen(pathToDazibao,"r+b"))==NULL){
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    if(flock(fileno(f),LOCK_EX)==-1){
        perror("flock");
        exit(EXIT_FAILURE);
    }
    for(i=1;i<=num_msg;i++) {
        fseek(f,posM[i],SEEK_SET);
        if(fread(&type,1,1,f)==0){
            perror("fread");
            exit(EXIT_FAILURE);
        }
        if(fread(&l1,1,1,f)==0){
            perror("fread");
            exit(EXIT_FAILURE);
        }
        if(fread(&l2,1,1,f)==0){
            perror("fread");
            exit(EXIT_FAILURE);
        }
        if(fread(&l3,1,1,f)==0){
            perror("fread");
            exit(EXIT_FAILURE);
        }
        lenght=lireLenght(l1,l2,l3);
        if((contenu=malloc((lenght+1)*sizeof(unsigned char)))==NULL){
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        if(fread(contenu,lenght,1,f)==-1){
            perror("fread");
            exit(EXIT_FAILURE);
        }
        fseek(f,tailleParc,SEEK_SET);
        if(fwrite(&type,1,1,f)==0){
            perror("fwrite");
            exit(EXIT_FAILURE);
        }
        if(fwrite(&l1,1,1,f)==0){
            perror("fwrite");
            exit(EXIT_FAILURE);
        }
        if(fwrite(&l2,1,1,f)==0){
            perror("fwrite");
            exit(EXIT_FAILURE);
        }
        if(fwrite(&l3,1,1,f)==0){
            perror("fwrite");
            exit(EXIT_FAILURE);
        }
        if(fwrite(contenu,lenght,1,f)==0){
            perror("fwrite");
            exit(EXIT_FAILURE);
        }

        posM[i]=tailleParc;
        tailleParc+=4+lenght;
        free(contenu);
    }
    if(truncate(pathToDazibao,tailleParc)==-1){
        perror("truncate");
        exit(EXIT_FAILURE);
    }

    if(flock(fileno(f),LOCK_UN)==-1){
        perror("flock");
        exit(EXIT_FAILURE);
    }
    fclose(f);

}
