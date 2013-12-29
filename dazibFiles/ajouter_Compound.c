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



int ajouterMessageCompound(int fd,tlv* tlv, int hasLock){ 
/* Cette fonction est celle qui va écrire dans le fichier dazibao.
 * Elle prend en argument le FILE* du fichier, un tlv* rempli 
 * (normalement avec un tlv compound) et un flag pour savoir 
 * si cette fonction doit poser un verrou sur le fichier 
 * (si le compound est appelé par un dated, le verrou est déjà posé).
*/
    int i;
    GtkTextIter end;
    GtkTextBuffer *buf;
    buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pTextView));
    gtk_text_view_set_wrap_mode((GTK_TEXT_VIEW(pTextView)),GTK_WRAP_WORD);
    gtk_text_view_set_justification((GTK_TEXT_VIEW(pTextView)),GTK_JUSTIFY_CENTER);
    gtk_text_buffer_get_end_iter(buf,&end);
    if(hasLock!=1) {
        if(flock(fd,LOCK_EX)==-1){
            perror("flock ");
            exit(EXIT_FAILURE);
        }
    }
    if(tlv->lenght>16777216) { // la taille n'est pas trop grande pour être stockée dans 3 octets
        printf("Erreur : La taille du message est trop grande\n");
        exit(EXIT_FAILURE);
    }
    if(write(fd,&tlv->type,1)==0) { // Ecriture du type
        perror("write");
        exit(EXIT_FAILURE);
    }
    // écriture de la longueur sur 3 octets 
    // (décomposition en 3 unsigned char stockés chacun sur 1 octet)
    unsigned char l1=0,l2=0,l3=0;
    l1=ecrireLenght1(tlv->lenght);
    l2=ecrireLenght2(tlv->lenght,l1);
    l3=ecrireLenght3(tlv->lenght,l1,l2);
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
    /* Ici sont écrits les tlv contenus dans le tlv compound
     * La boucle va passer autant de fois que le tlv possède de sous-tlv.
     * A chaque passage, le type du sous-tlv est examiné et on appelle la fonction
     * appropriée.
     */
    for (i = 0; i < tlv->nbTlv; i++) {
        if(tlv->tlvList[i]->type==2) {
            gtk_text_buffer_insert(buf, &end, g_locale_to_utf8(tlv->tlvList[0]->textOrPath, -1, NULL, NULL, NULL), -1);
            gtk_text_buffer_insert(buf, &end, g_locale_to_utf8("\n", -1, NULL, NULL, NULL), -1);
            ajouterMessageTxt(fd,(tlv->tlvList[i]),1);
        }else if(tlv->tlvList[i]->type==3){
            ajouterMessagePng(fd,(tlv->tlvList[i]),1);
        }else if(tlv->tlvList[i]->type==4){
            ajouterMessageJpeg(fd,(tlv->tlvList[i]),1);
        }else if(tlv->tlvList[i]->type==5){
            ajouterMessageCompound(fd,(tlv->tlvList[i]),1);
        }else if(tlv->tlvList[i]->type==6){
            ajouterMessageDated(fd,(tlv->tlvList[i]),1);
        }
    }
    if(hasLock!=1) {
        if(flock(fd,LOCK_UN)!=0){
            perror("flock : ");
            exit(EXIT_FAILURE);
        }
    }
    freeTlv(tlv);
    return 0;
}

void ajouter_compoundN(){
/* Fonction appelée par l'interface graphique. Elle appelle la fonction 
 * ajouter compound avec l'option 0.
 * Cela signifie que la fonction n'est pas appelée par une autre fonction.
*/
    ajouter_compound(0);
}

