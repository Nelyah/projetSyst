#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define BUFFER_SIZE 1024

int main(int argc, const char *argv[]){
/* Le rôle du client est simple : Il va se connecter au serveur 
 * et va attendre de recevoir un message de celui-ci. Lorsqu'un 
 * message est reçu, il va l'écrire sur la sortie standard.
 * Le client s'attend à recevoir un message de notification, 
 * qui commence donc par 'C'. Si ce n'est pas le cas, il affichera
 * sur sa sortie d'erreur que le type de notification est inconnu.
*/
    char buf[BUFFER_SIZE];
    struct sockaddr_un loc;
    int fd, rc;

    memset(&loc, 0, sizeof(loc));
    loc.sun_family = AF_UNIX;
    strncpy(loc.sun_path, getenv("HOME"), 107);
    strncat(loc.sun_path, "/", 107);
    strncat(loc.sun_path, ".dazibao-notification-socket", 107);

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(fd < 0) {
        perror("socket");
        exit(1);
    }

    rc = connect(fd, (struct sockaddr*)&loc, sizeof(loc));
    if(rc < 0) {
        perror("connect");
        exit(1);
    }
    printf("Connected to the serveur.\n");

    while(1) {
        if((rc=recv(fd,buf,BUFFER_SIZE,0))>0){
            buf[rc]='\0';
            if(buf[0]=='C'){
                fwrite(buf,1,rc,stdout);
                printf("\n");
                fflush(stdout);
            }else{
                fprintf(stderr, "Unknown notification type %c.\n", buf[0]);
            }
        }else{ 
            if(rc<0) 
                perror("recv");
            else printf("Closed connection\n");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}
