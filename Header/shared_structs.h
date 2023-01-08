#ifndef SHARED_STRUCTS_H
#define SHARED_STRUCTS_H
#include <pthread.h>

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

#endif