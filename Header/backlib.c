#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "backlib.h"

int VerificaComandoAdmin(char *string, User *user)
{
  int quantidade = 0;
  char *token;
  char **final;
  bool valid = false;
  token = strtok(string, " ");

  final = malloc(quantidade * sizeof(char*));

  while(token != NULL)
  {
    quantidade++;
    final = realloc(final, quantidade * sizeof(char*));
    final[quantidade-1] = token;
    token = strtok(NULL, " ");
  }

  if(strcmp(final[0], "users") == 0 || strcmp(final[0], "prom") == 0 || strcmp(final[0], "reprom") == 0 || strcmp(final[0], "items") == 0 || strcmp(final[0], "list") == 0 || strcmp(final[0], "close") == 0 || strcmp(final[0], "prom2") == 0)
  {
    if(quantidade == 1)
    {
      strcpy(user->input[0], final[0]);
      valid = true;
    }
  }

  else if(strcmp(final[0], "kick") == 0 || strcmp(final[0], "cancel") == 0)
  {
    if(quantidade == 2)
    {
      strcpy(user->input[0], final[0]);
      strcpy(user->input[1], final[1]);
      valid = true;
    }
  }

  free(token);
  free(final);
  return valid;
}

int saveItemsFile(char * filename, Item *Items, int Num_Items)
{
  FILE *fp;

  fp = fopen(filename, "w");
  if(fp == NULL)
    return -1;

  for(int i = 0; i < Num_Items; i++)
  {
    fprintf(fp, "%d %s %s %d %d %d %s %s\n", Items[i].ID, Items[i].Nome, Items[i].Categoria, Items[i].preco_base, Items[i].preco_agora, Items[i].duracao, Items[i].seller, Items[i].highestbidder);
  }

  fclose(fp);
  return 0;
}

int loadItemsFile(char *pathname, Item **Items, int *lastID)
{
  FILE *fp;
  int i = 0;
  char buffer[100];

  fp = fopen(pathname, "r");
  if(fp == NULL)
    return -1;
  
  while(fgets(buffer, sizeof(buffer), fp) != NULL)
  {
    *Items = realloc(*Items, (i+1) * sizeof(Item));
    sscanf(buffer, "%d%s%s%d%d%d%s%s", &(*Items)[i].ID, (*Items)[i].Nome, (*Items)[i].Categoria, &(*Items)[i].preco_base, &(*Items)[i].preco_agora, &(*Items)[i].duracao, (*Items)[i].seller, (*Items)[i].highestbidder);
    (*Items)[i].percentagem = 0;
    (*Items)[i].duracaoDiscount = 0;
    (*Items)[i].activeDiscount = false;
    *lastID = (*Items)[i].ID;
    i++;
  }

  fclose(fp);
  return i;
}

void getFileNames(char *env[], char filename[][50])
{
  char temp[20];

  strcpy(temp, "Ficheiros/");

  if(getenv("FUSERS") != NULL)
    strcat(temp, getenv("FUSERS"));
  else
    strcat(temp, "Users");
  
  strcat(temp,".txt");
  strcpy(filename[0], temp);

  strcpy(temp, "Ficheiros/");

  if(getenv("FITEMS") != NULL)
    strcat(temp, getenv("FITEMS"));
  else
    strcat(temp, "Items");

  strcat(temp,".txt");
  strcpy(filename[1], temp);

  strcpy(temp, "Ficheiros/");

  if(getenv("FPROMOTERS") != NULL)
    strcat(temp, getenv("FPROMOTERS"));
  else
    strcat(temp, "Promoters");

  strcat(temp,".txt");
  strcpy(filename[2], temp);
}

void Abort(char *msg)
{
  printf("%s", msg);
  exit(1);
}

bool VerificaComando(char *string, User *user)
{
  int quantidade = 0;
  char *token;
  char **final;
  bool valid = false;
  token = strtok(string, " ");

  final = malloc(quantidade * sizeof(char*));

  while(token != NULL)
  {
    quantidade++;
    final = realloc(final, quantidade * sizeof(char*));
    final[quantidade-1] = token;
    token = strtok(NULL, " ");
  }

  if(strcmp(final[0], "list") == 0 || strcmp(final[0], "cash") == 0 || strcmp(final[0], "time") == 0 || strcmp(final[0], "exit") == 0)
  {
    if(quantidade == 1)
    {
      strcpy(user->input[0], final[0]);
      valid = true;
    }
  }

  else if(strcmp(final[0], "licat") == 0 || strcmp(final[0], "lisel") == 0 || strcmp(final[0], "add") == 0 || strcmp(final[0], "lival") == 0 || strcmp(final[0], "litime") == 0)
  {
    if(quantidade == 2)
    {
      strcpy(user->input[0], final[0]);
      strcpy(user->input[1], final[1]);
      valid = true;
    }
  }

  else if(strcmp(final[0], "buy") == 0)
  {
    if(quantidade == 3)
    {
      strcpy(user->input[0], final[0]);
      strcpy(user->input[1], final[1]);
      strcpy(user->input[2], final[2]);
      valid = true;
    }
  }

  else if(strcmp(final[0], "sell") == 0)
  {
    if(quantidade == 6)
    {
      strcpy(user->input[0], final[0]);
      strcpy(user->input[1], final[1]);
      strcpy(user->input[2], final[2]);
      strcpy(user->input[3], final[3]);
      strcpy(user->input[4], final[4]);
      strcpy(user->input[5], final[5]);
      valid = true;
    }
  }

  free(token);
  free(final);

  return valid;
}