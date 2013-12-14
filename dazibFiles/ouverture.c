#include <stdlib.h>
#include <sys/file.h>
#include <sys/stat.h>
#include "ouverture.h"
#include "typedef.h"
#include "globalvar.h"
#include "ajouter_texte.h"
#include "ajouter_Dated.h"
#include "ajouter_Compound.h"
#include "ajouter_Jpeg.h"
#include "ajouter_Png.h"
#include "suppression.h"
#include "compression.h"
#include "quitter.h"
#include "lecture.h"



int ouverture(const char *path){
  
    FILE*dazibao=fopen(path,"rb");    
    if (dazibao==NULL) {
        printf("Problème à l'ouverture du fichier.\n");
    }
    int err;
    if((err=flock(fileno(dazibao),LOCK_SH))!=0) {
        perror("flock : ");
    }
    if((err=stat(path,&s))!=0) {
        perror("stat : ");
	exit(EXIT_FAILURE);
    }
    unsigned char magic,version;
    fread(&magic,1,1,dazibao);
    if (magic!=53) {
        printf("Le numéro magique n'est pas valide\n");
	return 10;
    }
    fread(&version,1,1,dazibao);
    if (version!=0) {
      printf("Le numéro de la version n'est pas valide\n");
      return 10;
    }
    short int mbz;
    fread(&mbz,2,1,dazibao);
    if((err=flock(fileno(dazibao),LOCK_UN))!=0) {
      perror("flock : ");
    }
    return 0;
}

void ouverture2(const char *path){
printf("le PATH : %s\n", path);
  FILE*f=fopen(pathToDazibao,"rb");
  if (f==NULL) {
    printf("Problème à l'ouverture du fichier.\n");
  }
  int err;
  if((err=flock(fileno(f),LOCK_SH))!=0) {
    perror("flock : ");
  }
  fseek(f,4,SEEK_SET);
  unsigned int i=0;
  num_msg = 0;
  unsigned long int tailleParcourueFichier=4,tailleLecture;
  stat(pathToDazibao,&s);
  while(tailleParcourueFichier<s.st_size){
    if(isPad1_N!=1 && isComp_Dated!=1){
        i++;
        num_msg ++;
        printf("Message n°%d\n",i);
        
    }
    if((tailleLecture=lectureDazibao(f))!=-1) {
      tailleParcourueFichier=tailleParcourueFichier+4+tailleLecture;
    }else{
      tailleParcourueFichier++;
    }
  }


  if((err=flock(fileno(f),LOCK_UN))!=0) {
    perror("flock : ");
  }
  fclose(f);
}



