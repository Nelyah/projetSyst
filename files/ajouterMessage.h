#include "typedef.h"
#include "fonctionsGenerales.h"


int ajouterMessageCompound(FILE* dazibao,tlv* tlv,int hasLock);
int ajouterMessageJpeg(FILE* dazibao,tlv* tlv,int hasLock);
int ajouterMessagePng(FILE* dazibao,tlv* tlv,int hasLock);
int ajouterMessageTxt(FILE* dazibao,tlv* tlv,int hasLock);
int ajouterMessageDated(FILE* dazibao,tlv* tlv,int hasLock);
void ajouter_compoundN();
tlv* ajouter_compound(int opt);
void ajouter_datedN();
tlv* ajouter_dated(int opt);
void ajouter_jpegN();
tlv* ajouter_jpeg(int opt);
void ajouter_pngN();
tlv* ajouter_png(int opt);
void ajouter_texteN();
tlv* ajouter_texte(int opt);
tlv* choisirTlv();
