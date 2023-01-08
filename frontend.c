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
#include "Header/frontend_lib.h"

int main(int argc, char* argv[])
{
  int bf, caixa, feedback;
  char comando[100], NomeCaixa[10];
  User user;
  USER_DATA data;
  pthread_mutex_t wait;
  pthread_t thread[2];

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
    if(feedback == 2)
      Abort("Utilizador ja esta online\n");
    else
      Abort("Utilizador nao existe\n");
  }

  printf("Welcome '%s'\n", user.Username);

  pthread_mutex_init(&wait, NULL);
  
  data.continua = 1;
  data.caixa = caixa;
  data.bf = bf;
  data.wait = &wait;
  data.forceExit = 1;
  data.user = user;
  
  if(getenv("HEARTBEAT") != NULL)
    data.nheartbeat = atoi(getenv("HEARTBEAT"));
  else
    data.nheartbeat = 10;

  pthread_create(&thread[0], NULL, recebe, &data);
  pthread_create(&thread[1], NULL, heartbeat, &data);

  do
  {
    fgets(comando, 100, stdin);
    pthread_mutex_lock(&wait);
    if(strcmp(comando, "\n") == 0)
      continue;
    comando[strcspn(comando, "\n")] = '\0'; //tira "\n" do input;
    if(VerificaComando(comando, &user))
      write(bf, &user, sizeof(user));
    pthread_mutex_unlock(&wait);
  } while(strcmp(comando, "exit") != 0 && data.forceExit != 0);

  data.continua = 0;
  pthread_join(thread[0], NULL);
  pthread_join(thread[1], NULL);
  pthread_mutex_destroy(&wait);

  close(caixa);
  close(bf);
  unlink(NomeCaixa);
  return 0;
}
