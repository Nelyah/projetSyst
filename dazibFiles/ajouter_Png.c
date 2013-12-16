#include <stdlib.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <string.h>
#include "typedef.h"
#include "globalvar.h"
#include "fonctionsGenerales.h"
#include "ajouter_Png.h"



int ajouterMessagePng(FILE* dazibao,tlv* tlv,int hasLock){
    if (hasLock!=1) {
        if(flock(fileno(dazibao),LOCK_EX)!=0){
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
    if (fwrite(&tlv->type,1,1,dazibao)==0) {
        perror("fwrite ");
        exit(EXIT_FAILURE);
    }
    unsigned int lenght=tlv->lenght; 
    unsigned char l1=0,l2=0,l3=0;
    l1=ecrireLenght1(lenght);
    l2=ecrireLenght2(lenght,l1);
    l3=ecrireLenght3(lenght,l1,l2);
    if (fwrite(&l1,1,1,dazibao)==0) {
        perror("fwrite :");
        exit(EXIT_FAILURE);
    }
    if (fwrite(&l2,1,1,dazibao)==0) {
        perror("fwrite :");
        exit(EXIT_FAILURE);
    }
    if (fwrite(&l3,1,1,dazibao)==0) {
        perror("fwrite :");
        exit(EXIT_FAILURE);
    }
    if(fwrite(tlv->contenuImage,tlv->lenght,1,dazibao)==0){
        perror("fwrite");
        exit(EXIT_FAILURE);
    }
    gtk_text_buffer_insert_pixbuf(buf,&end, pix);
    gtk_text_buffer_insert(buf, &end, "\n", -1);
    if (hasLock!=1) {
        if(flock(fileno(dazibao),LOCK_UN)!=0){
            perror("flock : ");
            exit(EXIT_FAILURE);
        }
    }
    freeTlv(tlv);
    return 0;
}



void ajouter_pngN(){
    ajouter_png(0);
}

tlv* ajouter_png(int opt){
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
    unsigned char* fmap=mmap(NULL,buf.st_size,PROT_READ,MAP_PRIVATE,fd,0);
    if(fmap==MAP_FAILED){
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    memcpy(tlvPng->contenuImage,fmap,buf.st_size);
    if(munmap(fmap,buf.st_size)==-1){
        perror("munmap");
        exit(EXIT_FAILURE);
    }
    if(close(fd)==-1){
        perror("close");
        exit(EXIT_FAILURE);
    }
    if((tlvPng->pathImage=malloc((strlen(filename)+1)*sizeof(char)))==NULL){
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    strcpy(tlvPng->pathImage,filename);

    if(opt==0){
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
        FILE* dazibao=NULL;
        if((dazibao=fopen(pathToDazibao,"a+b"))==NULL){
            perror("fopen");
            exit(EXIT_FAILURE);
        }
        struct stat statBuf;
        if(stat(pathToDazibao,&statBuf)==-1){
            perror("stat");
            exit(EXIT_FAILURE);
        }
        posM[num_msg]=statBuf.st_size;
        ajouterMessagePng(dazibao,tlvPng, 0);
        fclose(dazibao);
        return NULL;
    }else{
        return tlvPng;
    }
    return NULL;


}
