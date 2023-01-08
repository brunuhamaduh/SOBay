#ifndef BACKEND_STRUCTS_H
#define BACKEND_STRUCTS_H
#include "shared_structs.h"

typedef struct
{
  int nitems, continua, bf, maxheartbeat;
  int *index, *tempo, *cliente, *nclientes, *heartbeat, *prom, *nprom;
  char userfilename[50], promfilename[50], itemfilename[50];
  char **nomecliente, **nomeprom;
  bool *available;
  Item *Items;
  User user;
  pthread_mutex_t *wait;
} USER_DATA;

#endif