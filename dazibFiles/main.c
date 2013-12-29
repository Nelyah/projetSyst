#include <stdlib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "typedef.h"
#include "fileSelection.h"
#include "quitter.h"
#include "creerDazibao.h"


#define NB_MESSAGE 1024
#define TLV_MAX 6


const gchar* chemin;
char* pathToDazibao;
struct stat s;
const gchar *sText;
int num_msg=0;
long *posM=NULL;
int isPad1_N=0, isComp_Dated=0, isDatedNew=0;
GtkWidget *pWindow; GtkWidget *pWindow2;
GtkWidget *pVBox;
GtkWidget *pVBox2;
GtkWidget *pTextView;
GtkWidget *scrollbar;   
GtkWidget *dialog;


void interruptHandler(int sig){
/* Cette fonction catch les signals SIGINT et SIGTERM
 * Elle permet de terminer proprement le programme.
*/ 
    gtk_main_quit(); 
}




int main(int argc, char **argv){
/* Fonction main, point d'entrée du programme
 * l'interface graphique est initialisée, ainsi que 
 * certaines valeurs.
*/ 
    GtkWidget *pMenuBar;
    GtkWidget *pMenu;
    GtkWidget *pMenuItem;
    GtkWidget *pImage;
  
    signal(SIGINT,interruptHandler);
    signal(SIGTERM,interruptHandler);
    
    if((posM=malloc(NB_MESSAGE*sizeof(long)))==NULL){
        perror("malloc ");
        exit(EXIT_FAILURE);
    }

    gtk_init(&argc, &argv);

    pWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(pWindow), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(pWindow), 500, 150);
    gtk_window_set_title(GTK_WINDOW(pWindow), "Interface du  Dazibao");
    g_signal_connect(G_OBJECT(pWindow),"destroy",G_CALLBACK(gtk_main_quit), NULL);

    pVBox = gtk_vbox_new(FALSE,0);
    gtk_container_add(GTK_CONTAINER(pWindow),pVBox);


    pImage = gtk_image_new_from_file("i.jpg");
    gtk_box_pack_start(GTK_BOX(pVBox), pImage, FALSE, FALSE, 5);

    pMenuBar = gtk_menu_bar_new();
    pMenu = gtk_menu_new();
    pMenuItem = gtk_menu_item_new_with_label("Ouvrir...");
    g_signal_connect(G_OBJECT(pMenuItem), "activate", G_CALLBACK(creer_file_selection), (GtkWidget*)pWindow);
    gtk_menu_shell_append(GTK_MENU_SHELL(pMenu), pMenuItem);
    pMenuItem = gtk_menu_item_new_with_label("Créer");
    g_signal_connect(G_OBJECT(pMenuItem), "activate", G_CALLBACK(nouveauDazibao), (GtkWidget*)pWindow);
    gtk_menu_shell_append(GTK_MENU_SHELL(pMenu), pMenuItem);
    pMenuItem = gtk_menu_item_new_with_label("Dazibao");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(pMenuItem), pMenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(pMenuBar), pMenuItem);


    pMenu = gtk_menu_new();
    pMenuItem = gtk_menu_item_new_with_label("Quitter");
    g_signal_connect(G_OBJECT(pMenuItem), "activate", G_CALLBACK(quitter), (GtkWidget*)pWindow);
    gtk_menu_shell_append(GTK_MENU_SHELL(pMenu), pMenuItem);
    pMenuItem = gtk_menu_item_new_with_label("J'ai fini!");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(pMenuItem), pMenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(pMenuBar), pMenuItem);

    gtk_box_pack_start(GTK_BOX(pVBox), pMenuBar, FALSE, FALSE, 0);

    gtk_widget_show_all(pWindow);
    gtk_main();

    return EXIT_SUCCESS;
}
