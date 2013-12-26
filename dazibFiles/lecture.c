#include <stdlib.h>
#include "typedef.h"
#include "globalvar.h"
#include "lecture.h"
#include "fonctionsGenerales.h"

void lectureCompound(FILE* dazibao,unsigned int taille){
/* Cette fonction est appelée par la fonction de lecture
 * Elle va appeler la lecture du dazibao autant de fois que nécessaire
 * (ie : tant que la longueur totale lu est inferieure à la taille
 * du tlv compound)
*/
    unsigned int tailleParcourue=0;
    unsigned long int tailleLecture=0;
    while(taille>tailleParcourue){
        if((tailleLecture=lectureDazibao(dazibao))!=-1) {
            tailleParcourue=tailleParcourue+4+tailleLecture;
        }else{ // Si c'est un pad1 (1 octet)
            tailleParcourue++;
        }
    }
}

int lectureDazibao(FILE* dazibao){
/* Cette fonction va se charger de la lecture du fichier dazibao
 * spécifié par le FILE*. Cette fonciton ne va lire qu'un seul tlv
 * et va renvoyer sa longueur
*/
    GtkTextIter end;
    GtkTextBuffer *buf;
    unsigned char type=255, lenght1=0, lenght2=0, lenght3=0;
    unsigned int lenght = 0;
    char *contenu="";
    char *num_msg2=NULL;
 
    GError *error = NULL;
    GdkPixbuf *pix;

    gchar *nomImage = NULL;

    num_msg2=malloc(10*sizeof(unsigned char));
    sprintf(num_msg2, "%d", num_msg);
  
    // On commence par lire le type 
    fread(&type,1,1,dazibao);

    buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pTextView));
    gtk_text_view_set_wrap_mode((GTK_TEXT_VIEW(pTextView)),GTK_WRAP_WORD);
    gtk_text_view_set_justification((GTK_TEXT_VIEW(pTextView)),GTK_JUSTIFY_CENTER);
    gtk_text_buffer_get_end_iter(buf,&end);

    // Si ce n'est ni un pad1/padn, et que le tlv n'a pas de parent (compound ou Dated)
    if((type!=0 && type!=1) && isComp_Dated==0){
    // On affiche la barre sur l'interface graphique ainsi que son numéro
        gtk_text_buffer_insert(buf, &end, num_msg2, -1);
        gtk_text_buffer_insert(buf, &end, "------------------------------------------------------------------------------------------------------------------------------------\n", -1);
    }
    free(num_msg2);
    isPad1_N=0;
    if (type==0) { // Si il s'agit d'un Pad_1
        isPad1_N=1;
        return -1;
    }
    if((type!=0 && type!=1) && isComp_Dated==0){ // Si le message n'a pas de parents
        posM[num_msg]=ftell(dazibao)-1;          // Et n'est pas pad1 ou padN
    }
    // On lit la taille
    fread(&lenght1,1,1,dazibao);
    fread(&lenght2,1,1,dazibao);
    fread(&lenght3,1,1,dazibao);
    lenght=lireLenght(lenght1,lenght2,lenght3);
    // On alloue suffisemment de mémoire pour lire le contenu
    if((contenu=malloc((lenght+2)*sizeof(unsigned char)))==NULL && lenght!=0){
        perror("Malloc : ");
        exit(EXIT_FAILURE);
    }
    if (type==5) { // S'il s'agit d'un compound
        isComp_Dated=1; // On place le flag
        lectureCompound(dazibao,lenght); // On appelle la fonction pour les compounds
        free(contenu);
        isComp_Dated=0;
        return lenght; 
    }else if (type==6) { // Si c'est un dated
        time_t secondes=0;
        // On lit sa date
        unsigned char l1,l2,l3,l4;
        isComp_Dated=1; // On place le flag
        fread(&l1,1,1,dazibao);
        fread(&l2,1,1,dazibao);
        fread(&l3,1,1,dazibao);
        fread(&l4,1,1,dazibao);
        secondes=256*256*256*l1+256*256*l2+256*l3+l4;
        // On affiche la date
        buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pTextView));
        gtk_text_view_set_wrap_mode((GTK_TEXT_VIEW(pTextView)),GTK_WRAP_WORD);
        gtk_text_view_set_justification((GTK_TEXT_VIEW(pTextView)),GTK_JUSTIFY_CENTER);
        gtk_text_buffer_get_end_iter(buf,&end);
        gtk_text_buffer_insert(buf, &end, g_locale_to_utf8(ctime(&secondes), -1, NULL, NULL, NULL), -1);
        
        // On relance la fonction pour lire son sous-tlv
        lectureDazibao(dazibao);
        free(contenu);
        isComp_Dated=0;
        return lenght;
    }
    // On lit le contenu
    fread(contenu,lenght,1,dazibao);
    if (type==1) { // Si c'est un pad N
        isPad1_N=1;
        free(contenu);
        return lenght;
    }else if (type==2) { // Si c'est un texte
        contenu[lenght]='\n';
        contenu[lenght+1]='\0';
        // On affiche le texte
        buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pTextView));
        gtk_text_view_set_wrap_mode((GTK_TEXT_VIEW(pTextView)),GTK_WRAP_WORD);
        gtk_text_view_set_justification((GTK_TEXT_VIEW(pTextView)),GTK_JUSTIFY_CENTER);
        gtk_text_buffer_get_end_iter(buf,&end);
        gtk_text_buffer_insert(buf, &end, g_locale_to_utf8(contenu, -1, NULL, NULL, NULL), -1);
        free(contenu);
        return lenght;
    }else if (type==3) { // Si c'est un png
        // On crée un fichier temporaire dans /tmp/
        char tempFile[]="/tmp/dbz.XXXXXX";
        nomImage=mktemp(tempFile);
        FILE *f=NULL;
        if((f=fopen(nomImage,"w+b"))==NULL){
            perror("fopen ");
            exit(EXIT_FAILURE);
        }
        // On écrit le contenu du tlv dans le fichier temporaire que l'on vient de créer
        fwrite(contenu,lenght,1,f);
        fclose(f);
        pix = gdk_pixbuf_new_from_file_at_size(nomImage,400,400, &error);
        if (error){
            GtkWidget *msg;
            msg = gtk_message_dialog_new (GTK_WINDOW (pWindow2), 
                                	        GTK_DIALOG_MODAL,
                                            GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, 
                                            error->message);
            gtk_dialog_run (GTK_DIALOG (msg));
            gtk_widget_destroy (msg);
            unlink(nomImage);
            return lenght;
        }
    
        // On affiche l'image
        gtk_text_buffer_insert_pixbuf(buf,&end, pix);
        gtk_text_buffer_insert(buf, &end, "\n", -1);
        // On appelle unlink sur l'image
        unlink(nomImage);
        free(contenu);
        return lenght;
    }else if (type==4) { // Si c'est un jpeg (même chose que png)
        char tempFile[]="/tmp/test.dbzXXXXXX";
        nomImage=mktemp(tempFile);
        FILE *f=NULL;
        if((f=fopen(nomImage,"w+b"))==NULL){
            perror("fopen ");
            exit(EXIT_FAILURE);
        }
        fwrite(contenu,lenght,1,f);
        fclose(f);
        pix = gdk_pixbuf_new_from_file_at_size(nomImage,400,400, &error);
        if (error){
            GtkWidget *msg;
            msg = gtk_message_dialog_new (GTK_WINDOW (pWindow2), 
                                          GTK_DIALOG_MODAL,
                                          GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, 
                                          error->message);
            gtk_dialog_run (GTK_DIALOG (msg));
            gtk_widget_destroy (msg);
            unlink(nomImage);
            return lenght;
        }
    
        gtk_text_buffer_insert_pixbuf(buf,&end, pix);
        gtk_text_buffer_insert(buf, &end, "\n", -1);
        unlink(nomImage);
        free(contenu);
        return lenght;
    }else{ // Si le type n'est pas connu, on affiche que le type n'est pas connu
        buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pTextView));
        gtk_text_view_set_wrap_mode((GTK_TEXT_VIEW(pTextView)),GTK_WRAP_WORD);
        gtk_text_view_set_justification((GTK_TEXT_VIEW(pTextView)),GTK_JUSTIFY_CENTER);
        gtk_text_buffer_get_end_iter(buf,&end);
        gtk_text_buffer_insert(buf, &end, g_locale_to_utf8("TLV de type inconnu.\n", -1, NULL, NULL, NULL), -1);
        free(contenu);
        return lenght;
    }
  
}
