#ifndef BACKLIB_H
#define BACKLIB_H
#include "structs.h"

int VerificaComandoAdmin(char *string, User *user);
int saveItemsFile(char *filename, Item *Items, int Num_Items);
int loadItemsFile(char *pathname, Item **Items, int *lastID);
void getFileNames(char *env[], char filename[][50]);
void Abort(char *msg);
bool VerificaComando(char *string, User *user);
#endif