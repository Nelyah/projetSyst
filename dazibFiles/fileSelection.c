#include <stdlib.h>
#include <string.h>
#include "globalvar.h"
#include "ouverture.h"
#include "fileSelection.h"




void verifier_chemin(GtkWidget *widget, GtkWidget *file_selection){
/* Cette fonction a pour but de vérfier qu'il s'agit bien d'un dazibao.
 * Elle va tenter d'ouvrir le fichier donné, et la fonction d'ouverture 
 * va renovyer si jamais il ne s'agit pas d'un dazibao
 * (on vérifie cela par le numéro magique, entre autres).
*/
    chemin = gtk_file_selection_get_filename(GTK_FILE_SELECTION (file_selection));
    if(ouverture(chemin) == 10){ 
        dialog = gtk_message_dialog_new(GTK_WINDOW(file_selection),GTK_DIALOG_MODAL,
				    GTK_MESSAGE_ERROR,GTK_BUTTONS_OK,"Le fichier selectionne n'est pas un dazibao!\nVeuillez en saisir un autre...");
     
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        gtk_widget_destroy(file_selection);
        creer_file_selection();
    }else{
        dialog = gtk_message_dialog_new(GTK_WINDOW(file_selection),GTK_DIALOG_MODAL,
				    GTK_MESSAGE_INFO,GTK_BUTTONS_OK,"Vous avez choisi :\n%s", chemin);

        if((pathToDazibao=malloc((strlen(chemin)+1)*sizeof(char)))==NULL){
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        strcpy(pathToDazibao,chemin);
   
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        gtk_widget_destroy(file_selection);
        gtk_widget_destroy(pWindow);
        lancer_dazibao();
    }
}

void creer_file_selection(){
/* Cette fonction va permettre à l'utilisateur de sélectionner un fichier 
 * et va appeler la fonction verifier_chemin pour s'assurer qu'il s'agit bien d'un 
 * dazibao
*/
    GtkWidget *selection;
    selection = gtk_file_selection_new( g_locale_to_utf8( "Sélectionnez un fichier", -1, NULL, NULL, NULL) );
    gtk_widget_show(selection);
 
    gtk_window_set_modal(GTK_WINDOW(selection), TRUE);
    g_signal_connect(G_OBJECT(GTK_FILE_SELECTION(selection)->ok_button), "clicked", 
                                            G_CALLBACK(verifier_chemin), selection );
    g_signal_connect_swapped(G_OBJECT(GTK_FILE_SELECTION(selection)->cancel_button), 
                                "clicked", G_CALLBACK(gtk_widget_destroy), selection);
}




