#include <stdlib.h>
#include "typedef.h"
#include "globalvar.h"
#include "lecture.h"
#include "fonctionsGenerales.h"

void lectureCompound(FILE* dazibao,unsigned int taille){
    unsigned int tailleParcourue=0;
    unsigned long int tailleLecture=0;
    while(taille>tailleParcourue){
        if((tailleLecture=lectureDazibao(dazibao))!=1) {
            tailleParcourue=tailleParcourue+4+tailleLecture;
        }else{
            tailleParcourue++;
        }
    }
}

int lectureDazibao(FILE* dazibao){
  GtkTextIter end;
  GtkTextBuffer *buf;
  unsigned char type=255, lenght1=0, lenght2=0, lenght3=0;
  unsigned int lenght = 0;
  char *contenu="";
  char *num_msg2=NULL;
 
  GError *error = NULL;
  GdkPixbuf *pix;

  gchar *nomImage = "i.jpg";

  num_msg2=malloc(10*sizeof(unsigned char));
  sprintf(num_msg2, "%d", num_msg);
  printf("%s\n",num_msg2);
  
  fread(&type,1,1,dazibao);
  buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pTextView));
  gtk_text_view_set_wrap_mode((GTK_TEXT_VIEW(pTextView)),GTK_WRAP_WORD);
  gtk_text_view_set_justification((GTK_TEXT_VIEW(pTextView)),GTK_JUSTIFY_CENTER);
  gtk_text_buffer_get_end_iter(buf,&end);
  printf("need to know : type =%d, ispad1_n=%d isComp_D=%d\n",type,isPad1_N,isComp_Dated);
  if((type!=0 && type!=1) && isComp_Dated==0){
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
    posM[num_msg]=ftell(dazibao)-1;
  }
  fread(&lenght1,1,1,dazibao);
  fread(&lenght2,1,1,dazibao);
  fread(&lenght3,1,1,dazibao);
  lenght=lireLenght(lenght1,lenght2,lenght3);
  if((contenu=malloc((lenght+2)*sizeof(unsigned char)))==NULL && lenght!=0){
    perror("Malloc : ");
    exit(EXIT_FAILURE);
  }
  if (type==5) {
  printf("C'est un compound\n");
    isComp_Dated=1;
    lectureCompound(dazibao,lenght);
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
//    fread(&secondes,4,1,dazibao);
    fread(&l1,1,1,dazibao);
    fread(&l2,1,1,dazibao);
    fread(&l3,1,1,dazibao);
    fread(&l4,1,1,dazibao);
    secondes=256*256*256*l1+256*256*l2+256*l3+l4;
    printf("secondes : %ld\n",secondes);
    printf("%s",ctime(&secondes));
    buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pTextView));
    gtk_text_view_set_wrap_mode((GTK_TEXT_VIEW(pTextView)),GTK_WRAP_WORD);
    gtk_text_view_set_justification((GTK_TEXT_VIEW(pTextView)),GTK_JUSTIFY_CENTER);
    gtk_text_buffer_get_end_iter(buf,&end);
    gtk_text_buffer_insert(buf, &end, g_locale_to_utf8(ctime(&secondes), -1, NULL, NULL, NULL), -1);
    lectureDazibao(dazibao);
    free(contenu);
    isComp_Dated=0;
    return lenght;
  }
  fread(contenu,lenght,1,dazibao);
  if (type==1) {
    isPad1_N=1;
    printf("C'est un padN\n");
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
    buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pTextView));
    gtk_text_view_set_wrap_mode((GTK_TEXT_VIEW(pTextView)),GTK_WRAP_WORD);
    gtk_text_view_set_justification((GTK_TEXT_VIEW(pTextView)),GTK_JUSTIFY_CENTER);
    gtk_text_buffer_get_end_iter(buf,&end);
    gtk_text_buffer_insert(buf, &end, g_locale_to_utf8("TLV de type inconnu.\n", -1, NULL, NULL, NULL), -1);
    free(contenu);
    return lenght;
  }
  
}
