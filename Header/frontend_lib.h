#ifndef BACKLIB_H
#define BACKLIB_H
#include "frontend_structs.h"

void Abort(char *msg);
bool VerificaComando(char *string, User *user);
void *recebe(void *pdata);
void *heartbeat(void *pdata);
#endif