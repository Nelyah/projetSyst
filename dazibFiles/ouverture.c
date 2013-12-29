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
/* Cette fonction est chargée de vérifier si le fichier 
 * spécifié par path est bien un dazibao. si c'est un 
 * dazibao, elle renvoie 0, et 10 sinon.
*/
  
    int fd;
    if((fd=open(path,O_RDONLY))==-1){
        perror("open");
        exit(EXIT_FAILURE);
    }
    if((flock(fd,LOCK_SH))!=0) {
        perror("flock : ");
    }
    if(fstat(fd,&s)!=0) {
        perror("fstat");
        exit(EXIT_FAILURE);
    }
    // Lecture du numero magique et de la version
    unsigned char magic,version;
    if(read(fd,&magic,1)==0){
        perror("read");
        exit(EXIT_FAILURE);
    }
    if(magic!=53) {
        printf("Le numéro magique n'est pas valide\n");
        return 10;
    }
    if(read(fd,&version,1)==0){
        perror("read");
        exit(EXIT_FAILURE);
    }
    if (version!=0) {
      printf("Le numéro de la version n'est pas valide\n");
      return 10;
    }
    // Lecture de mbz
    short int mbz;
    if(read(fd,&mbz,2)==0){
        perror("read");
        exit(EXIT_FAILURE);
    }
    if((flock(fd,LOCK_UN))!=0) {
        perror("flock");
        exit(EXIT_FAILURE);
    }
    return 0;
}

void ouverture2(const char *path){
/* Cette fonction va se charger de remplir la fenêtre principale avec
 * le contenu du fichier dazibao. Pour cela, la fonction va faire appel
 * à la fonction "lectureDazibao" tant que la taille totale de ce qu'on a 
 * lu ne dépasse pas la taille du dazibao.
*/
    int fd;
    if((fd=open(pathToDazibao,O_RDONLY))==-1){
        perror("open");
        exit(EXIT_FAILURE);
    }
    int err;
    if((flock(fd,LOCK_SH))!=0) {
        perror("flock");
        exit(EXIT_FAILURE);
    }
    lseek(fd,4,SEEK_SET);
    unsigned int i=0;
    unsigned long int tailleParcourueFichier=4,tailleLecture;
    num_msg = 0;
    fstat(fd,&s);
    while(tailleParcourueFichier<s.st_size){
        if(isPad1_N!=1 && isComp_Dated!=1){
            i++;
            num_msg ++;
        }
        if((tailleLecture=lectureDazibao(fd))!=-1) {
            tailleParcourueFichier=tailleParcourueFichier+4+tailleLecture;
        }else{
            tailleParcourueFichier++;
        }
  }
  if((err=flock(fd,LOCK_UN))!=0) {
    perror("flock");
    exit(EXIT_FAILURE);
  }
  close(fd);
}



void lancer_dazibao (){
/* Cette fonction est utilisée pour initialiser les fonctions de la fenêtre principale 
 * lorsqu'on consulte le dazibao.
 * C'est elle qui va permettre d'associer chaque bouton de la fenêtre à une fonction spécifique.
*/
    GtkWidget *pMenuBar2;
    GtkWidget *pMenu2;
    GtkWidget *pMenuItem2;
    GtkWidget *pImage2;
    GdkColor color;

// La taille de la fenêtre est en plein écran
    pWindow2 = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(pWindow2), GTK_WIN_POS_CENTER);
    gtk_window_maximize(GTK_WINDOW(pWindow2));
    gtk_window_set_title(GTK_WINDOW(pWindow2), chemin);
    g_signal_connect(G_OBJECT(pWindow2),"destroy",G_CALLBACK(gtk_main_quit), NULL);
    color.pixel = 32;
    color.red = 1235;
    color.green = 155;
    color.blue = 0;
    gtk_widget_modify_bg (pWindow2, GTK_STATE_NORMAL, &color);
  
  // L'image dazibao nommée "i.jpg" est ajoutée en entête
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


    pMenuBar2 = gtk_menu_bar_new(); // Création de la barre de menu

    // Fonctions d'ajout
    pMenu2 = gtk_menu_new();
    // Texte
    pMenuItem2 = gtk_menu_item_new_with_label("Un texte");
    g_signal_connect(G_OBJECT(pMenuItem2), "activate", G_CALLBACK(ajouter_texteN), pWindow2);
    gtk_menu_shell_append(GTK_MENU_SHELL(pMenu2), pMenuItem2);

    // PNG
    pMenuItem2 = gtk_menu_item_new_with_label("Une image PNG");
    g_signal_connect(G_OBJECT(pMenuItem2), "activate", G_CALLBACK(ajouter_pngN), pWindow2);
    gtk_menu_shell_append(GTK_MENU_SHELL(pMenu2), pMenuItem2);

    // JPEG
    pMenuItem2 = gtk_menu_item_new_with_label("Une image JPEG");
    g_signal_connect(G_OBJECT(pMenuItem2), "activate", G_CALLBACK(ajouter_jpegN), pWindow2);
    gtk_menu_shell_append(GTK_MENU_SHELL(pMenu2), pMenuItem2);

    // Compound
    pMenuItem2 = gtk_menu_item_new_with_label("Un compound");
    g_signal_connect(G_OBJECT(pMenuItem2), "activate", G_CALLBACK(ajouter_compoundN), pWindow2);
    gtk_menu_shell_append(GTK_MENU_SHELL(pMenu2), pMenuItem2);

    // Dated
    pMenuItem2 = gtk_menu_item_new_with_label("Un dated");
    g_signal_connect(G_OBJECT(pMenuItem2), "activate", G_CALLBACK(ajouter_datedN), pWindow2);
    gtk_menu_shell_append(GTK_MENU_SHELL(pMenu2), pMenuItem2);

    // Menu ajout
    pMenuItem2 = gtk_menu_item_new_with_label("Ajouter");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(pMenuItem2), pMenu2);
    gtk_menu_shell_append(GTK_MENU_SHELL(pMenuBar2), pMenuItem2);


    // Selection de l'item à supprimer
    pMenu2 = gtk_menu_new();
    pMenuItem2 = gtk_menu_item_new_with_label("L'item...");
    g_signal_connect(G_OBJECT(pMenuItem2), "activate", G_CALLBACK(supprimer), pWindow2);
    gtk_menu_shell_append(GTK_MENU_SHELL(pMenu2), pMenuItem2);
    
    // Menu suppression
    pMenuItem2 = gtk_menu_item_new_with_label("Supprimer");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(pMenuItem2), pMenu2);
    gtk_menu_shell_append(GTK_MENU_SHELL(pMenuBar2), pMenuItem2);


    // Confirmation pour quitter
    pMenu2 = gtk_menu_new();
    pMenuItem2 = gtk_menu_item_new_with_label("Quitter");
    g_signal_connect(G_OBJECT(pMenuItem2), "activate", G_CALLBACK(quitter), pWindow2);
    gtk_menu_shell_append(GTK_MENU_SHELL(pMenu2), pMenuItem2);

    //Menu quitter
    pMenuItem2 = gtk_menu_item_new_with_label("J'ai fini!");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(pMenuItem2), pMenu2);
    gtk_menu_shell_append(GTK_MENU_SHELL(pMenuBar2), pMenuItem2);

    
    // Compression (double clic)
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
