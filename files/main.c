#include "typedef.h"
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

int main(int argc, char **argv){
  GtkWidget *pMenuBar;
  GtkWidget *pMenu;
  GtkWidget *pMenuItem;
  GtkWidget *pImage;
  

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
  /** Premier sous−menu **/
  /* ETAPE 2 */
  pMenu = gtk_menu_new();
  /* ETAPE 3 */
  pMenuItem = gtk_menu_item_new_with_label("Un Dazibao");
  g_signal_connect(G_OBJECT(pMenuItem), "activate", G_CALLBACK(creer_file_selection), (GtkWidget*)pWindow);
  gtk_menu_shell_append(GTK_MENU_SHELL(pMenu), pMenuItem);
  /* ETAPE 4 */
  pMenuItem = gtk_menu_item_new_with_label("Ouvrir...");
  /* ETAPE 5 */
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(pMenuItem), pMenu);
  /* ETAPE 6 */
  gtk_menu_shell_append(GTK_MENU_SHELL(pMenuBar), pMenuItem);


  /** Second sous−menu **/
  /* ETAPE 2 */
  pMenu = gtk_menu_new();
  /* ETAPE 3 */
  pMenuItem = gtk_menu_item_new_with_label("Quitter");
  g_signal_connect(G_OBJECT(pMenuItem), "activate", G_CALLBACK(quitter), (GtkWidget*)pWindow);
  gtk_menu_shell_append(GTK_MENU_SHELL(pMenu), pMenuItem);
  /* ETAPE 4 */
  pMenuItem = gtk_menu_item_new_with_label("J'ai fini!");
  /* ETAPE 5 */
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(pMenuItem), pMenu);
  /* ETAPE 6 */
  gtk_menu_shell_append(GTK_MENU_SHELL(pMenuBar), pMenuItem);

 /* Ajout du menu a la fenetre */
  gtk_box_pack_start(GTK_BOX(pVBox), pMenuBar, FALSE, FALSE, 0);

  gtk_widget_show_all(pWindow);
  printf("TEST1\n");
  gtk_main();
  printf("TEST2\n");

  return EXIT_SUCCESS;
}
