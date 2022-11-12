#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void Abort(char *msg)
{
  printf("%s", msg);
  exit(1);
}

int VerificaArgumentos(const char comando[20])
{
  int quantidade = 0;
  for(int i = 0; i < strlen(comando); i++)
  {
    if(comando[i] == ' ')
      quantidade++;
  }

  return quantidade;
}

bool VerificaComando(const char string[100])
{
  int i, argumentos, contador = 0;
  char temp[20];
  for(i = 0; i < strlen(string); i++)
  {
    if(string[i] == ' ')
      break;
    temp[i] = string[i];
  }
  temp[i] = '\0';

  argumentos = VerificaArgumentos(temp);
  printf("\nTEMP %s\n", temp);
  printf("\nARGUMENTOS %d\n", argumentos);
  /*
  if(strcmp(temp, "sell") == 0)
  {
    if(argumentos == 5)
      return true;
  }
  else if(strcmp(temp, "list") == 0)
  {
    printf("teste");
    if(argumentos == 0)
      return true;
  }
  else if(strcmp(temp, "licat") == 0)
  {
    if(argumentos == 1)
      return true;
  }
  else if(strcmp(temp, "lisel") == 0)
  {
    if(argumentos == 1)
      return true;
  }
  else if(strcmp(temp, "lival") == 0)
  {
    if(argumentos == 1)
      return true;
  }
  else if(strcmp(temp, "litime") == 0)
  {
    if(argumentos == 1)
      return true;
  }
  else if(strcmp(temp, "time") == 0)
  {
    if(argumentos == 0)
      return true;
  }
  else if(strcmp(temp, "buy") == 0)
  {
    if(argumentos == 2)
      return true;
  }
  else if(strcmp(temp, "cash") == 0)
  {
    if(argumentos == 0)
      return true;
  }
  else if(strcmp(temp, "add") == 0)
  {
    if(argumentos == 1)
      return true;
  }
*/
  return false;
}

int main(int argc, char* argv[])
{
  char comando[100];

  if(argc != 3)
  {
    Abort("\nSintaxe Errada\n./frontend <USERNAME> <PASSWORD>\n");
  }

  do
  {
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
    printf("Comando: ");
    
    fgets(comando, 100, stdin);
    if(VerificaComando(comando) == true)
      printf("[RESULT] Nice \n");
    else
      printf("[RESULT] Not valid\n");
    

  } while(strcmp(comando, "exit") != 0);
  return 0;
}
