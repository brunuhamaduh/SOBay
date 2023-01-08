#ifndef FRONTEND_STRUCTS_H
#define FRONTEND_STRUCTS_H
#include "shared_structs.h"

typedef struct
{
  User user;
  int continua, caixa, bf, forceExit, nheartbeat;
  pthread_mutex_t *wait;
} USER_DATA;

#endif