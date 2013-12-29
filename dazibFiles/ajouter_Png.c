#include <stdlib.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <string.h>
#include "typedef.h"
#include "fonctionsGenerales.h"
#include "globalvar.h"
#include "ajouter_Png.h"


int ajouterMessagePng(int fd,tlv* tlv, int hasLock){
/* Cette fonction permet d'écrire le tlv donné en argument dans le fichier dazibao.
 * Les arguments sont un FILE*, un pointeur vers le tlv rempli, et un flag
 * pour savoir si il y a eu un lock sur le fichier
*/
    if (hasLock!=1) {
        if(flock(fd,LOCK_EX)!=0){
            perror("flock : ");
            exit(EXIT_FAILURE);
        }
    }
    GError *error = NULL;
    GdkPixbuf *pix;
    GtkTextIter end;
    GtkTextBuffer *buf;
    buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pTextView));
    gtk_text_view_set_wrap_mode((GTK_TEXT_VIEW(pTextView)),GTK_WRAP_WORD);
    gtk_text_view_set_justification((GTK_TEXT_VIEW(pTextView)),GTK_JUSTIFY_CENTER);
    gtk_text_buffer_get_end_iter(buf,&end);
    pix = gdk_pixbuf_new_from_file_at_size(tlv->pathImage,400,400, &error);
    if (error){
	    GtkWidget *msg;
	    msg = gtk_message_dialog_new (GTK_WINDOW (pWindow2), 
	                                    GTK_DIALOG_MODAL,
				                        GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, 
				                        error->message);
	    gtk_dialog_run (GTK_DIALOG (msg));
	    gtk_widget_destroy (msg);
	    exit(EXIT_FAILURE);
    }


    if(tlv->lenght>16777216) { // taille max d'un entier sur 24 bits unsigned
        printf("Erreur : La taille de l'image est trop grande \n");
        exit(EXIT_FAILURE);
    }
    // On écrit le type du tlv
    if (write(fd,&tlv->type,1)==0) {
        perror("write");
        exit(EXIT_FAILURE);
    }
    // On écrit la longueur du tlv
    unsigned int lenght=tlv->lenght; 
    unsigned char l1=0,l2=0,l3=0;
    l1=ecrireLenght1(lenght);
    l2=ecrireLenght2(lenght,l1);
    l3=ecrireLenght3(lenght,l1,l2);
    if (write(fd,&l1,1)==0) {
        perror("write");
        exit(EXIT_FAILURE);
    }
    if (write(fd,&l2,1)==0) {
        perror("write :");
        exit(EXIT_FAILURE);
    }
    if (write(fd,&l3,1)==0) {
        perror("write :");
        exit(EXIT_FAILURE);
    }
    // On écrit le contenu de l'image qui était stocké dans un unsigned char*
    if(write(fd,tlv->contenuImage,tlv->lenght)==0){
        perror("write");
        exit(EXIT_FAILURE);
    }
    // Affichage sur l'interface graphique
    gtk_text_buffer_insert_pixbuf(buf,&end, pix);
    gtk_text_buffer_insert(buf, &end, "\n", -1);
    if (hasLock!=1) {
        if(flock(fd,LOCK_UN)!=0){
            perror("flock : ");
            exit(EXIT_FAILURE);
        }
    }
    freeTlv(tlv);
    return 0;
}


void ajouter_pngN(){
/* Cette fonction est appelée uniquement par l'interface graphique.
 * Elle va appeler la fonction ajouter_png(int opt) en spécifiant qu'elle
 * n'aura pas de parent
*/
    ajouter_png(0);
}

