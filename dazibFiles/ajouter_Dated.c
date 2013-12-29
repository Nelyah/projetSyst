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


int ajouterMessageDated(int fd,tlv* tlv,int hasLock){
/* Cette fonction s'occupe d'écrire directement dans le fichier dazibao.
 * Elle reçoit en argument le FILE *, un pointeur sur tlv (normalement rempli avec un Dated
 * et un flag pour savoir si cette fonction doit lock le fichier ou non 
 * (ie : si elle a été appelée par une autre fonction).
 */

    GtkTextIter end;
    GtkTextBuffer *buf;
    buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pTextView));
    gtk_text_view_set_wrap_mode((GTK_TEXT_VIEW(pTextView)),GTK_WRAP_WORD);
    gtk_text_view_set_justification((GTK_TEXT_VIEW(pTextView)),GTK_JUSTIFY_CENTER);
    gtk_text_buffer_get_end_iter(buf,&end);
    int err;
    // verrou du fichier
    if(hasLock!=1) {
        if((err=flock(fd,LOCK_EX))!=0){
            perror("flock ");
            exit(EXIT_FAILURE);
        }
    }
    // On écrit le type du tlv dans le fichier
    if(write(fd,&tlv->type,1)==0) {
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
        perror("write");
        exit(EXIT_FAILURE);
    }
    if (write(fd,&l3,1)==0) {
        perror("write");
        exit(EXIT_FAILURE);
    }
    // On écrit la date, en convertissant pour écrire octet par octet
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
    // Affichage sur l'interface graphique
    gtk_text_buffer_insert(buf, &end, g_locale_to_utf8(ctime(&tlv->time), -1, NULL, NULL, NULL), -1);
    if(write(fd,&t1,1)==0) {
        perror("write");
        exit(EXIT_FAILURE);
    }
    if(write(fd,&t2,1)==0) {
        perror("write");
        exit(EXIT_FAILURE);
    }
    if(write(fd,&t3,1)==0) {
        perror("write");
        exit(EXIT_FAILURE);
    }
    if(write(fd,&t4,1)==0) {
        perror("write");
        exit(EXIT_FAILURE);
    }
    // Ensuite on écrit le sous tlv en appelant la fonction adaptée au type.
    if(tlv->tlvList[0]->type==2) {
        gtk_text_buffer_insert(buf, &end, g_locale_to_utf8(tlv->tlvList[0]->textOrPath, -1, NULL, NULL, NULL), -1);
        gtk_text_buffer_insert(buf, &end, g_locale_to_utf8("\n", -1, NULL, NULL, NULL), -1);
        ajouterMessageTxt(fd,tlv->tlvList[0],1);
    }else if(tlv->tlvList[0]->type==3){
        ajouterMessagePng(fd,tlv->tlvList[0],1);
    }else if(tlv->tlvList[0]->type==4){
        ajouterMessageJpeg(fd,tlv->tlvList[0],1);
    }else if(tlv->tlvList[0]->type==5){ 
        ajouterMessageCompound(fd,tlv->tlvList[0],1);
    }else if(tlv->tlvList[0]->type==6){
        ajouterMessageDated(fd,tlv->tlvList[0],1);
    }
    if(hasLock!=1) {
        if((err=flock(fd,LOCK_UN))!=0){
            perror("flock ");
            exit(EXIT_FAILURE);
        }
    }
    freeTlv(tlv);
    return 0;
}



void ajouter_datedN(){
/* Cette fonction est appelée par l'interface graphique uniquement. 
 * Elle permet de spécifier à la fonction ajouter_dated(int opt) qu'elle n'a pas
 * de parents. 
 */
    ajouter_dated(0);
}

