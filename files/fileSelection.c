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
#include "fileSelection.h"
#include "globalvar.h"


void creer_file_selection(){
    GtkWidget *selection;
    selection = gtk_file_selection_new( g_locale_to_utf8( "Sélectionnez un fichier", -1, NULL, NULL, NULL) );
    gtk_widget_show(selection);
 
    //On interdit l'utilisation des autres fenêtres.
    gtk_window_set_modal(GTK_WINDOW(selection), TRUE);
    g_signal_connect(G_OBJECT(GTK_FILE_SELECTION(selection)->ok_button), "clicked", 
                                            G_CALLBACK(verifier_chemin), selection );
    g_signal_connect_swapped(G_OBJECT(GTK_FILE_SELECTION(selection)->cancel_button), 
                                "clicked", G_CALLBACK(gtk_widget_destroy), selection);
//    g_signal_connect_swapped(G_OBJECT(GTK_FILE_SELECTION(selection)->ok_button), 
  //                              "clicked", G_CALLBACK(gtk_widget_destroy), selection);
}

void verifier_chemin(GtkWidget *widget, GtkWidget *file_selection){
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

        printf("Dazibao choisi : %s \n ", chemin);
        if((pathToDazibao=malloc((strlen(chemin)+1)*sizeof(char)))==NULL){
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        strcpy(pathToDazibao,chemin);
   
  printf("coucou\n");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        gtk_widget_destroy(file_selection);
        gtk_widget_destroy(pWindow);
        lancer_dazibao();
    }
}





void quitter(GtkWidget* widget, gpointer data){
  GtkWidget *pQuestion;
  pQuestion = gtk_message_dialog_new(GTK_WINDOW(data),GTK_DIALOG_MODAL,
GTK_MESSAGE_QUESTION,GTK_BUTTONS_YES_NO,"Voulez vous vraiment\n"
"quitter le programme?");

  switch
    (gtk_dialog_run(GTK_DIALOG(pQuestion))){
  case
    GTK_RESPONSE_YES:
    gtk_main_quit();
    break
      ;
  case
    GTK_RESPONSE_NONE:
  case
    GTK_RESPONSE_NO:
    gtk_widget_destroy(pQuestion);
    break;
  }

  printf("Arret du programme \n");
}



