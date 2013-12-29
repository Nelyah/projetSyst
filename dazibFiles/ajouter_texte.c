#include <stdlib.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <string.h>
#include "globalvar.h"
#include "typedef.h"
#include "fonctionsGenerales.h"
#include "ajouter_texte.h"


int ajouterMessageTxt(int fd,tlv* tlv,int hasLock){
/* Cette fonction va permettre d'écrire dans le fichier dazibao
 * Elle prend en argument un FILE* sur le fichier, un pointeur sur tlv
 * qui est normalement rempli pour un tlv texte, et un flag qui permet de 
 * savoir si cette fonction doit lock le fichier ou non (si elle est appelée 
 * par un parent ou non).
*/
    if (hasLock!=1) {
        if(flock(fd,LOCK_EX)!=0){
            perror("lock");
            exit(EXIT_FAILURE);
        }
    }
    if(tlv->lenght>16777216) { // taille max d'un entier sur 24 bits unsigned
        printf("Erreur : Le message est trop long \n");
        exit(EXIT_FAILURE);
    }
    // On écrit le type du tlv
    if(write(fd,&tlv->type,1)==0) {
        perror("write");
        exit(EXIT_FAILURE);
    }
    // On écrit sa taille
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
    // On écrit le contenu du texte
    if(write(fd,tlv->textOrPath,tlv->lenght)==0) {
        perror("write");
        exit(EXIT_FAILURE);
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


void ajouter_texteN(){
/* Cette fonction est appelée par l'interface graphique uniquement
 * Elle va appeler ajouter_texte(int opt) avec l'option 0 qui signifiera
 * que le tlv en train d'être construit n'a pas de parent (il n'aura pas besoin
 * de renvoyer son pointeur, et il appelera directement la fonction pour écrire
 * son tlv dans le fichier dazibao).
*/
    ajouter_texte(0);
}

tlv* ajouter_texte(int opt){
/* Cette fonction va permettre de remplir le tlv texte 
 * Elle va renvoyer un pointeur sur le tlv rempli une fois qu'elle a terminé,
 * ou un pointeur vers NULL, si l'utilisateur annule, ou si quelque chose 
 * s'est mal passé, où encore si la fonction n'a rien à renvoyer (pas de parent).
*/
    GtkWidget* pBoite;
    GtkWidget* pEntry;
    const gchar* sNom;
 
    pBoite = gtk_dialog_new_with_buttons("Ajout",
				       GTK_WINDOW(pWindow2),
				       GTK_DIALOG_MODAL,
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       NULL);
 
 // On rentre le texte voulu
    pEntry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(pEntry), "Saisissez votre texte");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(pBoite)->vbox), pEntry, TRUE, FALSE, 0);
    gtk_widget_show_all(GTK_DIALOG(pBoite)->vbox);
    switch (gtk_dialog_run(GTK_DIALOG(pBoite))){
        case GTK_RESPONSE_OK:
        // On initialise le tlv texte et on le rempli
            sNom = gtk_entry_get_text(GTK_ENTRY(pEntry));
            tlv* new_msg=NULL;
            new_msg=newTlv(2);
            if((new_msg->textOrPath=malloc((strlen(sNom)+1)*sizeof(char)))==NULL){
                perror("malloc");
                exit(EXIT_FAILURE);
            }
            strcpy(new_msg->textOrPath,(char*)sNom);
            new_msg->lenght=strlen(new_msg->textOrPath);

            // si la fonction n'a pas de parent
            if(opt==0){
                num_msg++;
                struct stat statBuf;
                int fd;
                if((fd=open(pathToDazibao,O_RDWR|O_APPEND))==-1){
                    perror("open");
                    exit(EXIT_FAILURE);
                }
                if(fstat(fd,&statBuf)==-1){
                    perror("fstat");
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
                ajouterMessageTxt(fd,new_msg,0);
                close(fd);
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
            gtk_widget_destroy(pEntry);	
            gtk_widget_destroy(pBoite);
            break;
    }
    return NULL;
}
