#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "creerDazibao.h"


void nouveauDazibao(){
    int fd;
    if((fd=open("new_Dazibao.dzb",O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH))==-1){
        if(errno==EEXIST)
            printf("Fichier déjà créé\n");
        else{
            perror("open");
        }
    }
    unsigned char magic=53,version=0;
    short int mbz=0;
    write(fd,&magic,1);
    write(fd,&version,1);
    write(fd,&mbz,2);
    close(fd);
}

