#include <stdlib.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <string.h>
#include "globalvar.h"
#include "typedef.h"
#include "fonctionsGenerales.h"
#include "ajouter_Compound.h"
#include "ajouter_texte.h"
#include "ajouter_Png.h"
#include "ajouter_Jpeg.h"
#include "ajouter_Dated.h"



int ajouterMessageCompound(FILE* dazibao,tlv* tlv, int hasLock){ 
    int i;
    GtkTextIter end;
    GtkTextBuffer *buf;
    buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pTextView));
    gtk_text_view_set_wrap_mode((GTK_TEXT_VIEW(pTextView)),GTK_WRAP_WORD);
    gtk_text_view_set_justification((GTK_TEXT_VIEW(pTextView)),GTK_JUSTIFY_CENTER);
    gtk_text_buffer_get_end_iter(buf,&end);
    if(hasLock!=1) {
        if(flock(fileno(dazibao),LOCK_EX)==-1){
            perror("flock ");
            exit(EXIT_FAILURE);
        }
    }
    if(tlv->lenght>16777216) { // la taille n'est pas trop grande pour être stockée dans 3 octets
        printf("Erreur : La taille du message est trop grande\n");
        exit(EXIT_FAILURE);
    }
    if(fwrite(&tlv->type,1,1,dazibao)==0) {
        perror("fwrite :");
        exit(EXIT_FAILURE);
    }
    unsigned char l1=0,l2=0,l3=0;
    l1=ecrireLenght1(tlv->lenght);
    l2=ecrireLenght2(tlv->lenght,l1);
    l3=ecrireLenght3(tlv->lenght,l1,l2);
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
    for (i = 0; i < tlv->nbTlv; i++) {
        if(tlv->tlvList[i]->type==2) {
            gtk_text_buffer_insert(buf, &end, g_locale_to_utf8(tlv->tlvList[0]->textOrPath, -1, NULL, NULL, NULL), -1);
            gtk_text_buffer_insert(buf, &end, g_locale_to_utf8("\n", -1, NULL, NULL, NULL), -1);
            ajouterMessageTxt(dazibao,(tlv->tlvList[i]),1);
        }else if(tlv->tlvList[i]->type==3){
            ajouterMessagePng(dazibao,(tlv->tlvList[i]),1);
        }else if(tlv->tlvList[i]->type==4){
            ajouterMessageJpeg(dazibao,(tlv->tlvList[i]),1);
        }else if(tlv->tlvList[i]->type==5){
            ajouterMessageCompound(dazibao,(tlv->tlvList[i]),1);
        }else if(tlv->tlvList[i]->type==6){
            ajouterMessageDated(dazibao,(tlv->tlvList[i]),1);
        }
    }
    if(hasLock!=1) {
        if(flock(fileno(dazibao),LOCK_UN)!=0){
            perror("flock : ");
            exit(EXIT_FAILURE);
        }
    }
    freeTlv(tlv);
    return 0;
}

void ajouter_compoundN(){
    ajouter_compound(0);
}

tlv* ajouter_compound(int opt){
    GtkWidget* pBoite;
    GtkWidget* pEntry;
    const gchar* type;
    int len,i,j,valid=1;
    tlv* tlvComp=NULL;


    pBoite = gtk_dialog_new_with_buttons("Nombre de TLVs",
                        GTK_WINDOW(pWindow2),
                        GTK_DIALOG_MODAL,
                        GTK_STOCK_OK,GTK_RESPONSE_OK,
                        GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
                        NULL);
    pEntry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(pEntry), "Entrez le nombre de TLVs voulu");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(pBoite)->vbox), pEntry, TRUE, FALSE, 0);
    gtk_widget_show_all(GTK_DIALOG(pBoite)->vbox);
    switch (gtk_dialog_run(GTK_DIALOG(pBoite))){
        case GTK_RESPONSE_OK:
            type = gtk_entry_get_text(GTK_ENTRY(pEntry));
            len = strlen(type);
      
            for(i = 0; i < len; i++){
                if ((type[i]< '0') || (type[i]> '9')){
                    valid = 0;
                    break;
                }
            }
      
            if(valid == 0){
                dialog = gtk_message_dialog_new(GTK_WINDOW(pBoite),GTK_DIALOG_MODAL,
                    GTK_MESSAGE_ERROR,GTK_BUTTONS_OK,"Vous devez saisir un nombre entier!");
     
                gtk_dialog_run(GTK_DIALOG(dialog));
                gtk_widget_destroy(dialog);
                gtk_widget_destroy(pBoite);
            }else{
                int n=atol(type);
                tlvComp=newTlv(5);
                tlvComp->nbTlv=n;
                if((tlvComp->tlvList=malloc((n+1)*sizeof(tlv*)))==NULL){
                    perror("malloc");
                    exit(EXIT_FAILURE);
                }
                for(i=0;i<n;i++){
                    tlvComp->tlvList[i]=choisirTlv();               
                    if(tlvComp->tlvList[i]==NULL){
                        for(j=0;j<i;j++)
                            freeTlv(tlvComp->tlvList[j]);
                        free(tlvComp);
                        gtk_widget_destroy(pBoite);
                        return NULL;
                    }
                    tlvComp->lenght=tlvComp->lenght+tlvComp->tlvList[i]->lenght+4;
                }
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
                ajouterMessageCompound(dazibao,tlvComp,0);
                fclose(dazibao);
	            gtk_widget_destroy(pBoite);
                return NULL;
            }else{
	            gtk_widget_destroy(pBoite);
                return(tlvComp);
            }
	        gtk_widget_destroy(pBoite);
            break;
        case GTK_RESPONSE_CANCEL:
        case GTK_RESPONSE_NONE:
        default:
            dialog = gtk_message_dialog_new(GTK_WINDOW(pBoite),GTK_DIALOG_MODAL,
					  GTK_MESSAGE_ERROR,GTK_BUTTONS_OK,"Vous n'avez rien saisi!");
     
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            gtk_widget_destroy(pBoite);
            break;
    }
    return NULL;

}

tlv* choisirTlv(){
    GtkWidget* pBoite;
    GtkWidget* pEntry;
    const gchar* type;
    int len,i,valid=1;
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
	            if((type[i]<'0') || (type[i]>'9')){
	                valid=0;
	                break;
	            }
            }
      
            if(valid == 0){
	            dialog = gtk_message_dialog_new(GTK_WINDOW(pBoite),GTK_DIALOG_MODAL,
					GTK_MESSAGE_ERROR,GTK_BUTTONS_OK,"Vous devez saisir un nombre entier!");
     
	            gtk_dialog_run(GTK_DIALOG(dialog));
	            gtk_widget_destroy(dialog);
	            gtk_widget_destroy(pBoite);
            }else{
	            int n=atol(type);
	            if(n > 1 && n <= TLV_MAX){
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
            dialog = gtk_message_dialog_new(GTK_WINDOW(pBoite),GTK_DIALOG_MODAL,
					  GTK_MESSAGE_ERROR,GTK_BUTTONS_OK,"Vous n'avez rien saisi!");
     
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            gtk_widget_destroy(pBoite);
            break;
    }
    return tlv;
}
