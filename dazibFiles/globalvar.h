#include <gtk/gtk.h>
#include <gdk/gdk.h>

extern const char* chemin;
extern char* pathToDazibao;
extern struct stat s;
extern const gchar *sText;
extern int num_msg;
extern long *posM;
extern int isPad1_N, isComp_Dated, isDatedNew;
extern GtkWidget *pWindow; GtkWidget *pWindow2;
extern GtkWidget *pVBox;
extern GtkWidget *pVBox2;
extern GtkWidget *pTextView;
extern GtkWidget *scrollbar;   
extern GtkWidget *dialog;

#define TLV_MAX 6
