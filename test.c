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


typedef struct tlv{
    unsigned int lenght;
    unsigned char type;
    char *textOrPath;
    unsigned char* contenuImage;
    char* pathImage;
    int nbTlv;
    struct tlv **tlvList;
    long time;
} tlv;


const gchar* chemin;
tlv *tlvTemp;
char* pathToDazibao;
struct stat s;
const gchar *sText;
int num_msg=0;
long posDep=0;
long *posM=NULL;
int isPad1_N=0, isComp_Dated=0, isDatedNew=0;
GtkWidget *pWindow; GtkWidget *pWindow2;
GtkWidget *pVBox;
GtkWidget *pVBox2;
GtkWidget *pTextView;
GtkWidget *scrollbar;   
GtkWidget *dialog;
int ouverture(const char * path);
void ouverture2(const char * path);
void lectureCompound(FILE* nomBoudazi,unsigned int taille);
int lectureAdibouz(FILE* nomBoudazi);
int lireLenght(unsigned char a, unsigned char b, unsigned char c);
unsigned char ecrireLenght1(int a);
unsigned char ecrireLenght2(int a, unsigned char l1);
unsigned char ecrireLenght3(int a, unsigned char l1, unsigned char l2);
void quitter(GtkWidget* widget, gpointer data);
void creer_file_selection();
void verifier_chemin (GtkWidget *widget, GtkWidget *file_selection);
void lancer_dazibao();
tlv* choisirTlv();
tlv* ajouter_texte(int opt);
void ajouter_texteN();
tlv* ajouter_png(int opt);
void ajouter_pngN();
tlv* ajouter_jpeg(int opt);
void ajouter_jpegN();
void ajouter_compoundN();
tlv* ajouter_compound(int opt);
void ajouter_datedN();
tlv* ajouter_dated(int opt);
void add_jpeg_dated(tlv* tlvJ);
void supprimer();
void compression();
int ajouterMessageCompound(FILE* dazibao,tlv* tlv,int hasLock);
int ajouterMessageJpeg(FILE* dazibao,tlv* tlv,int hasLock);
int ajouterMessagePng(FILE* dazibao,tlv* tlv,int hasLock);
int ajouterMessageTxt(FILE* dazibao,tlv* tlv,int hasLock);
int ajouterMessageDated(FILE* dazibao,tlv* tlv,int hasLock);
tlv* chemin_jpeg(GtkWidget *widget, GtkWidget *file_selection);
void nouveauDziboa(FILE* nomDiboza);
void suppr(const char *dazibao, int num);