tlv* ajouter_dated(int opt){
/* Cette fonction va permettre de remplir un tlv dated, en appelant une fonction qui
 * sera adapter à remplir le sous tlv du type spécifié.
 * Elle renvoie un pointeur sur tlv, et NULL si quelque chose est arrivé.
 */
    GtkWidget* pBoite;
    GtkWidget* pEntry;
    const gchar* type;
    int len,i,valid=1;
    time_t now;
    tlv* tlvDated=NULL;

    // L'interface graphique demande à l'utilisateur de choisir le type du sous tlv
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
            // on vérifie qu'il s'agit bien d'un nombre ou d'un chiffre
            for(i = 0; i < len; i++){
                if((type[i]< '0') || (type[i]> '9')){
                    valid = 0;
                    break;
                }
            }
      
            if(valid == 0){ // Si la valeur rentrée n'est pas valide
                dialog = gtk_message_dialog_new(GTK_WINDOW(pBoite),GTK_DIALOG_MODAL,
                    GTK_MESSAGE_ERROR,GTK_BUTTONS_OK,"Vous devez saisir un nombre entier!");
     
	            gtk_dialog_run(GTK_DIALOG(dialog));
	            gtk_widget_destroy(dialog);
	            gtk_widget_destroy(pBoite);
            }else{
                int n=atol(type);
                if(n > 1 && n <= TLV_MAX){
                    tlv* tlv=NULL;
                    switch (n) { // On va agir selon le type du tlv
                        case 2 : // Dans le cas d'un message texte
                            tlvDated=newTlv(6);
                            // On remplie le tlv
                            tlvDated->nbTlv=1;
                            if((tlvDated->tlvList=malloc(2*sizeof(struct tlv*)))==NULL){
                                perror("malloc");
                                exit(EXIT_FAILURE);
                            }
                            time(&now);
                            tlvDated->time=now;
                            // on appelle la fonction pour un sous-tlv texte
                            tlv=ajouter_texte(2);
                            if(tlv==NULL){   // Si le tlv a retourné NULL
                                freeTlv(tlvDated);
                                gtk_widget_destroy(pBoite);
                                return NULL;
                            }
                            if(tlv->lenght > 16777216){ // Si la taille du Tlv est trop grande
                                freeTlv(tlv);
                                freeTlv(tlvDated);
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
                                freeTlv(tlvDated);
                                gtk_widget_destroy(pBoite);
                                return NULL;
                            }
                            if(tlvDated->tlvList[0]->lenght > 16777216){
                                freeTlv(tlv);
                                freeTlv(tlvDated);
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
                                freeTlv(tlvDated);
                                gtk_widget_destroy(pBoite);
                                return NULL;
                            }
                            if(tlvDated->tlvList[0]->lenght > 16777216){ 
                                freeTlv(tlv);
                                freeTlv(tlvDated);
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
                                freeTlv(tlvDated);
                                gtk_widget_destroy(pBoite);
                                return NULL;
                            }
                            if(tlvDated->tlvList[0]->lenght > 16777216){ 
                                freeTlv(tlv);
                                freeTlv(tlvDated);
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
                                freeTlv(tlvDated);
                                gtk_widget_destroy(pBoite);
                                return NULL;
                            }
                            if(tlvDated->tlvList[0]->lenght > 16777216){                                 
                                freeTlv(tlv);
                                freeTlv(tlvDated);
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
                // Si le chiffre ne spécifiait aucun TLV
	                dialog = gtk_message_dialog_new(GTK_WINDOW(pBoite),GTK_DIALOG_MODAL,
					GTK_MESSAGE_ERROR,GTK_BUTTONS_OK,"Vous devez rentrer un nombre compris dans les types de TLV");
     
                    gtk_dialog_run(GTK_DIALOG(dialog));
                    gtk_widget_destroy(dialog);
                    gtk_widget_destroy(pBoite);
                    return NULL;
                }
            }

            break;
        case GTK_RESPONSE_CANCEL:
        case GTK_RESPONSE_NONE:
        default:
            dialog = gtk_message_dialog_new(GTK_WINDOW(pBoite),GTK_DIALOG_MODAL,
					  GTK_MESSAGE_ERROR,GTK_BUTTONS_OK,"Vous n'avez rien saisi!");
     
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            gtk_widget_destroy(pBoite);
            return NULL;
            break;
    }
    if(opt==0){ // si la fonction est appelée par l'interface graphique (pas de parent).
        num_msg++;
        char* num_msg2=NULL;
        struct stat statBuf;
        int fd;
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
        if((fd=open(pathToDazibao,O_RDWR|O_APPEND))==-1){
            perror("open");
            exit(EXIT_FAILURE);
        }
        if(fstat(fd,&statBuf)==-1){
            perror("fstat");
            exit(EXIT_FAILURE);
        }
        posM[num_msg]=statBuf.st_size;
        ajouterMessageDated(fd,tlvDated,0);
        close(fd);
        return NULL;
    }else{
        return tlvDated;
    }

    return NULL;
}
