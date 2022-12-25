#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "Header/sharedlib.h"

#define MAX 100

void Abort(char *msg)
{
  printf("%s", msg);
  exit(1);
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

void VerificaComando(char *string)
{
  int i, argumentos, Res = 0;
  char *FirstWord;

  FirstWord = strtok(string, " ");
  argumentos = VerificaArgumentos(FirstWord);

  if(strcmp(FirstWord, "list") == 0 || strcmp(FirstWord, "cash") == 0 || strcmp(FirstWord, "time") == 0)
  {
    if(argumentos == 1)
      Res = 2;
    else
      Res = 1;
  }

  else if(strcmp(FirstWord, "licat") == 0 || strcmp(FirstWord, "lisel") == 0 || strcmp(FirstWord, "add") == 0 || strcmp(FirstWord, "lival") == 0 || strcmp(FirstWord, "litime") == 0)
  {
    if(argumentos == 2)
      Res = 2;
    else
      Res = 1;
  }

  else if(strcmp(FirstWord, "buy") == 0)
  {
    if(argumentos == 3)
      Res = 2;
    else
      Res = 1;
  }

  else if(strcmp(FirstWord, "sell") == 0)
  {
    if(argumentos == 6)
      Res = 2;
    else
      Res = 1;
  }

  if(Res == 0 && strcmp(string, "exit") != 0)
    printf("Comando inválido\n");
  else if(Res == 1)
    printf("Número de argumentos incorrecto\n");
  else if(Res == 2)
    printf("Sucesso\n");
}

int main(int argc, char* argv[])
{
  int Res, estado, send[2], child, bf;
  char comando[MAX];
  User user;

  if(argc != 3)
    Abort("[ERRO] Sintaxe Errada\nSintaxe Correta: ./frontend <USERNAME> <PASSWORD>\n");

  if(access("BF", F_OK) != 0)
  {
	  Abort("[ERRO] Servidor fechado...\n");
  }

  bf = open("BF", O_RDWR);

  strcpy(user.Username, argv[1]);
  strcpy(user.Password, argv[2]);
  user.pid = getpid();
  user.intent = 1;
  write(bf, &user, sizeof(User));

  printf("Comandos disponiveis\n");
  printf("sell <nome> <categoria> <preco-base> <preco-compre-ja> <duração>\n");
  printf("list\n");
  printf("licat <nome-categoria>\n");
  printf("lisel <username do vendedor>\n");
  printf("lival <preço-máximo>\n");
  printf("litime <hora-em-segundos>\n");
  printf("time\n");
  printf("buy <id> <valor>\n");
  printf("cash\n");
  printf("add <valor>\n");
  printf("exit\n");

  do
  {
    printf("Comando: ");
    fgets(comando, MAX, stdin);
    comando[strcspn(comando, "\n")] = '\0'; //tira "\n" do input;
    VerificaComando(comando);
  } while(strcmp(comando, "exit") != 0);

  user.intent = 0;
  write(bf, &user, sizeof(User));
  return 0;
}