tlv* newTlv(int type){
    tlv* new_msg=NULL;
    if((new_msg=malloc(sizeof(tlv)))==NULL){
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    new_msg->contenuImage=NULL;
    new_msg->nbTlv=0;
    new_msg->pathImage=NULL,
    new_msg->time=0;
    new_msg->lenght=0;
    new_msg->tlvList=NULL;
    new_msg->textOrPath=NULL;
    new_msg->type=type;
    return new_msg;
}

void freeTlv(tlv* tlv){
printf("Entrée dans le free\n");
    if(tlv->contenuImage!=NULL)
        free(tlv->contenuImage);
printf("Contenu free, pathImage now...\n");
    if(tlv->pathImage!=NULL)
        free(tlv->pathImage);
printf("PathImage free, textOrPath now...\n");
    if(tlv->textOrPath!=NULL)
        free(tlv->textOrPath);
printf("free du tlv...\n");
    free(tlv);
printf("Sortie du free\n");
}

tlv* tlvCopy(tlv* tlv1){
    tlv* tlv2=NULL;
    tlv2=newTlv(tlv1->type);
    tlv2->nbTlv=tlv1->nbTlv;
    if(tlv1->pathImage!=NULL){ // C'est un tlv Image (PNG ou JPEG)
        int fd;
        struct stat statBuf;
        unsigned char* fmap=NULL;
        if((tlv2->pathImage=malloc((strlen(tlv1->pathImage)+1)*sizeof(char)))==NULL){
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        strcpy(tlv2->pathImage,tlv1->pathImage);
        printf("avant le stat\n");
        if(stat(tlv2->pathImage,&statBuf)==-1){
            perror("stat");
            exit(EXIT_FAILURE);
        }
        printf("après le stat\n");
        tlv2->lenght=statBuf.st_size;
        if((tlv2->contenuImage=malloc((statBuf.st_size+1)*sizeof(unsigned char)))==NULL){
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        if((fd=open(tlv1->pathImage,O_RDONLY))==-1){
            perror("open");
            exit(EXIT_FAILURE);
        }
        fmap=mmap(NULL,statBuf.st_size,PROT_READ,MAP_PRIVATE,fd,0);
        if(fmap==MAP_FAILED){
            perror("mmap");
            exit(EXIT_FAILURE);
        }
        memcpy(tlv2->contenuImage,fmap,statBuf.st_size);
        munmap(fmap,statBuf.st_size);
        if(close(fd)==-1){
            perror("close");
            exit(EXIT_FAILURE);
        }
    }
    if(tlv1->textOrPath!=NULL){
        if((tlv2->textOrPath=malloc((strlen(tlv1->textOrPath)+1)*sizeof(char)))==NULL){
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        strcpy(tlv2->textOrPath,tlv1->textOrPath);
    }
    if(tlv1->tlvList!=NULL){
        tlv2->tlvList=tlv1->tlvList;
    }
    if(tlv1->time!=0){
        tlv2->time=tlv1->time;
    }
    freeTlv(tlv1);
    return tlv2;
}


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

void ajouter_texteN(){
    ajouter_texte(0);
}

void lectureCompound(FILE* nomBoudazi,unsigned int taille){
    unsigned int tailleParcourue=0;
    unsigned long int tailleLecture=0;
    while(taille>tailleParcourue){
        if((tailleLecture=lectureAdibouz(nomBoudazi))!=1) {
            tailleParcourue=tailleParcourue+4+tailleLecture;
        }else{
            tailleParcourue++;
        }
    }
}

int lireLenght(unsigned char a, unsigned char b, unsigned char c){
    return(256*256*a+256*b+c);
}

int lectureAdibouz(FILE* nomBoudazi){
  GtkTextIter end;
  GtkTextBuffer *buf;
  unsigned char type, lenght1=0, lenght2=0, lenght3=0;
  unsigned int lenght = 0;
  char *contenu="";
  char *num_msg2=NULL;
 
  GError *error = NULL;
  GdkPixbuf *pix;

  gchar *nomImage = "i.jpg";

  num_msg2=malloc(10*sizeof(unsigned char));
  sprintf(num_msg2, "%d", num_msg);
  printf("%s\n",num_msg2);
  
  fread(&type,1,1,nomBoudazi);
  buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pTextView));
  gtk_text_view_set_wrap_mode((GTK_TEXT_VIEW(pTextView)),GTK_WRAP_WORD);
  gtk_text_view_set_justification((GTK_TEXT_VIEW(pTextView)),GTK_JUSTIFY_CENTER);
  gtk_text_buffer_get_end_iter(buf,&end);
  printf("need to know : type =%d, ispad1_n=%d isComp_D=%d\n",type,isPad1_N,isComp_Dated);
  if((type!=0 || type!=1) && isPad1_N==0 && isComp_Dated==0){
  printf("------------------------------------------------------\n");
    gtk_text_buffer_insert(buf, &end, num_msg2, -1);
    gtk_text_buffer_insert(buf, &end, "------------------------------------------------------------------------------------------------------------------------------------\n", -1);
  }
  free(num_msg2);
  isPad1_N=0;
  if (type==0) {
  printf("pouet\n");
    isPad1_N=1;
    return -1;
  }
  printf("Type : %d\n",type);
  if(isPad1_N==0 && isComp_Dated==0){
    posM[num_msg]=ftell(nomBoudazi)-1;
  }
  fread(&lenght1,1,1,nomBoudazi);
  fread(&lenght2,1,1,nomBoudazi);
  fread(&lenght3,1,1,nomBoudazi);
  lenght=lireLenght(lenght1,lenght2,lenght3);
  if((contenu=malloc((lenght+1)*sizeof(unsigned char)))==NULL && lenght!=0){
    perror("Malloc : ");
    exit(EXIT_FAILURE);
  }
  if (type==5) {
  printf("C'est un compound\n");
    isComp_Dated=1;
    lectureCompound(nomBoudazi,lenght);
    free(contenu);
    isComp_Dated=0;
    return lenght;
  }else if (type==6) {
  printf("C'est un dated\n");
    time_t secondes=0;
    unsigned char l1,l2,l3,l4;
//    secondes=time(NULL);
//    ctime(&secondes);
    isComp_Dated=1;
//    fread(&secondes,4,1,nomBoudazi);
    fread(&l1,1,1,nomBoudazi);
    fread(&l2,1,1,nomBoudazi);
    fread(&l3,1,1,nomBoudazi);
    fread(&l4,1,1,nomBoudazi);
    secondes=256*256*256*l1+256*256*l2+256*l3+l4;
    printf("secondes : %ld\n",secondes);
    printf("%s",ctime(&secondes));
    buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pTextView));
    gtk_text_view_set_wrap_mode((GTK_TEXT_VIEW(pTextView)),GTK_WRAP_WORD);
    gtk_text_view_set_justification((GTK_TEXT_VIEW(pTextView)),GTK_JUSTIFY_CENTER);
    gtk_text_buffer_get_end_iter(buf,&end);
    gtk_text_buffer_insert(buf, &end, g_locale_to_utf8(ctime(&secondes), -1, NULL, NULL, NULL), -1);
    lectureAdibouz(nomBoudazi);
    free(contenu);
    isComp_Dated=0;
    return lenght;
  }
  fread(contenu,lenght,1,nomBoudazi);
  if (type==1) {
    isPad1_N=1;
    free(contenu);
    return lenght;
  }else if (type==2) {
    contenu[lenght]='\n';
    contenu[lenght+1]='\0';
    printf("%s\n",contenu);
    buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pTextView));
    gtk_text_view_set_wrap_mode((GTK_TEXT_VIEW(pTextView)),GTK_WRAP_WORD);
    gtk_text_view_set_justification((GTK_TEXT_VIEW(pTextView)),GTK_JUSTIFY_CENTER);
    gtk_text_buffer_get_end_iter(buf,&end);
    gtk_text_buffer_insert(buf, &end, g_locale_to_utf8(contenu, -1, NULL, NULL, NULL), -1);
    free(contenu);
    return lenght;
  }else if (type==3) {
    char tempFile[]="/tmp/test.dbzXXXXXX";
    nomImage=mktemp(tempFile);
    FILE *f=NULL;
    if((f=fopen(nomImage,"w+b"))==NULL){
        perror("fopen ");
        exit(EXIT_FAILURE);
    }
    fwrite(contenu,lenght,1,f);
    fclose(f);
//    filename="i.jpg";
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
    printf("C'est un PNG !!!\n");
    free(contenu);
    return lenght;
  }else if (type==4) {
    char tempFile[]="/tmp/test.dbzXXXXXX";
    nomImage=mktemp(tempFile);
    FILE *f=NULL;
    if((f=fopen(nomImage,"w+b"))==NULL){
        perror("fopen ");
        exit(EXIT_FAILURE);
    }
    fwrite(contenu,lenght,1,f);
    fclose(f);
//    filename="i.jpg";
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
    printf("C'est un JPEG !!!\n");
    free(contenu);
    return lenght;
  }else{
    printf("TLV inconnue.\n");
    free(contenu);
    return lenght;
  }
  
}


