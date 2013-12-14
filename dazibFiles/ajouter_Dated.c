#include <stdlib.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <string.h>
#include "typedef.h"
#include "globalvar.h"
#include "ajouter_Dated.h"
#include "ajouter_texte.h"
#include "ajouter_Png.h"
#include "ajouter_Jpeg.h"
#include "fonctionsGenerales.h"
#include "ajouter_Compound.h"


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



void ajouter_datedN(){
    ajouter_dated(0);
}

tlv* ajouter_dated(int opt){
    GtkWidget* pBoite;
    GtkWidget* pEntry;
    const gchar* type;
    int len,i,valid=1;
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
        case GTK_RESPONSE_OK:
            type = gtk_entry_get_text(GTK_ENTRY(pEntry));
            len = strlen(type);
      
            for(i = 0; i < len; i++){
                if((type[i]< '0') || (type[i]> '9')){
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
                            if(tlv==NULL){
                            printf("le texte est un NULL ! \n");
                                free(tlvDated);
                                gtk_widget_destroy(pBoite);
                                return NULL;
                            }
                            tlvDated->lenght=tlv->lenght+4+4; //+4 pour le tlv, +4 pour le time
                            tlvDated->tlvList[0]=tlv;
                            break;
                        case 3 :
                            tlvDated=newTlv(6);
                            tlvDated->nbTlv=1;
                            if((tlvDated->tlvList=malloc(2*sizeof(struct tlv*)))==NULL){
                                perror("malloc");
                                exit(EXIT_FAILURE);
                            }
                            tlvDated->tlvList[0]=ajouter_png(3);
                            if(tlvDated->tlvList[0]==NULL){
                                free(tlvDated);
                                gtk_widget_destroy(pBoite);
                                return NULL;
                            }
                            time(&now);
                            tlvDated->time=now;
                            tlvDated->lenght=tlvDated->tlvList[0]->lenght+4+4;
                            break;
                        case 4 : 
                            tlvDated=newTlv(6);
                            tlvDated->nbTlv=1;
                            if((tlvDated->tlvList=malloc(2*sizeof(struct tlv*)))==NULL){
                                perror("malloc");
                                exit(EXIT_FAILURE);
                            }
                            tlvDated->tlvList[0]=ajouter_jpeg(4);
                            if(tlvDated->tlvList[0]==NULL){
                                free(tlvDated);
                                gtk_widget_destroy(pBoite);
                                return NULL;
                            }
                            time(&now);
                            tlvDated->time=now;
                            tlvDated->lenght=tlvDated->tlvList[0]->lenght+4+4;
                            break;
                        case 5 :
                            tlvDated=newTlv(6);
                            tlvDated->nbTlv=1;
                            if((tlvDated->tlvList=malloc(2*sizeof(struct tlv*)))==NULL){
                                perror("malloc");
                                exit(EXIT_FAILURE);
                            }
                            tlvDated->tlvList[0]=ajouter_compound(5);
                            if(tlvDated->tlvList[0]==NULL){
                                free(tlvDated);
                                gtk_widget_destroy(pBoite);
                                return NULL;
                            }
                            time(&now);
                            tlvDated->time=now;
                            tlvDated->lenght=tlvDated->tlvList[0]->lenght+4+4;
                            break;
                        case 6 : 
                            tlvDated=newTlv(6);
                            tlvDated->nbTlv=1;
                            if((tlvDated->tlvList=malloc(2*sizeof(struct tlv*)))==NULL){
                                perror("malloc");
                                exit(EXIT_FAILURE);
                            }
                            tlvDated->tlvList[0]=ajouter_dated(6);
                            if(tlvDated->tlvList[0]==NULL){
                                free(tlvDated);
                                gtk_widget_destroy(pBoite);
                                return NULL;
                            }
                            time(&now);
                            tlvDated->time=now;
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
            return NULL;
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
