#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024


typedef struct dazib{
    char notifPath[BUFFER_SIZE];
    char pathDazibao[BUFFER_SIZE];
    int tailleNotif;
    time_t dateModif;
    int isModif;

} dazibao;

dazibao * tabDazi=NULL;
int nbDazib;

void reapChild(int sig){
    waitpid(-1,NULL,WNOHANG);

}


void checkModif(int sig){
    int i;
    struct stat bufStat;
    for(i=0;i<nbDazib;i++){
        if(stat(tabDazi[i].pathDazibao,&bufStat)==-1){
            perror("stat");
            exit(EXIT_FAILURE);
        }
        if(tabDazi[i].dateModif!=bufStat.st_mtime){
            tabDazi[i].isModif=1;
        }
        tabDazi[i].dateModif=bufStat.st_mtime;
    }


}

void parseFile(char *pathFile){
    unsigned char c;
    struct stat bufStat;
    char* buf=NULL;
    char buf2[BUFFER_SIZE];
    int fd,n=1;
    if((fd=open(pathFile,O_RDONLY))==-1){
        perror("open");
        exit(EXIT_FAILURE);
    }
    if((tabDazi=malloc(BUFFER_SIZE*sizeof(dazibao)))==NULL){
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    int i=0;
    if(stat(pathFile,&bufStat)==-1){
        perror("stat");
        exit(EXIT_FAILURE);
    }
    long int sizeFile=bufStat.st_size;
    long int pos=1;
    nbDazib=0;
    while(pos<=sizeFile){
        read(fd,&c,1);
        pos++;
        while(c!='\n' && pos<=sizeFile){
            buf2[i]=c;
            i++;
            read(fd,&c,1);
            pos++;
        }
        if(pos>=sizeFile){
            buf2[i]=c;
        }
        i++;
        buf2[i-1]='\0';
        if((buf=malloc(BUFFER_SIZE*sizeof(char)))==NULL){
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        if(realpath(buf2,buf)==NULL){
            perror("realpath");
            exit(EXIT_FAILURE);
        }
        printf("1buf : %s\n",buf);
        tabDazi[nbDazib].notifPath[0]='C';
        tabDazi[nbDazib].notifPath[1]='\0';
        strcat(tabDazi[nbDazib].notifPath,buf);
        strcpy(tabDazi[nbDazib].pathDazibao,buf);
        tabDazi[nbDazib].tailleNotif=strlen(buf)+1;
        tabDazi[nbDazib].isModif=0;
        
        if(stat(buf,&bufStat)==-1){
            perror("stat");
            exit(EXIT_FAILURE);
        }
        tabDazi[nbDazib].dateModif=bufStat.st_mtime;
        nbDazib++;
        i=0;
        free(buf);
        if(nbDazib>=BUFFER_SIZE){
            n++;
            if(realloc(tabDazi,n*BUFFER_SIZE*sizeof(dazibao))==NULL){
                perror("realloc");
                exit(EXIT_FAILURE);
            }
        }
    }
    close(fd);
}

int main(int argc, char* argv[]) {
    struct stat bufStat;
    struct sockaddr_un local, client;
    int fd, fd2, done,i;
    socklen_t socket_size;
    pid_t pid;
    char* buf=NULL;

    nbDazib=argc-1;
    if(argc<2){
        fprintf(stderr,"Au moins un argument est nécessaire\n");
        exit(EXIT_FAILURE);
    }
    if(strcmp(argv[1],"-f")==0){
        if(argc<3){
            fprintf(stderr,"Un fichier doit être donné en paramètre\n");
            exit(EXIT_FAILURE);
        }
        parseFile(argv[2]);
    }else{
        if((tabDazi=malloc(argc*sizeof(dazibao)))==NULL){
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        for(i=1;i<argc;i++){
            if((buf=malloc(BUFFER_SIZE*sizeof(char)))==NULL){
                perror("malloc");
                exit(EXIT_FAILURE);
            }
            if(realpath(argv[i],buf)==NULL){
                perror("realpath");
                exit(EXIT_FAILURE);
            }
            tabDazi[i-1].notifPath[0]='C';
            strcat(tabDazi[i-1].notifPath,buf);
            strcpy(tabDazi[i-1].pathDazibao,buf);
            tabDazi[i-1].tailleNotif=strlen(buf)+1;
            tabDazi[i-1].isModif=0;
        
            if(stat(buf,&bufStat)==-1){
                perror("stat");
                exit(EXIT_FAILURE);
            }
            tabDazi[i-1].dateModif=bufStat.st_mtime;
            free(buf);
        }
    }
    
    

    signal(SIGCHLD,reapChild);   
    signal(SIGALRM,checkModif);
    if((fd=socket(AF_UNIX,SOCK_STREAM,0))==-1){
        perror("socket");
        exit(EXIT_FAILURE);
    }
    memset(&local,0,sizeof(local));

    local.sun_family=AF_UNIX;
    strncpy(local.sun_path, getenv("HOME"), 107);
    strncat(local.sun_path, "/", 107);
    strncat(local.sun_path, ".dazibao-notification-socket", 107);
    unlink(local.sun_path);
    if(bind(fd,(struct sockaddr*)&local,sizeof(local))==-1){
        perror("bind");
        exit(EXIT_FAILURE);
    }
    if(listen(fd,10)==-1){
        perror("listen");
        exit(EXIT_FAILURE);
    }
    while(1){
        socket_size=sizeof(client);
        if((fd2=accept(fd,(struct sockaddr*)&client,&socket_size))==-1){
            perror("accept");
            continue;
        }
        if((pid=fork())<0){
            perror("fork");
            exit(EXIT_FAILURE);
        }else if(pid==0){ /* child */
            close(fd);
            for(i=0;i<nbDazib;i++){
                if(stat(tabDazi[i].pathDazibao,&bufStat)){
                    perror("stat");
                    exit(EXIT_FAILURE);
                }
                tabDazi[i].dateModif=bufStat.st_mtime;
            }
            done = 0;
            do {
                alarm(10);
                pause();
                for(i=0;i<nbDazib;i++){
                    if(tabDazi[i].isModif==1){
                        if(send(fd2,tabDazi[i].notifPath,strlen(tabDazi[i].notifPath),0)<0){
                            perror("send");
                            done=1;
                            break;
                        }
                        tabDazi[i].isModif=0;
                    }
                }
            }while(done!=1);
            close(fd2);
            break;
        }
        close(fd2);
    }
    return 0;
}