int ouverture(const char *path){
  
    FILE*nomBoudazi=fopen(path,"rb");    
    if (nomBoudazi==NULL) {
        printf("Problème à l'ouverture du fichier.\n");
    }
    int err;
    if((err=flock(fileno(nomBoudazi),LOCK_SH))!=0) {
        perror("flock : ");
    }
    if((err=stat(path,&s))!=0) {
        perror("stat : ");
	exit(EXIT_FAILURE);
    }
    unsigned char magic,version;
    fread(&magic,1,1,nomBoudazi);
    if (magic!=53) {
        printf("Le numéro magique n'est pas valide\n");
	return 10;
    }
    fread(&version,1,1,nomBoudazi);
    if (version!=0) {
      printf("Le numéro de la version n'est pas valide\n");
      return 10;
    }
    short int mbz;
    fread(&mbz,2,1,nomBoudazi);
    if((err=flock(fileno(nomBoudazi),LOCK_UN))!=0) {
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
  printf("sizeFile = %d\n",s.st_size);
  while(tailleParcourueFichier<s.st_size){
  printf("tailleP = %ld\n",tailleParcourueFichier);
    if(isPad1_N!=1 && isComp_Dated!=1){
        i++;
        num_msg ++;
        printf("Message n°%d\n",i);
        
    }
    if((tailleLecture=lectureAdibouz(f))!=-1) {
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


unsigned char ecrireLenght1(int a){
    if(a<(256*256+256)){
        return 0;
    }else {
        return (a/(256*256));
    }
}

unsigned char ecrireLenght2(int a, unsigned char l1){
    if(a<(256)){
        return 0;
    }else {
        return ((a-(256*256*l1))/256);
    }
}


unsigned char ecrireLenght3(int a, unsigned char l1, unsigned char l2){
    if(a<(256)){
        return a;
    }else {
        return (a-(256*256*l1)-(256*l2));
    }
}


tlv* ajouter_texte(int opt){
    GtkWidget* pBoite;
    GtkWidget* pEntry;
    const gchar* sNom;
 
  /* Création de la boite de dialogue */
  /* 1 bouton Valider */
  /* 1 bouton Annuler */
    pBoite = gtk_dialog_new_with_buttons("Ajout",
				       GTK_WINDOW(pWindow2),
				       GTK_DIALOG_MODAL,
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       NULL);
 
    pEntry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(pEntry), "Saisissez votre texte");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(pBoite)->vbox), pEntry, TRUE, FALSE, 0);
    gtk_widget_show_all(GTK_DIALOG(pBoite)->vbox);
    switch (gtk_dialog_run(GTK_DIALOG(pBoite))){
        case GTK_RESPONSE_OK:
            sNom = gtk_entry_get_text(GTK_ENTRY(pEntry));
            printf("%s\n",sNom); 
            tlv* new_msg=NULL;
            new_msg=newTlv(2);
            if((new_msg->textOrPath=malloc((strlen(sNom)+1)*sizeof(char)))==NULL){
                perror("malloc");
                exit(EXIT_FAILURE);
            }
            strcpy(new_msg->textOrPath,(char*)sNom);
            new_msg->lenght=strlen(new_msg->textOrPath);
            if(opt==0){
                num_msg++;
                struct stat statBuf;
                FILE *f=fopen(pathToDazibao,"a+b");
                if(stat(pathToDazibao,&statBuf)==-1){
                    perror("stat");
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
                ajouterMessageTxt(f,new_msg,0);
                fclose(f);
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
            printf("Vous n'avez rien saisi !");
            gtk_widget_destroy(pEntry);	
            gtk_widget_destroy(pBoite);
            break;
    }
    printf("ajout texte \n");
    return NULL;
}

int ajouterMessageTxt(FILE* dazibao,tlv* tlv,int hasLock){
    int err;
    printf("filed %d\n",fileno(dazibao));
    if (hasLock!=1) {
        if((err=flock(fileno(dazibao),LOCK_EX))!=0){
            perror("flock : ");
            exit(EXIT_FAILURE);
        }
    }
    if(tlv->lenght>16777216) { // taille max d'un entier sur 24 bits unsigned
        printf("Erreur : Le message est trop long \n");
        exit(EXIT_FAILURE);
    }
    if((err=fwrite(&tlv->type,1,1,dazibao))==0) {
        perror("fwrite :");
        exit(EXIT_FAILURE);
    }
    unsigned int lenght=tlv->lenght; 
    unsigned char l1=0,l2=0,l3=0;
    l1=ecrireLenght1(lenght);
    l2=ecrireLenght2(lenght,l1);
    l3=ecrireLenght3(lenght,l1,l2);
    printf("lenght:%d, tlvLen=%d l1=%d l2=%d l3=%d\n",lenght,tlv->lenght,l1,l2,l3);
    if ((err=fwrite(&l1,1,1,dazibao))==0) {
        perror("fwrite :");
        exit(EXIT_FAILURE);
    }
    if ((err=fwrite(&l2,1,1,dazibao))==0) {
        perror("fwrite :");
        exit(EXIT_FAILURE);
    }
    if ((err=fwrite(&l3,1,1,dazibao))==0) {
        perror("fwrite :");
        exit(EXIT_FAILURE);
    }
    printf("texte : %s\n",tlv->textOrPath);
    if((err=fwrite(tlv->textOrPath,tlv->lenght,1,dazibao))==0) {
        perror("fwrite :");
        exit(EXIT_FAILURE);
    }
    printf("err=%d\n%d\n",err,tlv->lenght);
    if(hasLock!=1) {
        if((err=flock(fileno(dazibao),LOCK_UN))!=0){
            perror("flock : ");
            exit(EXIT_FAILURE);
        }
    }
    freeTlv(tlv);
    printf("pouet fin txt\n");
    return 0;
}


int ajouterMessageDated(FILE *dazibao,tlv* tlv,int hasLock){
printf("coucou\n");
    int fd=fileno(dazibao);
    GtkTextIter end;
    GtkTextBuffer *buf;
    buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pTextView));
    gtk_text_view_set_wrap_mode((GTK_TEXT_VIEW(pTextView)),GTK_WRAP_WORD);
    gtk_text_view_set_justification((GTK_TEXT_VIEW(pTextView)),GTK_JUSTIFY_CENTER);
    gtk_text_buffer_get_end_iter(buf,&end);
    int err;
    if(hasLock!=1) {
        if((err=flock(fd,LOCK_EX))!=0){
            perror("flock ");
            exit(EXIT_FAILURE);
        }
    }
    if((err=fwrite(&tlv->type,1,1,dazibao))==0) {
        perror("fwrite :");
        exit(EXIT_FAILURE);
    }
printf("coucou\n");
    unsigned int lenght=tlv->lenght;
    unsigned char l1=0,l2=0,l3=0;
    l1=ecrireLenght1(lenght);
    l2=ecrireLenght2(lenght,l1);
    l3=ecrireLenght3(lenght,l1,l2);
    if ((err=fwrite(&l1,1,1,dazibao))==0) {
        perror("fwrite :");
        exit(EXIT_FAILURE);
    }
    if ((err=fwrite(&l2,1,1,dazibao))==0) {
        perror("fwrite :");
        exit(EXIT_FAILURE);
    }
    if ((err=fwrite(&l3,1,1,dazibao))==0) {
        perror("fwrite :");
        exit(EXIT_FAILURE);
    }
    unsigned int t1=0,t2=0,t3=0,t4=0;
    if(tlv->time<(256*256*256+256*256+256)){
        t1=0;
    }else{
        t1=tlv->time/(256*256*256);
    }
    if(tlv->time<(256*256+256)){
        t2=0;
    }else{
        t2=(tlv->time-(256*256*256*t1))/(256*256);
    }
    if(tlv->time<(256)){
        t3=0;
    }else{
        t3=(tlv->time-(256*256*256*t1)-(256*256*t2))/(256);
    }
    if(tlv->time<(256)){
        t4=tlv->time;
    }else{
        t4=(tlv->time-(256*256*256*t1)-(256*256*t2)-(256*t3));
    }
    gtk_text_buffer_insert(buf, &end, g_locale_to_utf8(ctime(&tlv->time), -1, NULL, NULL, NULL), -1);
    if((err=fwrite(&t1,1,1,dazibao))==0) {
        perror("fwrite :");
        exit(EXIT_FAILURE);
    }
printf("coucou\n");
    if((err=fwrite(&t2,1,1,dazibao))==0) {
        perror("fwrite :");
        exit(EXIT_FAILURE);
    }
    if((err=fwrite(&t3,1,1,dazibao))==0) {
        perror("fwrite :");
        exit(EXIT_FAILURE);
    }
    if((err=fwrite(&t4,1,1,dazibao))==0) {
        perror("fwrite :");
        exit(EXIT_FAILURE);
    }
printf("coucou\n");
    if(tlv->tlvList[0]->type==2) {
        gtk_text_buffer_insert(buf, &end, g_locale_to_utf8(tlv->tlvList[0]->textOrPath, -1, NULL, NULL, NULL), -1);
        gtk_text_buffer_insert(buf, &end, g_locale_to_utf8("\n", -1, NULL, NULL, NULL), -1);
        ajouterMessageTxt(dazibao,tlv->tlvList[0],1);
    }else if(tlv->tlvList[0]->type==3){
        ajouterMessagePng(dazibao,tlv->tlvList[0],1);
    }else if(tlv->tlvList[0]->type==4){
        ajouterMessageJpeg(dazibao,tlv->tlvList[0],1);
    }else if(tlv->tlvList[0]->type==5){ // Ne pas faire pour l'instant
        ajouterMessageCompound(dazibao,tlv->tlvList[0],1);
    }else if(tlv->tlvList[0]->type==6){
        ajouterMessageDated(dazibao,tlv->tlvList[0],1);
    }
    if(hasLock!=1) {
        if((err=flock(fd,LOCK_UN))!=0){
            perror("flock ");
            exit(EXIT_FAILURE);
        }
    }
printf("coucou\n");
printf("coucoui\n");
    freeTlv(tlv);
printf("coucouf\n");
    return 0;
}

int ajouterMessagePng(FILE* dazibao,tlv* tlv,int hasLock){
    if (hasLock!=1) {
        if(flock(fileno(dazibao),LOCK_EX)!=0){
            perror("flock : ");
            exit(EXIT_FAILURE);
        }
    }
    GError *error = NULL;
    GdkPixbuf *pix;
    GtkTextIter end;
    GtkTextBuffer *buf;
    buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pTextView));
    gtk_text_view_set_wrap_mode((GTK_TEXT_VIEW(pTextView)),GTK_WRAP_WORD);
    gtk_text_view_set_justification((GTK_TEXT_VIEW(pTextView)),GTK_JUSTIFY_CENTER);
    gtk_text_buffer_get_end_iter(buf,&end);
    pix = gdk_pixbuf_new_from_file_at_size(tlv->pathImage,400,400, &error);
    if (error){
	    GtkWidget *msg;
	    msg = gtk_message_dialog_new (GTK_WINDOW (pWindow2), 
	                                    GTK_DIALOG_MODAL,
				                        GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, 
				                        error->message);
	    gtk_dialog_run (GTK_DIALOG (msg));
	    gtk_widget_destroy (msg);
	    exit(EXIT_FAILURE);
    }


    if(tlv->lenght>16777216) { // taille max d'un entier sur 24 bits unsigned
        printf("Erreur : La taille de l'image est trop grande \n");
        exit(EXIT_FAILURE);
    }
    if (fwrite(&tlv->type,1,1,dazibao)==0) {
        perror("fwrite ");
        exit(EXIT_FAILURE);
    }
    unsigned int lenght=tlv->lenght; 
    unsigned char l1=0,l2=0,l3=0;
    l1=ecrireLenght1(lenght);
    l2=ecrireLenght2(lenght,l1);
    l3=ecrireLenght3(lenght,l1,l2);
    if (fwrite(&l1,1,1,dazibao)==0) {
        perror("fwrite :");
        exit(EXIT_FAILURE);
    }
    if (fwrite(&l2,1,1,dazibao)==0) {
        perror("fwrite :");
        exit(EXIT_FAILURE);
    }
    if (fwrite(&l3,1,1,dazibao)==0) {
        perror("fwrite :");
        exit(EXIT_FAILURE);
    }
    printf("tlvL= %d\n",tlv->lenght);
    if(fwrite(tlv->contenuImage,tlv->lenght,1,dazibao)==0){
        perror("fwrite");
        exit(EXIT_FAILURE);
    }
    gtk_text_buffer_insert_pixbuf(buf,&end, pix);
    gtk_text_buffer_insert(buf, &end, "\n", -1);
    if (hasLock!=1) {
        if(flock(fileno(dazibao),LOCK_UN)!=0){
            perror("flock : ");
            exit(EXIT_FAILURE);
        }
    }
    freeTlv(tlv);
    return 0;
}

