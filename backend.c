#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <stdbool.h>
#define MAX 100

typedef struct Item
{
  char Nome[20], Categoria[20];
  float preco_base, preco_agora;
  int duracao;
} Item;

typedef struct Utilizador
{
  char Username[20], Password[20];
  Item Vendas;
} Utilizador;

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

int main(int argc, char *argv[])
{
  char comando[MAX], input_username[20], input_password[20];
  int Res, Num_Users = 2;
  bool Match = false;

  Utilizador *Utilizadores = malloc(Num_Users * sizeof(Utilizador));

  strcpy(Utilizadores[0].Username, "Maria");
  strcpy(Utilizadores[0].Password, "Leal");
  strcpy(Utilizadores[1].Username, "Cristina");
  strcpy(Utilizadores[1].Password, "Ferreira");

  if(strcspn(argv[0], "/") != 1) //se não for executado pelo administador
  {
    scanf("%s %s", input_username, input_password);
    for(int i = 0; i < Num_Users; i++)
    {
      if(strcmp(Utilizadores[i].Username, input_username) == 0 && strcmp(Utilizadores[i].Password, input_password) == 0)
      {
        Match = true;
        break;
      }
    }

    if(Match == false)
      printf("Credenciais não existem\n");
    else if(Match == true)
      printf("Bem-vindo/a %s", input_username);

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