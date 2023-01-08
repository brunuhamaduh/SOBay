#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h> 
#include "Header/users_lib.h"
#include "Header/backend_lib.h"
#define MAX 100

int main(int argc, char *argv[], char *env[])
{
  int nclientes = 0, index = 0, nproms = 0, bf, tempo;
  int cliente[20] = {0}, prom[20] = {0}, heartbeat[20] = {0};
  char comando[MAX], buffer[MAX];
  char *nomecliente[20] = {'\0'};
  char *nomeProm[20] = {'\0'};
  char filename[3][50] = {'\0'}, nomeprom2[10][20];
  char NomeCli[10], buftemp[50];
  bool available[10];
  FILE *fp = fopen(filename[2], "r");
  User user;
  User temp;
  USER_DATA data;
  pthread_mutex_t wait;
  pthread_t thread[2];
  pthread_t promotor[10];
  
  for(int i = 0; i < 10; i++)
    available[i] = 1;
  
  getFileNames(env, filename);

  mkfifo("BF", 0666);
  bf = open("BF", O_RDWR);

  pthread_mutex_init(&wait, NULL);
  
  data.continua = 1;
  data.bf = bf;
  data.tempo = &tempo;
  
  data.cliente = cliente;
  data.nclientes = &nclientes;
  data.nomecliente = nomecliente;
  
  data.prom = prom;
  data.nprom = &nproms;
  data.nomeprom = nomeProm;
  data.index = &index;
  data.available = available;
  data.heartbeat = heartbeat;
  if(getenv("HEARTBEAT") != NULL)
    data.maxheartbeat = atoi(getenv("HEARTBEAT"));
  else
    data.maxheartbeat = 10;

  strcpy(data.userfilename, filename[0]);
  strcpy(data.itemfilename, filename[1]);
  strcpy(data.promfilename, filename[2]);
  data.wait = &wait;

  pthread_create(&thread[0], NULL, trata_comandos, &data);
  pthread_create(&thread[1], NULL, trata_segundos, &data);

  printf("Welcome Admin\n");

  do
  {
    fflush(stdout);
    fgets(comando, MAX, stdin);
    if(strcmp(comando, "\n") == 0)
      continue;
    comando[strcspn(comando, "\n")] = '\0'; //retira a newline do fgets;
    
    if(!VerificaComando(comando, &user))
      printf("Comando inválido\n");
    else
    {
      pthread_mutex_lock(&wait);
      if(strcmp(comando, "reprom") == 0)
      {
        for(int i = 0; i < 10; i++)
        {
          if(prom[i] != 0)
          {
            kill(prom[i], SIGUSR1);
            pthread_join(promotor[i], NULL);
          }
          available[i] = true;
        }
        nproms = 0;
        fp = fopen(filename[2], "r");

        if(fp != NULL)
        {
          while(fgets(buffer, sizeof(buffer), fp) != NULL)
          {
            sscanf(buffer, "%s", buftemp);
            if(nproms < 10)
            {
              for(int i = 0; i < 10; i++)
              {
                if(available[i] == true && (strcmp(buftemp, "promotor_oficial") == 0 || strcmp(buftemp, "black_friday") == 0))
                {
                  index = i;
                  strcpy(nomeprom2[i], buftemp);
                  nomeProm[i] = nomeprom2[i];
                  available[i] = false;
                  nproms++;
                  pthread_create(&promotor[i], NULL, trata_promotor, &data);
                  break;
                }
              }
            } 
            sleep(1);
          }
          fclose(fp);
        }
        else
          printf("Nao foi possivel abrir ficheiro dos promotores\n");
      }
      else if(strcmp(comando, "prom") == 0)
      {
        if(nproms != 0)
          printf("Promotores ativos: \n");
        else
          printf("Nao existem promotores ativos\n");
        for(int i = 0; i < nproms; i++)
        {
          if(strcmp(nomeprom2[i], "REMOVED") != 0)
            printf("%s ", nomeprom2[i]);
        }
        printf("\n");
      }
      else if(strcmp(comando, "cancel") == 0)
      {
        for(int i = 0; i < nproms; i++)
        {
          if(strcmp(user.input[1], nomeProm[i]) == 0)
          {
            kill(prom[i], SIGUSR1);
            pthread_join(promotor[i], NULL);    
            available[i] = true;
            strcpy(nomeprom2[i], "REMOVED");
            prom[i] = 0;
          }
        }
      }
      else if(strcmp(comando, "users") == 0)
      {
        if(nclientes != 0)
          printf("Clientes Online\n");
        else
          printf("Nao existem clientes online\n");
        for(int i = 0; i < nclientes; i++)
        {
          printf("%s\n", nomecliente[i]);
        }
      }
      else if(strcmp(comando, "list") == 0)
      {
        if(data.nitems != 0)
          printf("|ID|NOME PRODUTO|CATEGORIA|PRECOB|PRECOA|SEC|SELLER|HBidder|\n");
        else
          printf("Nao existem produtos a serem vendidos\n");
        for(int i = 0; i < data.nitems; i++)
        {
          printf("|%-2.2d|%-12.12s|%-9.9s|%-6.6d|%-6.6d|%-3.3d|%-6.6s|%-7.7s|", data.Items[i].ID, data.Items[i].Nome, data.Items[i].Categoria, data.Items[i].preco_base, data.Items[i].preco_agora, data.Items[i].duracao, data.Items[i].seller, data.Items[i].highestbidder);
          if(data.Items[i].activeDiscount)
            printf("EM DESCONTO\n");
          else
            printf("\n");
        }
      }
      else if(strcmp(comando, "kick") == 0)
      {
        int i = 0;
        for(i = 0; i < nclientes; i++)
        {
          if(strcmp(user.input[1], nomecliente[i]) == 0)
            break;
        }
        if(i == nclientes)
          printf("Nao existe clientes com este username\n");
        else
        {
          printf("Utilizador kickado\n");
          sprintf(NomeCli, "CLI%d", cliente[i]);
          int fdcli = open(NomeCli, O_WRONLY);
          strcpy(comando, "serverkick");
          write(fdcli, comando, sizeof(comando));
          close(fdcli);
          cliente[i] = 0;
          int k = 0;
          for(int j = 0; k < nclientes; j++)
          {
            if(j == i)
              k++;
            nomecliente[j] = nomecliente[k];
            k++;
          }

          nclientes = nclientes - 1;
        }
      }
    }
    pthread_mutex_unlock(&wait);
  } while(strcmp(comando, "close") != 0);

  data.continua = 0;
  write(bf, &temp, sizeof(User));

  for(int i = 0; i < 10; i++)
  {
    if(prom[i] != 0)
    {
      kill(prom[i], SIGUSR1);
      pthread_join(promotor[i], NULL);
      available[i] = true;
    }
  }

  pthread_join(thread[0], NULL);
  pthread_join(thread[1], NULL);

  pthread_mutex_destroy(&wait);

  int fdcli;

  for(int i = 0; i < nclientes; i++)
  {
    sprintf(NomeCli, "CLI%d", cliente[i]);
    fdcli = open(NomeCli, O_WRONLY);
    strcpy(comando, "serverlogout");
    write(fdcli, comando, sizeof(comando));
    close(fdcli);
  }

  close(bf); //Fecha pipe
  unlink("BF"); //Apaga pipe
  return 0;
}