int ajouterMessageJpeg(FILE* dazibao,tlv* tlv, int hasLock){
    if (hasLock!=1) {
        if(flock(fileno(dazibao),LOCK_EX)!=0){
            perror("flock : ");
            exit(EXIT_FAILURE);
        }
    }
    GError *error = NULL;
    GdkPixbuf *pix;
    GtkTextIter end;
    GtkTextBuffer *buf;
    buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pTextView));
    gtk_text_view_set_wrap_mode((GTK_TEXT_VIEW(pTextView)),GTK_WRAP_WORD);
    gtk_text_view_set_justification((GTK_TEXT_VIEW(pTextView)),GTK_JUSTIFY_CENTER);
    gtk_text_buffer_get_end_iter(buf,&end);
    pix = gdk_pixbuf_new_from_file_at_size(tlv->pathImage,400,400, &error);
    if (error){
	    GtkWidget *msg;
	    msg = gtk_message_dialog_new (GTK_WINDOW (pWindow2), 
	                                    GTK_DIALOG_MODAL,
				                        GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, 
				                        error->message);
	    gtk_dialog_run (GTK_DIALOG (msg));
	    gtk_widget_destroy (msg);
	    exit(EXIT_FAILURE);
    }


    if(tlv->lenght>16777216) { // taille max d'un entier sur 24 bits unsigned
        printf("Erreur : La taille de l'image est trop grande \n");
        exit(EXIT_FAILURE);
    }
    if (fwrite(&tlv->type,1,1,dazibao)==0) {
        perror("fwrite ");
        exit(EXIT_FAILURE);
    }
    unsigned int lenght=tlv->lenght; 
    unsigned char l1=0,l2=0,l3=0;
    l1=ecrireLenght1(lenght);
    l2=ecrireLenght2(lenght,l1);
    l3=ecrireLenght3(lenght,l1,l2);
    if (fwrite(&l1,1,1,dazibao)==0) {
        perror("fwrite :");
        exit(EXIT_FAILURE);
    }
    if (fwrite(&l2,1,1,dazibao)==0) {
        perror("fwrite :");
        exit(EXIT_FAILURE);
    }
    if (fwrite(&l3,1,1,dazibao)==0) {
        perror("fwrite :");
        exit(EXIT_FAILURE);
    }
    printf("tlvL= %d\n",tlv->lenght);
    if(fwrite(tlv->contenuImage,tlv->lenght,1,dazibao)==0){
        perror("fwrite");
        exit(EXIT_FAILURE);
    }
    gtk_text_buffer_insert_pixbuf(buf,&end, pix);
    gtk_text_buffer_insert(buf, &end, "\n", -1);
    if (hasLock!=1) {
        if(flock(fileno(dazibao),LOCK_UN)!=0){
            perror("flock : ");
            exit(EXIT_FAILURE);
        }
    }
    freeTlv(tlv);
    return 0;
}

