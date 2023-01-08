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
#include "Header/backlib.h"

typedef struct
{
  User user;
  int continua, caixa, bf, forceExit, nheartbeat;
  pthread_mutex_t *wait;
} USER_DATA;

void *recebe(void *pdata)
{
  USER_DATA *data = pdata;
  Item *item = malloc(0);
  Discount discount;
  int feedback, n;
  char comando[20], comando2[20], str[20], last[20];
  do
  {
    read(data->caixa, comando, sizeof(comando));
    pthread_mutex_lock(data->wait);
    if(strcmp(comando, "add") == 0 || strcmp(comando, "cash") == 0)
    {
      read(data->caixa, &feedback, sizeof(feedback));
      printf("[SALDO] = %d\n", feedback);
    }
    else if(strcmp(comando, "time") == 0)
    {
      read(data->caixa, &feedback, sizeof(feedback));
      printf("[TIME] = %d\n", feedback);
    }
    else if(strcmp(comando, "sell") == 0)
    {
      read(data->caixa, &feedback, sizeof(feedback));
      printf("[ID do item] = %d\n", feedback);
    }
    else if(strcmp(comando, "list") == 0 || strcmp(comando, "licat") == 0 || strcmp(comando, "lisel") == 0 || strcmp(comando, "lival") == 0 || strcmp(comando, "litime") == 0)
    {
      read(data->caixa, &feedback, sizeof(feedback));
      read(data->caixa, comando2, sizeof(comando2));
      if(feedback != 0)
      {
        printf("[Items a serem vendidos");
        fflush(stdout);
        if(strcmp(comando, "licat") == 0)
          printf(" na categoria %s]\n", comando2);
        else if(strcmp(comando, "lisel") == 0)
          printf(" por %s]\n", comando2);
        else if(strcmp(comando, "list") == 0)
          printf("]\n");
        else if(strcmp(comando, "lival") == 0)
          printf(" até %s euros]\n", comando2);
        else if(strcmp(comando, "litime") == 0)
          printf(" até %s segundos]\n", comando2);

        printf("|ID|NOME PRODUTO|CATEGORIA|PRECOB|PRECOA|SEC|SELLER|HBidder|\n");
        item = realloc(item, sizeof(Item) * feedback);
        read(data->caixa, item, sizeof(Item) * feedback);
        for(int i = 0; i < feedback; i++)
        {
          printf("|%-2.2d|%-12.12s|%-9.9s|%-6.6d|%-6.6d|%-3.3d|%-6.6s|%-7.7s|\n", item[i].ID, item[i].Nome, item[i].Categoria, item[i].preco_base, item[i].preco_agora, item[i].duracao, item[i].seller, item[i].highestbidder);
        }
      }
      else
      {
        if(strcmp(comando, "licat") == 0)
          printf("Nao existem produtos a serem vendidos com esta categoria\n");
        else if(strcmp(comando, "lisel") == 0)
          printf("Nao existem produtos a serem vendidos por este utilizador\n");
        else if(strcmp(comando, "list") == 0)
          printf("Nao existem produtos a serem vendidos\n");
        else if(strcmp(comando, "lival") == 0)
          printf("Nao existem produtos a serem vendidos abaixo deste preco\n");
        else if(strcmp(comando, "litime") == 0)
          printf("Nao existem produtos a serem vendidos abaixo destes segundos\n");
      }
    }
    else if(strcmp(comando, "buy") == 0)
    {
      read(data->caixa, comando2, sizeof(comando2));

      if(strcmp(comando2, "Own Buy") == 0)
        printf("[AVISO] Nao podes comprar uma coisa que estas a vender\n");
      else if(strcmp(comando2, "Success") == 0)
        printf("[SUCESSO] Es o bidder mais alto\n");
      else if(strcmp(comando2, "Success Bought") == 0)
        printf("[SUCESSO] Adquirido com sucesso\n");
      else if(strcmp(comando2, "Low price") == 0)
        printf("[AVISO] Tens que aumentar o preco\n");
      else if(strcmp(comando2, "Not Found") == 0)
        printf("[AVISO] ID Invalido\n");
      else if(strcmp(comando2, "Broke") == 0)
        printf("[AVISO] Nao tem saldo disponivel");
    }
    else if(strcmp(comando, "newitem") == 0)
    {
      item = realloc(item, sizeof(Item));
      n = read(data->caixa, item, sizeof(Item));
      if(n == sizeof(Item))
      {
        printf("[Novo item a ser vendido]\n");
        printf("ID: %d Nome: %s Categoria: %s Preco Atual: %d Preco Agora: ", item[0].ID, item[0].Nome, item[0].Categoria, item[0].preco_base);
        if(item[0].preco_agora == 0)
          printf("- ");
        else
          printf("%d ", item[0].preco_agora);
        fflush(stdout);
        printf("Duracao: %d\n", item[0].duracao);
      }
    }
    else if(strcmp(comando, "solditem") == 0)
    {
      item = realloc(item, sizeof(Item));
      n = read(data->caixa, item, sizeof(Item));
      if(n == sizeof(Item))
      {
        printf("[Item vendido]\n");
        printf("ID: %d Nome: %s Categoria: %s Preco: %d Buyer: %s\n", item[0].ID, item[0].Nome, item[0].Categoria, item[0].preco_base, item[0].highestbidder);
      }
    }
    else if(strcmp(comando, "expireditem") == 0)
    {
      item = realloc(item, sizeof(Item));
      n = read(data->caixa, item, sizeof(Item));
      if(n == sizeof(Item))
      {
        printf("[Item nao vendido (passou o tempo)]\n");
        printf("ID: %d Nome: %s Categoria: %s Preco: %d Seller: %s\n", item[0].ID, item[0].Nome, item[0].Categoria, item[0].preco_base, item[0].seller);
      }
    }
    else if(strcmp(comando, "serverlogout") == 0)
    {
      printf("SERVIDOR FECHOU!\n");
      printf("Clique ENTER para sair...\n");
      data->forceExit = 0;
    }
    else if(strcmp(comando, "serverkick") == 0)
    {
      printf("BANIDO TEMPORARIAMENTE!\n");
      printf("Clique ENTER para sair...\n");
      data->forceExit = 0;
    }
    else if(strcmp(comando, "discount") == 0)
    {
      n = read(data->caixa, &discount, sizeof(Discount));
      if(n == sizeof(Discount))
      {
        printf("Novo desconto!\n");
        printf("Categoria: %s Percentagem: %d Duracao: %d\n", discount.Categoria, discount.percentagem, discount.duracao);
      }
    }
    else if(strcmp(comando, "expireddiscount") == 0)
    {
      n = read(data->caixa, str, sizeof(str));
      if(n == sizeof(str))
      {
        if(strcmp(last, str) != 0)
        {
          printf("Desconto acabou na categoria %s\n", str);
          strcpy(last, str);
        }
      }
    }
    
    fflush(stdout);
    pthread_mutex_unlock(data->wait);
  } while (data->continua && data->forceExit);
  
  free(item);
  pthread_exit(NULL);
}

void *heartbeat(void *pdata)
{
  USER_DATA *data = pdata;
  do
  {
    pthread_mutex_lock(data->wait);
    strcpy(data->user.input[0], "HEARTBEAT");
    write(data->bf, &data->user, sizeof(data->user));
    pthread_mutex_unlock(data->wait);
    for(int i = 0; i < data->nheartbeat; i++)
    {
      sleep(1);
      if(!(data->continua && data->forceExit))
        break;
    }
  } while (data->continua && data->forceExit);
  pthread_exit(NULL);
}

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
