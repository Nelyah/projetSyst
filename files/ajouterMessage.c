#include <stdlib.h>
#include <sys/mman.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
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
#include "ajouterMessage.h"



int ajouterMessageDated(FILE *dazibao,tlv* tlv,int hasLock){
printf("coucou\n");
    int fd=fileno(dazibao);
    GtkTextIter end;
    GtkTextBuffer *buf;
    buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pTextView));
    gtk_text_view_set_wrap_mode((GTK_TEXT_VIEW(pTextView)),GTK_WRAP_WORD);
    gtk_text_view_set_justification((GTK_TEXT_VIEW(pTextView)),GTK_JUSTIFY_CENTER);
    gtk_text_buffer_get_end_iter(buf,&end);
    int err;
    if(hasLock!=1) {
        if((err=flock(fd,LOCK_EX))!=0){
            perror("flock ");
            exit(EXIT_FAILURE);
        }
    }
    if((err=fwrite(&tlv->type,1,1,dazibao))==0) {
        perror("fwrite :");
        exit(EXIT_FAILURE);
    }
printf("coucou\n");
    unsigned int lenght=tlv->lenght;
    unsigned char l1=0,l2=0,l3=0;
    l1=ecrireLenght1(lenght);
    l2=ecrireLenght2(lenght,l1);
    l3=ecrireLenght3(lenght,l1,l2);
    if ((err=fwrite(&l1,1,1,dazibao))==0) {
        perror("fwrite :");
        exit(EXIT_FAILURE);
    }
    if ((err=fwrite(&l2,1,1,dazibao))==0) {
        perror("fwrite :");
        exit(EXIT_FAILURE);
    }
    if ((err=fwrite(&l3,1,1,dazibao))==0) {
        perror("fwrite :");
        exit(EXIT_FAILURE);
    }
    unsigned int t1=0,t2=0,t3=0,t4=0;
    if(tlv->time<(256*256*256+256*256+256)){
        t1=0;
    }else{
        t1=tlv->time/(256*256*256);
    }
    if(tlv->time<(256*256+256)){
        t2=0;
    }else{
        t2=(tlv->time-(256*256*256*t1))/(256*256);
    }
    if(tlv->time<(256)){
        t3=0;
    }else{
        t3=(tlv->time-(256*256*256*t1)-(256*256*t2))/(256);
    }
    if(tlv->time<(256)){
        t4=tlv->time;
    }else{
        t4=(tlv->time-(256*256*256*t1)-(256*256*t2)-(256*t3));
    }
    gtk_text_buffer_insert(buf, &end, g_locale_to_utf8(ctime(&tlv->time), -1, NULL, NULL, NULL), -1);
    if((err=fwrite(&t1,1,1,dazibao))==0) {
        perror("fwrite :");
        exit(EXIT_FAILURE);
    }
printf("coucou\n");
    if((err=fwrite(&t2,1,1,dazibao))==0) {
        perror("fwrite :");
        exit(EXIT_FAILURE);
    }
    if((err=fwrite(&t3,1,1,dazibao))==0) {
        perror("fwrite :");
        exit(EXIT_FAILURE);
    }
    if((err=fwrite(&t4,1,1,dazibao))==0) {
        perror("fwrite :");
        exit(EXIT_FAILURE);
    }
printf("coucou\n");
    if(tlv->tlvList[0]->type==2) {
        gtk_text_buffer_insert(buf, &end, g_locale_to_utf8(tlv->tlvList[0]->textOrPath, -1, NULL, NULL, NULL), -1);
        gtk_text_buffer_insert(buf, &end, g_locale_to_utf8("\n", -1, NULL, NULL, NULL), -1);
        ajouterMessageTxt(dazibao,tlv->tlvList[0],1);
    }else if(tlv->tlvList[0]->type==3){
        ajouterMessagePng(dazibao,tlv->tlvList[0],1);
    }else if(tlv->tlvList[0]->type==4){
        ajouterMessageJpeg(dazibao,tlv->tlvList[0],1);
    }else if(tlv->tlvList[0]->type==5){ // Ne pas faire pour l'instant
        ajouterMessageCompound(dazibao,tlv->tlvList[0],1);
    }else if(tlv->tlvList[0]->type==6){
        ajouterMessageDated(dazibao,tlv->tlvList[0],1);
    }
    if(hasLock!=1) {
        if((err=flock(fd,LOCK_UN))!=0){
            perror("flock ");
            exit(EXIT_FAILURE);
        }
    }
