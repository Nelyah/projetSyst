#include "globalvar.h"
#include "quitter.h"


void quitter(GtkWidget* widget, gpointer data){
/* Cette fonction est appelée pour quitter le programme.
 * Elle permet d'afficher une boite de dialogue en demandant confirmation
 * à l'utilisateur-trice s'il/elle veut quitter.
*/
    GtkWidget *pQuestion;
    pQuestion = gtk_message_dialog_new(GTK_WINDOW(data),GTK_DIALOG_MODAL,
        GTK_MESSAGE_QUESTION,GTK_BUTTONS_YES_NO,"Voulez vous vraiment\n"
                                                "quitter le programme?");

    switch(gtk_dialog_run(GTK_DIALOG(pQuestion))){
        case GTK_RESPONSE_YES:
            gtk_main_quit();
            break;
        case GTK_RESPONSE_NONE:
        case GTK_RESPONSE_NO:
            gtk_widget_destroy(pQuestion);
            break;
    }
  printf("Arret du programme \n");
}
