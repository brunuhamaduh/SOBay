#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <stdbool.h>
#include "Lib/users_lib.h"
#define MAX 100

typedef struct Item
{
  char Nome[20], Categoria[20];
  float preco_base, preco_agora;
  int ID, duracao;
} Item;

typedef struct Utilizador
{
  char Username[20], Password[20];
  int saldo;
} Utilizador;

Utilizador *Utilizadores;
int Num_Users;

int saveUsersFile(char * filename)
{
  FILE *fp;

  fp = fopen(filename, "w");
  if(fp == NULL)
    return -1;

  for(int i = 0; i <  Num_Users; i++)
  {
    fprintf(fp, "%s %s %d\n", Utilizadores[i].Username, Utilizadores[i].Password, Utilizadores[i].saldo);
  }

  fclose(fp);
  return 0;
}

int loadUsersFile(char *pathname)
{
  FILE *fp;
  int Count, i = 0;
  char buffer[100];

  fp = fopen(pathname, "r");
  if(fp == NULL)
    return -1;

  while(fgets(buffer, sizeof(buffer), fp) != NULL)
  {
    Utilizadores = realloc(Utilizadores, (i+1) * sizeof(Utilizador));
    sscanf(buffer, "%s%s%d", Utilizadores[i].Username, Utilizadores[i].Password, &Utilizadores[i].saldo);
    i++;
  }

  fclose(fp);
  return i;
}

int isUserValid(char * username, char * password)
{
  for(int i = 0; i < Num_Users; i++)
  {
    if(strcmp(Utilizadores[i].Username, username) == 0 && strcmp(Utilizadores[i].Password, password) == 0)
      return 1;
  }
  return 0;
}

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

  if(strcmp(FirstWord, "users") == 0 || strcmp(FirstWord, "prom") == 0 || strcmp(FirstWord, "reprom") == 0)
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

void getFileName(char *env[], char *filename)
{
  strcpy(filename, "Ficheiros/");
  if(getenv("FUSERS") != NULL)
    strcat(filename, getenv("FUSERS"));
  else
    strcat(filename, "Users");
  strcat(filename,".txt");
}

int main(int argc, char *argv[], char *env[])
{
  char comando[MAX], input_username[20], input_password[20], filename[30];
  int Res;
  getFileName(env, filename);
  Num_Users = loadUsersFile(filename);

  if(strcspn(argv[0], "/") != 1) //Se não for executado pelo administrador
  {
    scanf("%s %s", input_username, input_password);
    printf("User Exists = %d", isUserValid(input_username, input_password));
    free(Utilizadores);
    return 0;
  }
    
  printf("\nComandos disponiveis\n");
  printf("users\n");
  printf("list\n"); //ONLY ONE (id, nome item, categoria, preço atual, preço compre já, vendedor, licitador mais elevado ou menos elevado)
  printf("kick <username>\n");
  printf("prom\n");
  printf("reprom\n");
  printf("cancel <nome-do-executável-do-promotor>\n");
  printf("close\n");

  do
  {
    printf("Comando: ");
    fgets(comando, MAX, stdin);
    comando[strcspn(comando, "\n")] = '\0'; //retira a newline do fgets;
    
    Res = VerificaComando(comando);
    if(Res == 0 && strcmp(comando, "close") != 0)
      printf("Comando inválido\n");
    else if(Res == 1)
      printf("Número de argumentos incorrecto\n");
    else if(Res == 2)
      printf("Sucesso\n");
    
  } while(strcmp(comando, "close") != 0);

  return 0;
}