#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
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

bool VerificaComando(char *string)
{
  int i, argumentos = 0;
  char *FirstWord;

  FirstWord = strtok(string, " ");
  argumentos = VerificaArgumentos(FirstWord);

  if(strcmp(FirstWord, "list") == 0 || strcmp(FirstWord, "cash") == 0 || strcmp(FirstWord, "time") == 0)
  {
    if(argumentos == 1)
      return true;
  }

  else if(strcmp(FirstWord, "licat") == 0 || strcmp(FirstWord, "lisel") == 0 || strcmp(FirstWord, "add") == 0 || strcmp(FirstWord, "lival") == 0 || strcmp(FirstWord, "litime") == 0)
  {
    if(argumentos == 2)
      return true;
  }

  else if(strcmp(FirstWord, "buy") == 0)
  {
    if(argumentos == 3)
      return true;
  }

  else if(strcmp(FirstWord, "sell") == 0)
  {
    if(argumentos == 6)
      return true;
  }

  return false;
}

int main(int argc, char* argv[])
{
  char comando[MAX];

  if(argc != 3)
    Abort("\nSintaxe Errada\n./frontend <USERNAME> <PASSWORD>\n");

  printf("\nComandos disponiveis\n");
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
    comando[strcspn(comando, "\n")] = '\0'; //retira a newline do fgets;
    
    if(VerificaComando(comando) == true)
      printf("[RESULT] Nice\n");
    else if(strcmp(comando, "exit") != 0)
      printf("[RESULT] Not valid\n");
    
  } while(strcmp(comando, "exit") != 0);
  return 0;
}