tlv* ajouter_png(int opt){
/* Cette fonction a comme but de remplir un tlv png et de le renvoyer si
 * la fonction a un parent, ou d'appeler directement la fonciton pour l'écrire sur
 * le fichier dazibao sinon.
 * Elle renvoie NULL si il y a eu une erreur ou que l'utilisateur annule.
*/

// On récupère le chemin vers l'image que l'on veut afficher
    char* filename=NULL;
    GtkWidget *selection;
    selection = gtk_file_chooser_dialog_new( g_locale_to_utf8( "Sélectionnez un fichier", -1, NULL, NULL, NULL),
                                            GTK_WINDOW(pWindow2), GTK_FILE_CHOOSER_ACTION_OPEN,
                                            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				                            GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                            NULL);

    if(gtk_dialog_run(GTK_DIALOG(selection))==GTK_RESPONSE_ACCEPT)
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER (selection));
    if(gtk_dialog_run(GTK_DIALOG(selection))==GTK_RESPONSE_CANCEL){
        gtk_widget_destroy(selection);
        return NULL;
    }
 
    dialog = gtk_message_dialog_new(GTK_WINDOW(selection),GTK_DIALOG_MODAL,
	    GTK_MESSAGE_INFO,GTK_BUTTONS_OK,"Vous avez choisi :\n%s", filename);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    gtk_widget_destroy(selection);
    // On ouvre le fichier de l'image
    int fd;
    if((fd=open(filename,O_RDONLY))==-1){
        perror("open");
        exit(EXIT_FAILURE);
    }
    struct stat buf;
    if(fstat(fd,&buf)==-1){
        perror("fstat");
        exit(EXIT_FAILURE);
    }
    tlv* tlvPng=NULL;
    tlvPng=newTlv(4);
    if((tlvPng->contenuImage=malloc((buf.st_size+1)*sizeof(unsigned char)))==NULL){
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    tlvPng->lenght=buf.st_size;
    // On utilise mmap pour faire une map_private de l'image
    unsigned char* fmap=mmap(NULL,buf.st_size,PROT_READ,MAP_PRIVATE,fd,0);
    if(fmap==MAP_FAILED){
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    // On utilise memcpy pour copier notre map_private dans notre pointeur contenuImage
    memcpy(tlvPng->contenuImage,fmap,buf.st_size);
    if(munmap(fmap,buf.st_size)==-1){
        perror("munmap");
        exit(EXIT_FAILURE);
    }
    if(close(fd)==-1){
        perror("close");
        exit(EXIT_FAILURE);
    }
    // On enregistre le path vers l'image
    if((tlvPng->pathImage=malloc((strlen(filename)+1)*sizeof(char)))==NULL){
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    strcpy(tlvPng->pathImage,filename);

    if(opt==0){ // Si la fonction n'a pas de parents
        num_msg++;
        char* num_msg2=NULL;
        if((num_msg2=malloc(10*sizeof(char)))==NULL){
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        sprintf(num_msg2,"%d",num_msg);
        GtkTextIter end;
        GtkTextBuffer *buf2;
        buf2 = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pTextView));
        gtk_text_view_set_wrap_mode((GTK_TEXT_VIEW(pTextView)),GTK_WRAP_WORD);
        gtk_text_view_set_justification((GTK_TEXT_VIEW(pTextView)),GTK_JUSTIFY_CENTER);
        gtk_text_buffer_get_end_iter(buf2,&end);
        gtk_text_buffer_insert(buf2, &end, num_msg2, -1);
        gtk_text_buffer_insert(buf2, &end, "------------------------------------------------------------------------------------------------------------------------------------\n", -1);
        free(num_msg2);
        int fd;
        if((fd=open(pathToDazibao,O_RDWR|O_APPEND))==-1){
            perror("open");
            exit(EXIT_FAILURE);
        }
        struct stat statBuf;
        if(fstat(fd,&statBuf)==-1){
            perror("fstat");
            exit(EXIT_FAILURE);
        }
        posM[num_msg]=statBuf.st_size;
        ajouterMessagePng(fd,tlvPng, 0);
        close(fd);
        return NULL;
    }else{
        return tlvPng;
    }
    return NULL;


}