tlv* ajouter_compound(int opt){
/* Cette fonction est celle qui va se charger de la construction de la structure tlv
 * qui sera associée au compound que l'on veut créer. Elle renverra un pointeur sur 
 * cette structure.
*/
    GtkWidget* pBoite;
    GtkWidget* pEntry;
    const gchar* type;
    int len,i,j,valid=1;
    tlv* tlvComp=NULL;

    // On demande le nombre de tlv voulus dans le compound
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
            // On vérifie qu'il s'agit bien d'un nombre (ou chiffre)
            for(i = 0; i < len; i++){
                if ((type[i]< '0') || (type[i]> '9')){
                    valid = 0;
                    break;
                }
            }
      
            if(valid == 0){ // Si le nombre saisi n'est pas valide
                dialog = gtk_message_dialog_new(GTK_WINDOW(pBoite),GTK_DIALOG_MODAL,
                    GTK_MESSAGE_ERROR,GTK_BUTTONS_OK,"Vous devez saisir un nombre entier!");
     
                gtk_dialog_run(GTK_DIALOG(dialog));
                gtk_widget_destroy(dialog);
                gtk_widget_destroy(pBoite);
            }else{ // On va remplir un à un les sous tlv de notre tlv compound
                int n=atol(type);
                tlvComp=newTlv(5);
                tlvComp->nbTlv=n;
                if((tlvComp->tlvList=malloc((n+1)*sizeof(tlv*)))==NULL){
                    perror("malloc");
                    exit(EXIT_FAILURE);
                }
                for(i=0;i<n;i++){
                    // La fonction choisir tlv renverra un pointeur vers un 
                    // tlv déjà rempli, ou NULL si l'action n'a pas pu aboutir
                    tlvComp->tlvList[i]=choisirTlv();               
                    // Dans le cas où cela n'a pas pu aboutir 
                    if(tlvComp->tlvList[i]==NULL){
                        for(j=0;j<i;j++)
                            freeTlv(tlvComp->tlvList[j]);
                        free(tlvComp);
                        gtk_widget_destroy(pBoite);
                        return NULL;
                    }
                    if(tlvComp->tlvList[i]->lenght > 16777216){
                        printf("La taille du tlv est trop grande\n");
                        for(j=0;j<i;j++)
                            freeTlv(tlvComp->tlvList[j]);
                        free(tlvComp);
                        gtk_widget_destroy(pBoite);
                        return NULL;
                    }
                    tlvComp->lenght=tlvComp->lenght+tlvComp->tlvList[i]->lenght+4;
                }
            } 
            // Si la fonction a été appelée par l'interface graphique 
            // (c'est à dire qu'elle n'a pas de parent)
            if(opt==0){
                struct stat statBuf;
                char* num_msg2=NULL;
                int fd;
                num_msg++;
                if((num_msg2=malloc(10*sizeof(char)))==NULL){
                    perror("malloc");
                    exit(EXIT_FAILURE);
                }
                sprintf(num_msg2,"%d",num_msg);
                GtkTextIter end;
                GtkTextBuffer *buf;
                // pour l'interface graphique (ajout du nouveau message)
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
                // On note la position du début de ce tlv
                posM[num_msg]=statBuf.st_size;
                ajouterMessageCompound(fd,tlvComp,0);
                close(fd);
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
/* Cette fonction s'occupe de demander à l'utilisateur de choisir un tlv
 * et de l'aider à remplir ce tlv, pour ensuite renvoyer un pointeur sur la structure.
 * Un pointeur NULL est renvoyé si l'utilisateur annule où qu'il y a une erreur.
*/
    GtkWidget* pBoite;
    GtkWidget* pEntry;
    const gchar* type;
    int len,i,valid=1;
    tlv* tlv=NULL;


    // Boites de dialogue pour choisir le tlv
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
                // On vérifie que le type du tlv existe
	            if(n > 1 && n <= TLV_MAX){
                    gtk_widget_destroy(pBoite);	
                    // On appelle la fonction qui renverra le pointeur vers le tlv voulu
                    // Cette fonction sert d'intermédiaire, et renverra le pointeur reçu.
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
                    return NULL;
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
            return NULL;
            break;
    }
    return tlv;
}
