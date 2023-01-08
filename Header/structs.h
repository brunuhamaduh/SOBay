#ifndef STRUCTS_H
#define STRUCTS_H

typedef struct
{
  char Nome[20], Categoria[20], seller[20], highestbidder[20];
  int preco_base, preco_agora, ID, duracao, duracaoDiscount, percentagem;
  bool activeDiscount;
} Item;

typedef struct 
{
  int pid, ninput;
  char Username[20], Password[20];
  char input[6][20];
} User;

typedef struct
{
  char Categoria[50];
  int percentagem;
  int duracao;
} Discount;

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

typedef struct
{
  User user;
  int continua, caixa, bf, forceExit, nheartbeat;
  pthread_mutex_t *wait;
} USER_DATA2;

#endif