printf("coucou\n");
printf("coucoui\n");
    freeTlv(tlv);
printf("coucouf\n");
    return 0;
}


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
    printf("tlvL= %d\n",tlv->lenght);
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


int ajouterMessageJpeg(FILE* dazibao,tlv* tlv, int hasLock){
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
    printf("tlvL= %d\n",tlv->lenght);
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


int ajouterMessageTxt(FILE* dazibao,tlv* tlv,int hasLock){
    int err;
    printf("filed %d\n",fileno(dazibao));
    if (hasLock!=1) {
        if((err=flock(fileno(dazibao),LOCK_EX))!=0){
            perror("flock : ");
            exit(EXIT_FAILURE);
        }
    }
    if(tlv->lenght>16777216) { // taille max d'un entier sur 24 bits unsigned
        printf("Erreur : Le message est trop long \n");
        exit(EXIT_FAILURE);
    }
    if((err=fwrite(&tlv->type,1,1,dazibao))==0) {
        perror("fwrite :");
        exit(EXIT_FAILURE);
    }
    unsigned int lenght=tlv->lenght; 
    unsigned char l1=0,l2=0,l3=0;
    l1=ecrireLenght1(lenght);
    l2=ecrireLenght2(lenght,l1);
    l3=ecrireLenght3(lenght,l1,l2);
    printf("lenght:%d, tlvLen=%d l1=%d l2=%d l3=%d\n",lenght,tlv->lenght,l1,l2,l3);
    if ((err=fwrite(&l1,1,1,dazibao))==0) {
        perror("fwrite :");
        exit(EXIT_FAILURE);
    }
    if ((err=fwrite(&l2,1,1,dazibao))==0) {
        perror("fwrite :");
        exit(EXIT_FAILURE);
    }
    if ((err=fwrite(&l3,1,1,dazibao))==0) {
        perror("fwrite :");
        exit(EXIT_FAILURE);
    }
    printf("texte : %s\n",tlv->textOrPath);
    if((err=fwrite(tlv->textOrPath,tlv->lenght,1,dazibao))==0) {
        perror("fwrite :");
        exit(EXIT_FAILURE);
    }
    printf("err=%d\n%d\n",err,tlv->lenght);
    if(hasLock!=1) {
        if((err=flock(fileno(dazibao),LOCK_UN))!=0){
            perror("flock : ");
            exit(EXIT_FAILURE);
        }
    }
    freeTlv(tlv);
    printf("pouet fin txt\n");
    return 0;
}





void ajouter_datedN(){
    ajouter_dated(0);
}

tlv* ajouter_dated(int opt){
    GtkWidget* pBoite;
    GtkWidget* pEntry;
    const gchar* type;
    int len,i,valid;
    char c;
    time_t now;
    tlv* tlvDated=NULL;


    pBoite = gtk_dialog_new_with_buttons("Choix du dated",
				       GTK_WINDOW(pWindow2),
				       GTK_DIALOG_MODAL,
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       NULL);
    pEntry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(pEntry), "Entrez le type du TLV voulu");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(pBoite)->vbox), pEntry, TRUE, FALSE, 0);
    gtk_widget_show_all(GTK_DIALOG(pBoite)->vbox);
    switch (gtk_dialog_run(GTK_DIALOG(pBoite))){
      /* L utilisateur valide */
        case GTK_RESPONSE_OK:
            type = gtk_entry_get_text(GTK_ENTRY(pEntry));
            len = strlen(type);
      
            for(i = 0; i < len; i++){
	            c = type[i];
	
	            if ((c < '0') || (c > '9')){
	                valid = 0;
	                break;
	            }
            }
      
            if(valid == 0){
	            printf("La chaine n'est pas un nombre entier.\n");
	            dialog = gtk_message_dialog_new(GTK_WINDOW(pBoite),GTK_DIALOG_MODAL,
					GTK_MESSAGE_ERROR,GTK_BUTTONS_OK,"Vous devez saisir un nombre entier!");
     
	            gtk_dialog_run(GTK_DIALOG(dialog));
	            gtk_widget_destroy(dialog);
	            gtk_widget_destroy(pBoite);
            }else{
	            int n=atol(type);
	            if(n > 1 && n <= TLV_MAX){
	                printf("type choisi : %d\n", n);
                    tlv* tlv=NULL;
                    switch (n) {
                        case 2 :
                            tlvDated=newTlv(6);
                            tlvDated->nbTlv=1;
                            if((tlvDated->tlvList=malloc(2*sizeof(struct tlv*)))==NULL){
                                perror("malloc");
                                exit(EXIT_FAILURE);
                            }
                            time(&now);
                            tlvDated->time=now;
                            tlv=ajouter_texte(2);
                            tlvDated->lenght=tlv->lenght+4+4; //+4 pour le tlv, +4 pour le time
                            tlvDated->tlvList[0]=tlv;
                            break;
                        case 3 :
                            tlvDated=newTlv(6);
                            if((tlvDated->tlvList=malloc(2*sizeof(struct tlv*)))==NULL){
                                perror("malloc");
                                exit(EXIT_FAILURE);
                            }
                            tlvDated->tlvList[0]=ajouter_png(3);
                            time(&now);
                            tlvDated->time=now;
                            tlvDated->nbTlv=1;
                            tlvDated->lenght=tlvDated->tlvList[0]->lenght+4+4;
                            break;
                        case 4 : 
                            tlvDated=newTlv(6);
                            if((tlvDated->tlvList=malloc(2*sizeof(struct tlv*)))==NULL){
                                perror("malloc");
                                exit(EXIT_FAILURE);
                            }
                            tlvDated->tlvList[0]=ajouter_jpeg(4);
                            time(&now);
                            tlvDated->time=now;
                            tlvDated->nbTlv=1;
                            tlvDated->lenght=tlvDated->tlvList[0]->lenght+4+4;
                            break;
                        case 5 :
                            tlvDated=newTlv(6);
                            if((tlvDated->tlvList=malloc(2*sizeof(struct tlv*)))==NULL){
                                perror("malloc");
                                exit(EXIT_FAILURE);
                            }
                            tlvDated->tlvList[0]=ajouter_compound(5);
                            printf("On y arrive?\n");
                            time(&now);
                            tlvDated->time=now;
                            tlvDated->nbTlv=1;
                            tlvDated->lenght=tlvDated->tlvList[0]->lenght+4+4;
                            break;
                        case 6 : 
                            tlvDated=newTlv(6);
                            if((tlvDated->tlvList=malloc(2*sizeof(struct tlv*)))==NULL){
                                perror("malloc");
                                exit(EXIT_FAILURE);
                            }
                            tlvDated->tlvList[0]=ajouter_dated(6);
                            time(&now);
                            tlvDated->time=now;
                            tlvDated->nbTlv=1;
                            tlvDated->lenght=tlvDated->tlvList[0]->lenght+4+4;
                            break;
                    }
                    gtk_widget_destroy(pBoite);	

                }else {
	                printf("Vous devez rentrer un nombre compris dans les types de TLV");
	                dialog = gtk_message_dialog_new(GTK_WINDOW(pBoite),GTK_DIALOG_MODAL,
					GTK_MESSAGE_ERROR,GTK_BUTTONS_OK,"Vous devez rentrer un nombre compris dans les types de TLV");
     
                    gtk_dialog_run(GTK_DIALOG(dialog));
                    gtk_widget_destroy(dialog);
                    gtk_widget_destroy(pBoite);
                }
            }

            break;
      /* L utilisateur annule */
        case GTK_RESPONSE_CANCEL:
        case GTK_RESPONSE_NONE:
        default:
            printf("Vous n'avez rien saisi !");
            dialog = gtk_message_dialog_new(GTK_WINDOW(pBoite),GTK_DIALOG_MODAL,
					  GTK_MESSAGE_ERROR,GTK_BUTTONS_OK,"Vous n'avez rien saisi!");
     
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            gtk_widget_destroy(pBoite);
            break;
    }
    if(opt==0){
        num_msg++;
        char* num_msg2=NULL;
        if((num_msg2=malloc(10*sizeof(char)))==NULL){
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        sprintf(num_msg2,"%d",num_msg);
        GtkTextIter end;
        GtkTextBuffer *buf;
        buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pTextView));
        gtk_text_view_set_wrap_mode((GTK_TEXT_VIEW(pTextView)),GTK_WRAP_WORD);
        gtk_text_view_set_justification((GTK_TEXT_VIEW(pTextView)),GTK_JUSTIFY_CENTER);
        gtk_text_buffer_get_end_iter(buf,&end);
        gtk_text_buffer_insert(buf, &end, num_msg2, -1);
        gtk_text_buffer_insert(buf, &end, "------------------------------------------------------------------------------------------------------------------------------------\n", -1);
        struct stat statBuf;
        if(stat(pathToDazibao,&statBuf)==-1){
            perror("stat");
            exit(EXIT_FAILURE);
        }
        posM[num_msg]=statBuf.st_size;
        FILE* dazibao=NULL;
        if((dazibao=fopen(pathToDazibao,"a+b"))==NULL){
            perror("fopen");
            exit(EXIT_FAILURE);
        }
        ajouterMessageDated(dazibao,tlvDated,0);
        fclose(dazibao);
        return NULL;
    }else{
        return tlvDated;
    }

    printf("ajout dated\n");
    return NULL;
}



