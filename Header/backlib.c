#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "backlib.h"

int VerificaArgumentos(char *token)
{
  int quantidade = 0;
  while(token != NULL)
  {
    quantidade++;
    token = strtok(NULL, " ");
  }
  return quantidade;
}

int VerificaComando(char *string)
{
  int i, argumentos = 0;
  char *FirstWord;

  FirstWord = strtok(string, " ");
  argumentos = VerificaArgumentos(FirstWord);

  if(strcmp(FirstWord, "users") == 0 || strcmp(FirstWord, "prom") == 0 || strcmp(FirstWord, "reprom") == 0 || strcmp(FirstWord, "items") == 0)
  {
    if(argumentos == 1)
      return 2;
    return 1;
  }

  else if(strcmp(FirstWord, "list") == 0 || strcmp(FirstWord, "kick") == 0 || strcmp(FirstWord, "cancel") == 0)
  {
    if(argumentos == 2)
      return 2;
    return 1;
  }
  return 0;
}

int saveItemsFile(char * filename, struct Item *Items, int Num_Items)
{
  FILE *fp;

  fp = fopen(filename, "w");
  if(fp == NULL)
    return -1;

  for(int i = 0; i < Num_Items; i++)
  {
    fprintf(fp, "%d %s %s %d %d %d\n", Items[i].ID, Items[i].Nome, Items[i].Categoria, Items[i].preco_base, Items[i].preco_agora, Items[i].duracao);
  }

  fclose(fp);
  return 0;
}

int loadItemsFile(char *pathname, struct Item **Items)
{
  FILE *fp;
  int i = 0;
  char buffer[100];

  fp = fopen(pathname, "r");
  if(fp == NULL)
    return -1;
  
  while(fgets(buffer, sizeof(buffer), fp) != NULL)
  {
    *Items = realloc(*Items, (i+1) * sizeof(struct Item));
    sscanf(buffer, "%d%s%s%d%d%d", &(*Items)[i].ID, (*Items)[i].Nome, (*Items)[i].Categoria, &(*Items)[i].preco_base, &(*Items)[i].preco_agora, &(*Items)[i].duracao);
    i++;
  }

  fclose(fp);
  return i;
}

void getFileNames(char *env[], char *filename1, char *filename2)
{
  strcpy(filename1, "Ficheiros/");
  strcpy(filename2, "Ficheiros/");

  if(getenv("FUSERS") != NULL)
    strcat(filename1, getenv("FUSERS"));
  else
    strcat(filename1, "Users");

  if(getenv("FITEMS") != NULL)
    strcat(filename2, getenv("FITEMS"));
  else
    strcat(filename2, "Items");

  strcat(filename1,".txt");
  strcat(filename2,".txt");
}