void ajouter_pngN(){
    ajouter_png(0);
}

tlv* ajouter_png(int opt){
    printf("ajout png \n");
    char* filename=NULL;
    GtkWidget *selection;
    selection = gtk_file_chooser_dialog_new( g_locale_to_utf8( "Sélectionnez un fichier", -1, NULL, NULL, NULL),
                                            GTK_WINDOW(pWindow2), GTK_FILE_CHOOSER_ACTION_OPEN,
                                            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				                            GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                            NULL);

    if(gtk_dialog_run(GTK_DIALOG(selection))==GTK_RESPONSE_ACCEPT)
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER (selection));
 
    dialog = gtk_message_dialog_new(GTK_WINDOW(selection),GTK_DIALOG_MODAL,
	    GTK_MESSAGE_INFO,GTK_BUTTONS_OK,"Vous avez choisi :\n%s", filename);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    gtk_widget_destroy(selection);
    int fd;
    if((fd=open(filename,O_RDONLY))==-1){
        perror("open");
        exit(EXIT_FAILURE);
    }
    struct stat buf;
    if(fstat(fd,&buf)==-1){
        perror("fstat");
        exit(EXIT_FAILURE);
    }
    printf("tailleImage: %d\n",buf.st_size);
    tlv* tlvJpeg=NULL;
    tlvJpeg=newTlv(4);
    if((tlvJpeg->contenuImage=malloc((buf.st_size+1)*sizeof(unsigned char)))==NULL){
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    tlvJpeg->lenght=buf.st_size;
    unsigned char* fmap=mmap(NULL,buf.st_size,PROT_READ,MAP_PRIVATE,fd,0);
    if(fmap==MAP_FAILED){
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    memcpy(tlvJpeg->contenuImage,fmap,buf.st_size);
    if(munmap(fmap,buf.st_size)==-1){
        perror("munmap");
        exit(EXIT_FAILURE);
    }
    if(close(fd)==-1){
        perror("close");
        exit(EXIT_FAILURE);
    }
    if((tlvJpeg->pathImage=malloc((strlen(filename)+1)*sizeof(char)))==NULL){
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    strcpy(tlvJpeg->pathImage,filename);

    printf("path : %s\n",tlvJpeg->pathImage);
    if(opt==0){
        num_msg++;
        char* num_msg2=NULL;
        if((num_msg2=malloc(10*sizeof(char)))==NULL){
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        sprintf(num_msg2,"%d",num_msg);
        printf("num msg%d\n",num_msg);
        GtkTextIter end;
        GtkTextBuffer *buf2;
        buf2 = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pTextView));
        gtk_text_view_set_wrap_mode((GTK_TEXT_VIEW(pTextView)),GTK_WRAP_WORD);
        gtk_text_view_set_justification((GTK_TEXT_VIEW(pTextView)),GTK_JUSTIFY_CENTER);
        gtk_text_buffer_get_end_iter(buf2,&end);
        gtk_text_buffer_insert(buf2, &end, num_msg2, -1);
        gtk_text_buffer_insert(buf2, &end, "------------------------------------------------------------------------------------------------------------------------------------\n", -1);
        free(num_msg2);
        FILE* dazibao=NULL;
        if((dazibao=fopen(pathToDazibao,"a+b"))==NULL){
            perror("fopen");
            exit(EXIT_FAILURE);
        }
        struct stat statBuf;
        if(stat(pathToDazibao,&statBuf)==-1){
            perror("stat");
            exit(EXIT_FAILURE);
        }
        posM[num_msg]=statBuf.st_size;
        ajouterMessageJpeg(dazibao,tlvJpeg, 0);
        fclose(dazibao);
        return NULL;
    }else{
        return tlvJpeg;
    }
    return NULL;


}

void ajouter_jpegN(){
    ajouter_jpeg(0);
}
tlv* ajouter_jpeg(int opt){
    char* filename=NULL;
    GtkWidget *selection;
    selection = gtk_file_chooser_dialog_new( g_locale_to_utf8( "Sélectionnez un fichier", -1, NULL, NULL, NULL),
                                            GTK_WINDOW(pWindow2), GTK_FILE_CHOOSER_ACTION_OPEN,
                                            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				                            GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                            NULL);

    if(gtk_dialog_run(GTK_DIALOG(selection))==GTK_RESPONSE_ACCEPT)
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER (selection));
 
    dialog = gtk_message_dialog_new(GTK_WINDOW(selection),GTK_DIALOG_MODAL,
	    GTK_MESSAGE_INFO,GTK_BUTTONS_OK,"Vous avez choisi :\n%s", filename);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    gtk_widget_destroy(selection);
    int fd;
    if((fd=open(filename,O_RDONLY))==-1){
        perror("open");
        exit(EXIT_FAILURE);
    }
    struct stat buf;
    if(fstat(fd,&buf)==-1){
        perror("fstat");
        exit(EXIT_FAILURE);
    }
    printf("tailleImage: %d\n",buf.st_size);
    tlv* tlvJpeg=NULL;
    tlvJpeg=newTlv(4);
    if((tlvJpeg->contenuImage=malloc((buf.st_size+1)*sizeof(unsigned char)))==NULL){
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    tlvJpeg->lenght=buf.st_size;
    unsigned char* fmap=mmap(NULL,buf.st_size,PROT_READ,MAP_PRIVATE,fd,0);
    if(fmap==MAP_FAILED){
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    memcpy(tlvJpeg->contenuImage,fmap,buf.st_size);
    if(munmap(fmap,buf.st_size)==-1){
        perror("munmap");
        exit(EXIT_FAILURE);
    }
    if(close(fd)==-1){
        perror("close");
        exit(EXIT_FAILURE);
    }
    if((tlvJpeg->pathImage=malloc((strlen(filename)+1)*sizeof(char)))==NULL){
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    strcpy(tlvJpeg->pathImage,filename);

    printf("path : %s\n",tlvJpeg->pathImage);
    if(opt==0){
        num_msg++;
        char* num_msg2=NULL;
        if((num_msg2=malloc(10*sizeof(char)))==NULL){
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        sprintf(num_msg2,"%d",num_msg);
        printf("num msg%d\n",num_msg);
        GtkTextIter end;
        GtkTextBuffer *buf2;
        buf2 = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pTextView));
        gtk_text_view_set_wrap_mode((GTK_TEXT_VIEW(pTextView)),GTK_WRAP_WORD);
        gtk_text_view_set_justification((GTK_TEXT_VIEW(pTextView)),GTK_JUSTIFY_CENTER);
        gtk_text_buffer_get_end_iter(buf2,&end);
        gtk_text_buffer_insert(buf2, &end, num_msg2, -1);
        gtk_text_buffer_insert(buf2, &end, "------------------------------------------------------------------------------------------------------------------------------------\n", -1);
        free(num_msg2);
        FILE* dazibao=NULL;
        if((dazibao=fopen(pathToDazibao,"a+b"))==NULL){
            perror("fopen");
            exit(EXIT_FAILURE);
        }
        struct stat statBuf;
        if(stat(pathToDazibao,&statBuf)==-1){
            perror("stat");
            exit(EXIT_FAILURE);
        }
        posM[num_msg]=statBuf.st_size;
        ajouterMessageJpeg(dazibao,tlvJpeg, 0);
        fclose(dazibao);
        return NULL;
    }else{
        printf("test de retour ?\n");
        return tlvJpeg;
    }
    return NULL;


}

void ajouter_compoundN(){
    ajouter_compound(0);
}

tlv* ajouter_compound(int opt){
    printf("ajout compound \n");
    GtkWidget* pBoite;
    GtkWidget* pEntry;
    const gchar* type;
    int len,i,valid;
    char c;
    FILE* dazibao;
    time_t now;
    tlv* tlvComp=NULL;
    tlv* tlvDated=NULL;


    pBoite = gtk_dialog_new_with_buttons("Nombre de TLVs",
				       GTK_WINDOW(pWindow2),
				       GTK_DIALOG_MODAL,
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       NULL);
    pEntry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(pEntry), "Entrez le nombre de TLVs voulu");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(pBoite)->vbox), pEntry, TRUE, FALSE, 0);
    gtk_widget_show_all(GTK_DIALOG(pBoite)->vbox);
    switch (gtk_dialog_run(GTK_DIALOG(pBoite))){
      /* L utilisateur valide */
        case GTK_RESPONSE_OK:
            type = gtk_entry_get_text(GTK_ENTRY(pEntry));
            len = strlen(type);
      
            for(i = 0; i < len; i++){
	            c = type[i];
	
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
            }else{
	            int n=atol(type);
                tlvComp=newTlv(5);
                tlvComp->nbTlv=n;
                if((tlvComp->tlvList=malloc((n+1)*sizeof(tlv*)))==NULL){
                    perror("malloc");
                    exit(EXIT_FAILURE);
                }
                for(i=0;i<n;i++){
                    tlvComp->tlvList[i]=choisirTlv();               
                    tlvComp->lenght=tlvComp->lenght+tlvComp->tlvList[i]->lenght+4;
                    printf("choix num : %d\n",i+1);
                }
            }
            if(opt==0){
                num_msg++;
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
                struct stat statBuf;
                if(stat(pathToDazibao,&statBuf)==-1){
                    perror("stat");
                    exit(EXIT_FAILURE);
                }
                posM[num_msg]=statBuf.st_size;
                FILE* dazibao=NULL;
                if((dazibao=fopen(pathToDazibao,"a+b"))==NULL){
                    perror("fopen");
                    exit(EXIT_FAILURE);
                }
                ajouterMessageCompound(dazibao,tlvComp,0);
                fclose(dazibao);
	            gtk_widget_destroy(pBoite);
                return NULL;
            }else{
	            gtk_widget_destroy(pBoite);
                    printf("après choix\n");
                return(tlvComp);
            }
	        gtk_widget_destroy(pBoite);
            break;
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
    return NULL;

}

