#ifndef BACKEND_LIB_H
#define BACKEND_LIB_H
#include "backend_structs.h"

bool VerificaComando(char *string, User *user);
int saveItemsFile(char *filename, Item *Items, int Num_Items);
int loadItemsFile(char *pathname, Item **Items, int *lastID);
void getFileNames(char *env[], char filename[][50]);
void Abort(char *msg);
void *trata_comandos(void *pdata);
void *trata_segundos(void *pdata);
void *trata_promotor(void *pdata);
#endif