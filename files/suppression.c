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
#include "suppression.h"
#include "globalvar.h"



void suppr(const char *dazibao, int num){
    struct stat sbuf;
    int lenght=0,fd=0;
    if((fd=open(pathToDazibao,O_RDWR))==-1){
        perror("open :");
        exit(EXIT_FAILURE);
    }
    if(stat(pathToDazibao,&sbuf)==-1){
        perror("stat");
        exit(EXIT_FAILURE);
    }
    if(flock(fd,LOCK_EX)==-1){
        perror("flock");
        exit(EXIT_FAILURE);
    }
    unsigned char* fmap=mmap(NULL,sbuf.st_size,PROT_WRITE|PROT_READ,MAP_SHARED,fd,0);
    if(fmap==MAP_FAILED){
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    printf("pos1 : %ld\nfmap : %d\n",posM[num],fmap[posM[num]]);
    fmap[posM[num]]=1;
    if(num==num_msg){
        if(ftruncate(fd,posM[num])==-1){
            perror("truncate");
            exit(EXIT_FAILURE);
        }
        if(flock(fd,LOCK_UN)==-1){
            perror("flock");
            exit(EXIT_FAILURE);
        }
        if(munmap(fmap,sbuf.st_size)==-1){
            perror("munmap");
            exit(EXIT_FAILURE);
        }
        close(fd);
    }else{
//      printf("pos2 : %ld\n",ftell(f));
        lenght=lireLenght(fmap[posM[num]+1],fmap[posM[num]+2],fmap[posM[num]+3]);
        printf("coucou %d  %d\n",lenght,fmap[0]);
        memset(fmap+(posM[num]+4),0,lenght);
        printf("coucou\n");
        if(flock(fd,LOCK_UN)==-1){
            perror("flock");
            exit(EXIT_FAILURE);
        }
        printf("coucou\n");
        if(munmap(fmap,sbuf.st_size)==-1){
            perror("munmap");
            exit(EXIT_FAILURE);
        }
        close(fd);
        printf("fin suppr\n");
    }
}

void supprimer(){
 
  GtkWidget* pBoite;
  GtkWidget* pEntry;
  const gchar* sNom;

  size_t len;
  unsigned int i=0;
  int valid = 1;
  char c;
 
  pBoite = gtk_dialog_new_with_buttons("Suppression",
				       GTK_WINDOW(pWindow2),
				       GTK_DIALOG_MODAL,
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       NULL);
  
  /* Création de la zone de saisie */
  pEntry = gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(pEntry), "Element a supprimer");
  /* Insertion de la zone de saisie dans la boite de dialogue */
  /* Rappel : paramètre 1 de gtk_box_pack_start de type GtkBox */
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(pBoite)->vbox), pEntry, TRUE, FALSE, 0);
  
  /* Affichage des éléments de la boite de dialogue */
  gtk_widget_show_all(GTK_DIALOG(pBoite)->vbox);
  
  /* On lance la boite de dialogue et on récupéré la réponse */
  switch (gtk_dialog_run(GTK_DIALOG(pBoite)))
    {
      /* L utilisateur valide */
    case GTK_RESPONSE_OK:
      sNom = gtk_entry_get_text(GTK_ENTRY(pEntry));
      len = strlen(sNom);
      
      for(i = 0; i < len; i++){
	      c = sNom[i];
	
	      if ((c < '0') || (c > '9')){
	          valid = 0;
	          break;
	      }
      }
      
      if(valid == 0){
	printf("La chaine n'est pas un nombre entier.\n");
	dialog = gtk_message_dialog_new(GTK_WINDOW(pBoite),GTK_DIALOG_MODAL,
					GTK_MESSAGE_ERROR,GTK_BUTTONS_OK,"Vous devez saisir un nombre entier!");
     
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	gtk_widget_destroy(pBoite);
      }
      else{
	int n=atol(sNom);
	if(n > 0 && n <= num_msg){
	    printf("n=%d\n", n);
	    suppr(pathToDazibao, n);
	    gtk_widget_destroy(pWindow2);
        gtk_widget_destroy(pBoite);	
	    lancer_dazibao();
        }
	else {
	  printf("Vous devez rentrer un nombre compris dans les numeros de messages");
	  dialog = gtk_message_dialog_new(GTK_WINDOW(pBoite),GTK_DIALOG_MODAL,
					  GTK_MESSAGE_ERROR,GTK_BUTTONS_OK,"Vous devez rentrer un nombre compris dans les numeros de messages!");
     
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    gtk_widget_destroy(pBoite);
    }
      }

      break;
      /* L utilisateur annule */
    case GTK_RESPONSE_CANCEL:
    case GTK_RESPONSE_NONE:
    default:
      printf("Vous n'avez rien saisi !");
      dialog = gtk_message_dialog_new(GTK_WINDOW(pBoite),GTK_DIALOG_MODAL,
					  GTK_MESSAGE_ERROR,GTK_BUTTONS_OK,"Vous n'avez rien saisi!");
     
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
      gtk_widget_destroy(pBoite);
      break;
    }
 
  /* Destruction de la boite de dialogue */
  printf("suppression \n");
}