tlv* choisirTlv(){
    GtkWidget* pBoite;
    GtkWidget* pEntry;
    const gchar* type;
    int len,i,valid;
    char c;
    time_t now;
    tlv* tlv=NULL;


    pBoite = gtk_dialog_new_with_buttons("Choix du TLV",
				       GTK_WINDOW(pWindow2),
				       GTK_DIALOG_MODAL,
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       NULL);
    pEntry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(pEntry), "Entrez le type du TLV voulu");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(pBoite)->vbox), pEntry, TRUE, FALSE, 0);
    gtk_widget_show_all(GTK_DIALOG(pBoite)->vbox);
    switch (gtk_dialog_run(GTK_DIALOG(pBoite))){
      /* L utilisateur valide */
        case GTK_RESPONSE_OK:
            type=gtk_entry_get_text(GTK_ENTRY(pEntry));
            len=strlen(type);
            for(i=0; i<len;i++){
	            c=type[i];
	            if((c<'0') || (c>'9')){
	                valid=0;
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
            }else{
	            int n=atol(type);
	            if(n > 1 && n <= TLV_MAX){
	                printf("type choisi : %d\n", n);
                    gtk_widget_destroy(pBoite);	
                    switch (n) {
                        case 2 :
                            tlv=ajouter_texte(2);
                            return(tlv);
                            break;
                        case 3 :
                            tlv=ajouter_png(3);
                            return(tlv);
                            break;
                        case 4 : 
                            tlv=ajouter_jpeg(4);
                            return(tlv);
                            break;
                        case 5 :
                            tlv=ajouter_compound(5);
                            return(tlv);
                            break;
                        case 6 : 
                            tlv=ajouter_dated(6);
                            return(tlv);
                            break;
                    }

                }else {
	                printf("Vous devez rentrer un nombre compris dans les types de TLV");
	                dialog = gtk_message_dialog_new(GTK_WINDOW(pBoite),GTK_DIALOG_MODAL,
					GTK_MESSAGE_ERROR,GTK_BUTTONS_OK,"Vous devez rentrer un nombre compris dans les types de TLV");
     
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
    return tlv;
}



int ajouterMessageCompound(FILE* dazibao,tlv* tlv, int hasLock){ 
    int i;
    GtkTextIter end;
    GtkTextBuffer *buf;
    buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pTextView));
    gtk_text_view_set_wrap_mode((GTK_TEXT_VIEW(pTextView)),GTK_WRAP_WORD);
    gtk_text_view_set_justification((GTK_TEXT_VIEW(pTextView)),GTK_JUSTIFY_CENTER);
    gtk_text_buffer_get_end_iter(buf,&end);
    if(hasLock!=1) {
        if(flock(fileno(dazibao),LOCK_EX)==-1){
            perror("flock ");
            exit(EXIT_FAILURE);
        }
    }
    if(tlv->lenght>16777216) { // la taille n'est pas trop grande pour être stockée dans 3 octets
        printf("Erreur : La taille du message est trop grande\n");
        exit(EXIT_FAILURE);
    }
    if(fwrite(&tlv->type,1,1,dazibao)==0) {
        perror("fwrite :");
        exit(EXIT_FAILURE);
    }
    unsigned char l1=0,l2=0,l3=0;
    l1=ecrireLenght1(tlv->lenght);
    l2=ecrireLenght2(tlv->lenght,l1);
    l3=ecrireLenght3(tlv->lenght,l1,l2);
    if (fwrite(&l1,1,1,dazibao)==0) {
        perror("fwrite :");
        exit(EXIT_FAILURE);
    }
    if (fwrite(&l2,1,1,dazibao)==0) {
        perror("fwrite :");
        exit(EXIT_FAILURE);
    }
    if (fwrite(&l3,1,1,dazibao)==0) {
        perror("fwrite :");
        exit(EXIT_FAILURE);
    }
    for (i = 0; i < tlv->nbTlv; i++) {
        printf("Passage %d :\n",i);
        if(tlv->tlvList[i]->type==2) {
            gtk_text_buffer_insert(buf, &end, g_locale_to_utf8(tlv->tlvList[0]->textOrPath, -1, NULL, NULL, NULL), -1);
            gtk_text_buffer_insert(buf, &end, g_locale_to_utf8("\n", -1, NULL, NULL, NULL), -1);
            ajouterMessageTxt(dazibao,(tlv->tlvList[i]),1);
        }else if(tlv->tlvList[i]->type==3){
            ajouterMessagePng(dazibao,(tlv->tlvList[i]),1);
        }else if(tlv->tlvList[i]->type==4){
            ajouterMessageJpeg(dazibao,(tlv->tlvList[i]),1);
        }else if(tlv->tlvList[i]->type==5){
            ajouterMessageCompound(dazibao,(tlv->tlvList[i]),1);
        }else if(tlv->tlvList[i]->type==6){
            ajouterMessageDated(dazibao,(tlv->tlvList[i]),1);
        }
    }
    if(hasLock!=1) {
        if(flock(fileno(dazibao),LOCK_UN)!=0){
            perror("flock : ");
            exit(EXIT_FAILURE);
        }
    }
    printf("pouet ici ?\n");
    freeTlv(tlv);
    return 0;
}


void ajouter_datedN(){
    ajouter_dated(0);
}

tlv* ajouter_dated(int opt){
    GtkWidget* pBoite;
    GtkWidget* pEntry;
    const gchar* type;
    int len,i,valid;
    char c;
    time_t now;
    tlv* tlvDated=NULL;


    pBoite = gtk_dialog_new_with_buttons("Choix du dated",
				       GTK_WINDOW(pWindow2),
				       GTK_DIALOG_MODAL,
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       NULL);
    pEntry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(pEntry), "Entrez le type du TLV voulu");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(pBoite)->vbox), pEntry, TRUE, FALSE, 0);
    gtk_widget_show_all(GTK_DIALOG(pBoite)->vbox);
    switch (gtk_dialog_run(GTK_DIALOG(pBoite))){
      /* L utilisateur valide */
        case GTK_RESPONSE_OK:
            type = gtk_entry_get_text(GTK_ENTRY(pEntry));
            len = strlen(type);
      
            for(i = 0; i < len; i++){
	            c = type[i];
	
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
            }else{
	            int n=atol(type);
	            if(n > 1 && n <= TLV_MAX){
	                printf("type choisi : %d\n", n);
                    tlv* tlv=NULL;
                    switch (n) {
                        case 2 :
                            tlvDated=newTlv(6);
                            tlvDated->nbTlv=1;
                            if((tlvDated->tlvList=malloc(2*sizeof(struct tlv*)))==NULL){
                                perror("malloc");
                                exit(EXIT_FAILURE);
                            }
                            time(&now);
                            tlvDated->time=now;
                            tlv=ajouter_texte(2);
                            tlvDated->lenght=tlv->lenght+4+4; //+4 pour le tlv, +4 pour le time
                            tlvDated->tlvList[0]=tlv;
                            break;
                        case 3 :
                            tlvDated=newTlv(6);
                            if((tlvDated->tlvList=malloc(2*sizeof(struct tlv*)))==NULL){
                                perror("malloc");
                                exit(EXIT_FAILURE);
                            }
                            tlvDated->tlvList[0]=ajouter_png(3);
                            time(&now);
                            tlvDated->time=now;
                            tlvDated->nbTlv=1;
                            tlvDated->lenght=tlvDated->tlvList[0]->lenght+4+4;
                            break;
                        case 4 : 
                            tlvDated=newTlv(6);
                            if((tlvDated->tlvList=malloc(2*sizeof(struct tlv*)))==NULL){
                                perror("malloc");
                                exit(EXIT_FAILURE);
                            }
                            tlvDated->tlvList[0]=ajouter_jpeg(4);
                            time(&now);
                            tlvDated->time=now;
                            tlvDated->nbTlv=1;
                            tlvDated->lenght=tlvDated->tlvList[0]->lenght+4+4;
                            break;
                        case 5 :
                            tlvDated=newTlv(6);
                            if((tlvDated->tlvList=malloc(2*sizeof(struct tlv*)))==NULL){
                                perror("malloc");
                                exit(EXIT_FAILURE);
                            }
                            tlvDated->tlvList[0]=ajouter_compound(5);
                            printf("On y arrive?\n");
                            time(&now);
                            tlvDated->time=now;
                            tlvDated->nbTlv=1;
                            tlvDated->lenght=tlvDated->tlvList[0]->lenght+4+4;
                            break;
                        case 6 : 
                            tlvDated=newTlv(6);
                            if((tlvDated->tlvList=malloc(2*sizeof(struct tlv*)))==NULL){
                                perror("malloc");
                                exit(EXIT_FAILURE);
                            }
                            tlvDated->tlvList[0]=ajouter_dated(6);
                            time(&now);
                            tlvDated->time=now;
                            tlvDated->nbTlv=1;
                            tlvDated->lenght=tlvDated->tlvList[0]->lenght+4+4;
                            break;
                    }
                    gtk_widget_destroy(pBoite);	

                }else {
	                printf("Vous devez rentrer un nombre compris dans les types de TLV");
	                dialog = gtk_message_dialog_new(GTK_WINDOW(pBoite),GTK_DIALOG_MODAL,
					GTK_MESSAGE_ERROR,GTK_BUTTONS_OK,"Vous devez rentrer un nombre compris dans les types de TLV");
     
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
    if(opt==0){
        num_msg++;
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
        struct stat statBuf;
        if(stat(pathToDazibao,&statBuf)==-1){
            perror("stat");
            exit(EXIT_FAILURE);
        }
        posM[num_msg]=statBuf.st_size;
        FILE* dazibao=NULL;
        if((dazibao=fopen(pathToDazibao,"a+b"))==NULL){
            perror("fopen");
            exit(EXIT_FAILURE);
        }
        ajouterMessageDated(dazibao,tlvDated,0);
        fclose(dazibao);
        return NULL;
    }else{
        return tlvDated;
    }

    printf("ajout dated\n");
    return NULL;
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

 

void suppr(const char *dazibao, int num){
  FILE *f=NULL;
  struct stat sbuf;
  unsigned char type=0,t=0;
  if((f=fopen(dazibao,"r+b"))==NULL){
    perror("fopen :");
    exit(EXIT_FAILURE);
  }
  printf("pos1 : %ld\n",posM[num]);
  fseek(f,posM[num],SEEK_SET);
//  fread(&type,1,1,f);
//  fseek(f,posM[num-1],SEEK_SET);
  fwrite(&t,1,1,f);
  if(num==num_msg){
    if(truncate(dazibao,ftell(f)-1)==-1){
        perror("truncate");
        exit(EXIT_FAILURE);
    }
    fclose(f);
  }else{
    long end=0,pos=ftell(f);
    printf("pos2 : %ld\n",ftell(f));
    unsigned char l1=0,l2=0,l3=0;
    fread(&l1,1,1,f);
    fread(&l2,1,1,f);
    fread(&l3,1,1,f);
    lireLenght(l1,l2,l3);
    fseek(f,0,SEEK_END);
    int i=0;
    for (i = 1; i <= num_msg; i++) {
        printf("pos msg %d : %ld\n",i,posM[i]);
    }
    printf("TEST : %d %d\n",num, num_msg);
    end=ftell(f);
    fseek(f,posM[num+1],SEEK_SET);
    printf("pos3 : %ld\n",ftell(f));
    unsigned char *buf=NULL;
    long tailleBuf=end-ftell(f);
    if((buf=malloc((tailleBuf+1)*sizeof(unsigned char)))==NULL){
        perror("malloc :");
        exit(EXIT_FAILURE);
    }
    fread(buf,tailleBuf,1,f);
    fseek(f,pos,SEEK_SET);
    fwrite(buf,tailleBuf,1,f);
    if(stat(dazibao,&sbuf)==-1){
        perror("stat :");
        exit(EXIT_FAILURE);
    }
    off_t taille = ftell(f);
    if(truncate(dazibao,taille)==-1){
        perror("truncate");
        exit(EXIT_FAILURE);
    }
    printf("fin suppr\n");
    fclose(f);
    free(buf);
  }
}

void compression(){
    long pos=4,tailleParc=4;
    int i;
    unsigned char type=0, l1=0,l2=0,l3=0;
    unsigned int lenght=0;
    unsigned char* contenu=NULL;
    FILE* f=NULL;
    if((f=fopen(pathToDazibao,"r+b"))==NULL){
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    if(flock(fileno(f),LOCK_EX)==-1){
        perror("flock");
        exit(EXIT_FAILURE);
    }
    for(i=1;i<=num_msg;i++) {
        fseek(f,posM[i],SEEK_SET);
        if(fread(&type,1,1,f)==0){
            perror("fread");
            exit(EXIT_FAILURE);
        }
        if(fread(&l1,1,1,f)==0){
            perror("fread");
            exit(EXIT_FAILURE);
        }
        if(fread(&l2,1,1,f)==0){
            perror("fread");
            exit(EXIT_FAILURE);
        }
        if(fread(&l3,1,1,f)==0){
            perror("fread");
            exit(EXIT_FAILURE);
        }
        lenght=lireLenght(l1,l2,l3);
        if((contenu=malloc((lenght+1)*sizeof(unsigned char)))==NULL){
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        if(fread(contenu,lenght,1,f)==-1){
            perror("fread");
            exit(EXIT_FAILURE);
        }
        fseek(f,tailleParc,SEEK_SET);
        if(fwrite(&type,1,1,f)==0){
            perror("fwrite");
            exit(EXIT_FAILURE);
        }
        if(fwrite(&l1,1,1,f)==0){
            perror("fwrite");
            exit(EXIT_FAILURE);
        }
        if(fwrite(&l2,1,1,f)==0){
            perror("fwrite");
            exit(EXIT_FAILURE);
        }
        if(fwrite(&l3,1,1,f)==0){
            perror("fwrite");
            exit(EXIT_FAILURE);
        }
        if(fwrite(contenu,lenght,1,f)==0){
            perror("fwrite");
            exit(EXIT_FAILURE);
        }

        posM[i]=tailleParc;
        tailleParc+=4+lenght;
        free(contenu);
    }
    if(truncate(pathToDazibao,tailleParc)==-1){
        perror("truncate");
        exit(EXIT_FAILURE);
    }

    if(flock(fileno(f),LOCK_UN)==-1){
        perror("flock");
        exit(EXIT_FAILURE);
    }
    fclose(f);
    printf("Fin compression\n");

}