void ajouter_jpegN(){
    ajouter_jpeg(0);
}

tlv* ajouter_jpeg(int opt){
    char* filename=NULL;
    GtkWidget *selection;
    selection = gtk_file_chooser_dialog_new( g_locale_to_utf8( "Sélectionnez un fichier", -1, NULL, NULL, NULL),
                                            GTK_WINDOW(pWindow2), GTK_FILE_CHOOSER_ACTION_OPEN,
                                            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				                            GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                            NULL);

    if(gtk_dialog_run(GTK_DIALOG(selection))==GTK_RESPONSE_ACCEPT)
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER (selection));
 
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
    tlv* tlvJpeg=NULL;
    tlvJpeg=newTlv(4);
    if((tlvJpeg->contenuImage=malloc((buf.st_size+1)*sizeof(unsigned char)))==NULL){
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    tlvJpeg->lenght=buf.st_size;
    unsigned char* fmap=mmap(NULL,buf.st_size,PROT_READ,MAP_PRIVATE,fd,0);
    if(fmap==MAP_FAILED){
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    memcpy(tlvJpeg->contenuImage,fmap,buf.st_size);
    if(munmap(fmap,buf.st_size)==-1){
        perror("munmap");
        exit(EXIT_FAILURE);
    }
    if(close(fd)==-1){
        perror("close");
        exit(EXIT_FAILURE);
    }
    if((tlvJpeg->pathImage=malloc((strlen(filename)+1)*sizeof(char)))==NULL){
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    strcpy(tlvJpeg->pathImage,filename);

    printf("path : %s\n",tlvJpeg->pathImage);
    if(opt==0){
        num_msg++;
        char* num_msg2=NULL;
        if((num_msg2=malloc(10*sizeof(char)))==NULL){
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        sprintf(num_msg2,"%d",num_msg);
        printf("num msg%d\n",num_msg);
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
        ajouterMessageJpeg(dazibao,tlvJpeg, 0);
        fclose(dazibao);
        return NULL;
    }else{
        printf("test de retour ?\n");
        return tlvJpeg;
    }
    return NULL;


}

void ajouter_pngN(){
    ajouter_png(0);
}

tlv* ajouter_png(int opt){
    printf("ajout png \n");
    char* filename=NULL;
    GtkWidget *selection;
    selection = gtk_file_chooser_dialog_new( g_locale_to_utf8( "Sélectionnez un fichier", -1, NULL, NULL, NULL),
                                            GTK_WINDOW(pWindow2), GTK_FILE_CHOOSER_ACTION_OPEN,
                                            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				                            GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                            NULL);

    if(gtk_dialog_run(GTK_DIALOG(selection))==GTK_RESPONSE_ACCEPT)
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER (selection));
 
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
    tlv* tlvJpeg=NULL;
    tlvJpeg=newTlv(4);
    if((tlvJpeg->contenuImage=malloc((buf.st_size+1)*sizeof(unsigned char)))==NULL){
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    tlvJpeg->lenght=buf.st_size;
    unsigned char* fmap=mmap(NULL,buf.st_size,PROT_READ,MAP_PRIVATE,fd,0);
    if(fmap==MAP_FAILED){
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    memcpy(tlvJpeg->contenuImage,fmap,buf.st_size);
    if(munmap(fmap,buf.st_size)==-1){
        perror("munmap");
        exit(EXIT_FAILURE);
    }
    if(close(fd)==-1){
        perror("close");
        exit(EXIT_FAILURE);
    }
    if((tlvJpeg->pathImage=malloc((strlen(filename)+1)*sizeof(char)))==NULL){
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    strcpy(tlvJpeg->pathImage,filename);

    printf("path : %s\n",tlvJpeg->pathImage);
    if(opt==0){
        num_msg++;
        char* num_msg2=NULL;
        if((num_msg2=malloc(10*sizeof(char)))==NULL){
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        sprintf(num_msg2,"%d",num_msg);
        printf("num msg%d\n",num_msg);
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
        ajouterMessageJpeg(dazibao,tlvJpeg, 0);
        fclose(dazibao);
        return NULL;
    }else{
        return tlvJpeg;
    }
    return NULL;


}


void ajouter_texteN(){
    ajouter_texte(0);
}

tlv* ajouter_texte(int opt){
    GtkWidget* pBoite;
    GtkWidget* pEntry;
    const gchar* sNom;
 
  /* Création de la boite de dialogue */
  /* 1 bouton Valider */
  /* 1 bouton Annuler */
    pBoite = gtk_dialog_new_with_buttons("Ajout",
				       GTK_WINDOW(pWindow2),
				       GTK_DIALOG_MODAL,
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       NULL);
 
    pEntry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(pEntry), "Saisissez votre texte");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(pBoite)->vbox), pEntry, TRUE, FALSE, 0);
    gtk_widget_show_all(GTK_DIALOG(pBoite)->vbox);
    switch (gtk_dialog_run(GTK_DIALOG(pBoite))){
        case GTK_RESPONSE_OK:
            sNom = gtk_entry_get_text(GTK_ENTRY(pEntry));
            printf("%s\n",sNom); 
            tlv* new_msg=NULL;
            new_msg=newTlv(2);
            if((new_msg->textOrPath=malloc((strlen(sNom)+1)*sizeof(char)))==NULL){
                perror("malloc");
                exit(EXIT_FAILURE);
            }
            strcpy(new_msg->textOrPath,(char*)sNom);
            new_msg->lenght=strlen(new_msg->textOrPath);
            if(opt==0){
                num_msg++;
                struct stat statBuf;
                FILE *f=fopen(pathToDazibao,"a+b");
                if(stat(pathToDazibao,&statBuf)==-1){
                    perror("stat");
                    exit(EXIT_FAILURE);
                }
                posM[num_msg]=statBuf.st_size;
                char* num_msg2=NULL;
                if((num_msg2=malloc(10*sizeof(char)))==NULL){
                    perror("malloc");
                    exit(EXIT_FAILURE);
                }
                sprintf(num_msg2,"%d",num_msg);
                GtkTextIter end;
                GtkTextBuffer *buf;
                buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pTextView));
                gtk_text_view_set_wrap_mode((GTK_TEXT_VIEW(pTextView)),GTK_WRAP_WORD);
                gtk_text_view_set_justification((GTK_TEXT_VIEW(pTextView)),GTK_JUSTIFY_CENTER);
                gtk_text_buffer_get_end_iter(buf,&end);
                gtk_text_buffer_insert(buf, &end, num_msg2, -1);
                gtk_text_buffer_insert(buf, &end, "------------------------------------------------------------------------------------------------------------------------------------\n", -1);
                gtk_text_buffer_insert(buf, &end, g_locale_to_utf8(new_msg->textOrPath, -1, NULL, NULL, NULL), -1);
                gtk_text_buffer_insert(buf, &end, g_locale_to_utf8("\n", -1, NULL, NULL, NULL), -1);
                ajouterMessageTxt(f,new_msg,0);
                fclose(f);
                free(num_msg2);
                gtk_widget_destroy(pEntry);	
                gtk_widget_destroy(pBoite);	
                return NULL;
            }else{
                gtk_widget_destroy(pEntry);	
                gtk_widget_destroy(pBoite);	
                return new_msg;
            }
            break;
        case GTK_RESPONSE_CANCEL:
        case GTK_RESPONSE_NONE:
            gtk_widget_destroy(pEntry);	
            gtk_widget_destroy(pBoite);
            break;
        default:
            printf("Vous n'avez rien saisi !");
            gtk_widget_destroy(pEntry);	
            gtk_widget_destroy(pBoite);
            break;
    }
    printf("ajout texte \n");
    return NULL;
}

tlv* choisirTlv(){
    GtkWidget* pBoite;
    GtkWidget* pEntry;
    const gchar* type;
    int len,i,valid;
    char c;
    tlv* tlv=NULL;


    pBoite = gtk_dialog_new_with_buttons("Choix du TLV",
				       GTK_WINDOW(pWindow2),
				       GTK_DIALOG_MODAL,
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       NULL);
    pEntry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(pEntry), "Entrez le type du TLV voulu");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(pBoite)->vbox), pEntry, TRUE, FALSE, 0);
    gtk_widget_show_all(GTK_DIALOG(pBoite)->vbox);
    switch (gtk_dialog_run(GTK_DIALOG(pBoite))){
      /* L utilisateur valide */
        case GTK_RESPONSE_OK:
            type=gtk_entry_get_text(GTK_ENTRY(pEntry));
            len=strlen(type);
            for(i=0; i<len;i++){
	            c=type[i];
	            if((c<'0') || (c>'9')){
	                valid=0;
	                break;
	            }
            }
      
            if(valid == 0){
	            printf("La chaine n'est pas un nombre entier.\n");
	            dialog = gtk_message_dialog_new(GTK_WINDOW(pBoite),GTK_DIALOG_MODAL,
					GTK_MESSAGE_ERROR,GTK_BUTTONS_OK,"Vous devez saisir un nombre entier!");
     
	            gtk_dialog_run(GTK_DIALOG(dialog));
	            gtk_widget_destroy(dialog);
	            gtk_widget_destroy(pBoite);
            }else{
	            int n=atol(type);
	            if(n > 1 && n <= TLV_MAX){
	                printf("type choisi : %d\n", n);
                    gtk_widget_destroy(pBoite);	
                    switch (n) {
                        case 2 :
                            tlv=ajouter_texte(2);
                            return(tlv);
                            break;
                        case 3 :
                            tlv=ajouter_png(3);
                            return(tlv);
                            break;
                        case 4 : 
                            tlv=ajouter_jpeg(4);
                            return(tlv);
                            break;
                        case 5 :
                            tlv=ajouter_compound(5);
                            return(tlv);
                            break;
                        case 6 : 
                            tlv=ajouter_dated(6);
                            return(tlv);
                            break;
                    }

                }else {
	                printf("Vous devez rentrer un nombre compris dans les types de TLV");
	                dialog = gtk_message_dialog_new(GTK_WINDOW(pBoite),GTK_DIALOG_MODAL,
					GTK_MESSAGE_ERROR,GTK_BUTTONS_OK,"Vous devez rentrer un nombre compris dans les types de TLV");
     
                    gtk_dialog_run(GTK_DIALOG(dialog));
                    gtk_widget_destroy(dialog);
                    gtk_widget_destroy(pBoite);
                }
            }

            break;
      /* L utilisateur annule */
        case GTK_RESPONSE_CANCEL:
        case GTK_RESPONSE_NONE:
        default:
            printf("Vous n'avez rien saisi !");
            dialog = gtk_message_dialog_new(GTK_WINDOW(pBoite),GTK_DIALOG_MODAL,
					  GTK_MESSAGE_ERROR,GTK_BUTTONS_OK,"Vous n'avez rien saisi!");
     
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            gtk_widget_destroy(pBoite);
            break;
    }
    return tlv;
}
