#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
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

  for(int i = 0; i < Num_Users; i++)
  {
    fprintf(fp, "%s %s %d\n", Utilizadores[i].Username, Utilizadores[i].Password, Utilizadores[i].saldo);
  }

  fclose(fp);
  return 0;
}

int loadUsersFile(char *pathname)
{
  FILE *fp;
  int i = 0;
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

int getUserBalance(char * username)
{
  for(int i = 0; i < Num_Users; i++)
  {
    if(strcmp(Utilizadores[i].Username, username) == 0)
      return Utilizadores[i].saldo;
  }
  return -1; //caso não exista utilizador com o username dado
}

int updateUserBalance(char * username, int value)
{
  for(int i = 0; i < Num_Users; i++)
  {
    if(strcmp(Utilizadores[i].Username, username) == 0)
    {
      Utilizadores[i].saldo = value;
      return 0;
    }
  }
  return -1; //caso não exista utilizador com o username dado
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

int saveItemsFile(char * filename, Item *Items, int Num_Items) //FALTA TESTAR ISTO
{
  FILE *fp;

  fp = fopen(filename, "w");
  if(fp == NULL)
    return -1;

  for(int i = 0; i < Num_Items; i++)
  {
    fprintf(fp, "%d %s %s %.2f %.2f %d\n", Items[i].ID, Items[i].Nome, Items[i].Categoria, Items[i].preco_base, Items[i].preco_agora, Items[i].duracao);
  }

  fclose(fp);
  return 0;
}

int loadItemsFile(char *pathname, Item **Items)
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
    sscanf(buffer, "%d%s%s%f%f%d", &(*Items)[i].ID, (*Items)[i].Nome, (*Items)[i].Categoria, &(*Items)[i].preco_base, &(*Items)[i].preco_agora, &(*Items)[i].duracao);
    i++;
  }

  fclose(fp);
  return i;
}

void getUserFileName(char *env[], char *filename)
{
  strcpy(filename, "Ficheiros/");
  if(getenv("FUSERS") != NULL)
    strcat(filename, getenv("FUSERS"));
  else
    strcat(filename, "Users");
  strcat(filename,".txt");
}

void getItemFileName(char *env[], char *filename)
{
  strcpy(filename, "Ficheiros/");
  if(getenv("FITEMS") != NULL)
    strcat(filename, getenv("FITEMS"));
  else
    strcat(filename, "Items");
  strcat(filename,".txt");
}

int main(int argc, char *argv[], char *env[])
{
  char comando[MAX];
  char input_username[20], input_password[20];
  char Userfilename[30], Itemfilename[30];
  int Res, Num_Items;
  Item *Items = malloc(0);

  if(strcspn(argv[0], "/") != 1) //Verifica se foi executado diretamente ou não
  {
    getUserFileName(env, Userfilename);
    getItemFileName(env, Itemfilename);

    Num_Users = loadUsersFile(Userfilename);
    Num_Items = loadItemsFile(Itemfilename, &Items);

    scanf("%s %s", input_username, input_password);
    printf("User Exists = %d\n", isUserValid(input_username, input_password));
    free(Utilizadores);
    free(Items);
    return 0;
  }

  //PARTE DO ADMINISTRADOR
  //SÓ ENTRA AQUI QUANDO FOR EXECUTADO DIRETAMENTE
  printf("Comandos disponiveis\n");
  printf("------------------------\n");
  printf("users\n");
  printf("list\n"); //ONLY ONE (id, nome item, categoria, preço atual, preço compre já, vendedor, licitador mais elevado ou menos elevado)
  printf("kick <username>\n");
  printf("prom\n");
  printf("reprom\n");
  printf("cancel <nome-do-executavel-do-promotor>\n");
  printf("close\n");
  printf("------------------------\n");
  
  do
  {
    fgets(comando, MAX, stdin);
    comando[strcspn(comando, "\n")] = '\0'; //retira a newline do fgets;
    
    Res = VerificaComando(comando);
    if(Res == 0 && strcmp(comando, "close") != 0)
      printf("Comando inválido\n");
    else if(Res == 1)
      printf("Número de argumentos incorrecto\n");
    else if(Res == 2)
    {
      if(strcmp(comando, "reprom") == 0)
      {
        if(fork() == 0)
        {
          int estado, PID_Promotor, prom[2];
          char buffer[100];
          union sigval stop;

          pipe(prom);
          PID_Promotor = fork();

          if(PID_Promotor == 0)
          {
            close(prom[0]); //close read
            close(1);
            dup(prom[1]);
            close(prom[1]);
            execl("Promotor/promotor_oficial", "Promotor/promotor_oficial", NULL);
          }
          close(prom[1]); //close write
          while(kill(PID_Promotor, 0) == 0) //enquanto estiver a correr
          {
            int nbytes = read(prom[0], buffer, sizeof(buffer));
            buffer[nbytes] = '\0';
            printf("%s", buffer);
            sigqueue(PID_Promotor, SIGUSR1, stop);
          }
          close(prom[0]);
          exit(0);
        }
      }
    }
  } while(strcmp(comando, "close") != 0);

  return 0;
}