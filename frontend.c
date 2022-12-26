#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <pthread.h>
#include "Header/sharedlib.h"

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

typedef struct
{
  int continua;
  int caixa;
  int bf;
  pthread_mutex_t *wait;
} USER_DATA;

void *recebe(void *pdata)
{
  USER_DATA *data = pdata;
  int feedback;
  char comando[20];

  do
  {
    read(data->caixa, comando, sizeof(comando));
    read(data->caixa, &feedback, sizeof(feedback));
    if(strcmp(comando, "add") == 0 || strcmp(comando, "cash") == 0)
    {
      printf("[SALDO] = %d\nComando: ", feedback);
    }
    fflush(stdout);
  } while (data->continua);
  pthread_exit(NULL);
}

int main(int argc, char* argv[])
{
  int bf, caixa, feedback;
  char comando[100], NomeCaixa[10];
  User user;
  USER_DATA data;
  pthread_mutex_t wait;
  pthread_t thread;

  if(argc != 3)
    Abort("[ERRO] Sintaxe Errada\nSintaxe Correta: ./frontend <USERNAME> <PASSWORD>\n");

  if(access("BF", F_OK) != 0)
  {
	  Abort("[ERRO] Servidor fechado...\n");
  }

  user.pid = getpid();
  sprintf(NomeCaixa, "CLI%d", user.pid);
  mkfifo(NomeCaixa, 0666);

  bf = open("BF", O_WRONLY);

  strcpy(user.Username, argv[1]);
  strcpy(user.Password, argv[2]);
  strcpy(user.input[0], "login");
  write(bf, &user, sizeof(User));
  
  caixa = open(NomeCaixa, O_RDWR);
  read(caixa, &feedback, sizeof(feedback));

  if(feedback != 1)
  {
    close(caixa);
    close(bf);
    unlink(NomeCaixa);
    Abort("Utilizador nao existe\n");
  }

  printf("Welcome '%s'\n", user.Username);

  pthread_mutex_init(&wait, NULL);
  
  data.continua = 1;
  data.caixa = caixa;
  data.bf = bf;
  data.wait = &wait;

  pthread_create(&thread, NULL, recebe, &data);

  printf("Comando: ");
  fflush(stdout);
  do
  {

    fgets(comando, 100, stdin);
    comando[strcspn(comando, "\n")] = '\0'; //tira "\n" do input;
    if(VerificaComando(comando, &user))
      write(bf, &user, sizeof(user));
  } while(strcmp(comando, "exit") != 0);

  data.continua = 0;
  strcpy(user.input[0], "end");
  write(caixa, &user, sizeof(User));
  pthread_join(thread, NULL);

  close(caixa);
  close(bf);
  unlink(NomeCaixa);
  return 0;
}
