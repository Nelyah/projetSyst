#include <gtk/gtk.h>
#include <gdk/gdk.h>

extern const char* chemin; // Chemin du dazibao
extern char* pathToDazibao; // Chemin du dazibao
extern struct stat s; // structure stat utilisée dans plusieurs fonctions
extern const gchar *sText;
extern int num_msg; // Nombre de messages non Pad1_N présent dans le dazibao
extern long *posM; // Tableau contenu les emplacements de chaque message
extern int isPad1_N, isComp_Dated; // Flags pour savoir si le dernier tlv lu était 
                                    // pad1_N ou compt ou dated
// Différentes variables globales nécessaires à gtk
extern GtkWidget *pWindow; GtkWidget *pWindow2; 
extern GtkWidget *pVBox;
extern GtkWidget *pVBox2;
extern GtkWidget *pTextView;
extern GtkWidget *scrollbar;   
extern GtkWidget *dialog;

// Type de tlv max : A incrémenter si l'on rajoute de nouveaux types
#define TLV_MAX 6