void lancer_dazibao (){
  GtkWidget *pMenuBar2;
  GtkWidget *pMenu2;
  GtkWidget *pMenuItem2;
  GtkWidget *pImage2;
  GdkColor color;

  pWindow2 = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position(GTK_WINDOW(pWindow2), GTK_WIN_POS_CENTER);
  gtk_window_maximize(GTK_WINDOW(pWindow2));
//  gtk_window_set_default_size(GTK_WINDOW(pWindow2), 810, 400);
  gtk_window_set_title(GTK_WINDOW(pWindow2), chemin);
  g_signal_connect(G_OBJECT(pWindow2),"destroy",G_CALLBACK(gtk_main_quit), NULL);
  color.pixel = 32;
  color.red = 1235;
  color.green = 155;
  color.blue = 0;
  gtk_widget_modify_bg (pWindow2, GTK_STATE_NORMAL, &color);
  
  pVBox2 = gtk_vbox_new(FALSE,0);
  gtk_container_add(GTK_CONTAINER(pWindow2),pVBox2);
  pImage2 = gtk_image_new_from_file("i.jpg");
  gtk_box_pack_start(GTK_BOX(pVBox2), pImage2, FALSE, FALSE, 5);
  scrollbar = gtk_scrolled_window_new(NULL, NULL);
  gtk_box_pack_start(GTK_BOX(pVBox2), scrollbar, TRUE, TRUE, 5);

  pTextView = gtk_text_view_new();
  gtk_container_add(GTK_CONTAINER(scrollbar), pTextView);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollbar), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_text_view_set_editable((GTK_TEXT_VIEW(pTextView)),FALSE);
  gtk_text_view_set_cursor_visible((GTK_TEXT_VIEW(pTextView)),FALSE);


  pMenuBar2 = gtk_menu_bar_new();

  pMenu2 = gtk_menu_new();
  pMenuItem2 = gtk_menu_item_new_with_label("Un texte");
  g_signal_connect(G_OBJECT(pMenuItem2), "activate", G_CALLBACK(ajouter_texteN), pWindow2);
  gtk_menu_shell_append(GTK_MENU_SHELL(pMenu2), pMenuItem2);
  pMenuItem2 = gtk_menu_item_new_with_label("Une image PNG");
  g_signal_connect(G_OBJECT(pMenuItem2), "activate", G_CALLBACK(ajouter_pngN), pWindow2);
  gtk_menu_shell_append(GTK_MENU_SHELL(pMenu2), pMenuItem2);
  pMenuItem2 = gtk_menu_item_new_with_label("Une image JPEG");
  g_signal_connect(G_OBJECT(pMenuItem2), "activate", G_CALLBACK(ajouter_jpegN), pWindow2);
  gtk_menu_shell_append(GTK_MENU_SHELL(pMenu2), pMenuItem2);
  pMenuItem2 = gtk_menu_item_new_with_label("Un compound");
  g_signal_connect(G_OBJECT(pMenuItem2), "activate", G_CALLBACK(ajouter_compoundN), pWindow2);
  gtk_menu_shell_append(GTK_MENU_SHELL(pMenu2), pMenuItem2);
  pMenuItem2 = gtk_menu_item_new_with_label("Un dated");
  g_signal_connect(G_OBJECT(pMenuItem2), "activate", G_CALLBACK(ajouter_datedN), pWindow2);
  gtk_menu_shell_append(GTK_MENU_SHELL(pMenu2), pMenuItem2);
  pMenuItem2 = gtk_menu_item_new_with_label("Ajouter");
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(pMenuItem2), pMenu2);
  gtk_menu_shell_append(GTK_MENU_SHELL(pMenuBar2), pMenuItem2);

  pMenu2 = gtk_menu_new();
  pMenuItem2 = gtk_menu_item_new_with_label("L'item...");
  g_signal_connect(G_OBJECT(pMenuItem2), "activate", G_CALLBACK(supprimer), pWindow2);
  gtk_menu_shell_append(GTK_MENU_SHELL(pMenu2), pMenuItem2);
  pMenuItem2 = gtk_menu_item_new_with_label("Supprimer");
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(pMenuItem2), pMenu2);
  gtk_menu_shell_append(GTK_MENU_SHELL(pMenuBar2), pMenuItem2);


  pMenu2 = gtk_menu_new();
  pMenuItem2 = gtk_menu_item_new_with_label("Quitter");
  g_signal_connect(G_OBJECT(pMenuItem2), "activate", G_CALLBACK(quitter), pWindow2);
  gtk_menu_shell_append(GTK_MENU_SHELL(pMenu2), pMenuItem2);
  pMenuItem2 = gtk_menu_item_new_with_label("J'ai fini!");
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(pMenuItem2), pMenu2);
  gtk_menu_shell_append(GTK_MENU_SHELL(pMenuBar2), pMenuItem2);

  pMenu2 = gtk_menu_new();
  pMenuItem2 = gtk_menu_item_new_with_label("Compression");
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(pMenuItem2), pMenu2);
  g_signal_connect(G_OBJECT(pMenuItem2), "activate", G_CALLBACK(compression), pWindow2);
  gtk_menu_shell_append(GTK_MENU_SHELL(pMenuBar2), pMenuItem2);


  gtk_box_pack_start(GTK_BOX(pVBox2), pMenuBar2, FALSE, FALSE, 0);

  ouverture2(pathToDazibao);

  gtk_widget_show_all(pWindow2);

  gtk_main();
  
}
