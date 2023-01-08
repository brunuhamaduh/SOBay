#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h> 
#include "Header/users_lib.h"
#include "Header/backlib.h"
#define MAX 100

typedef struct
{
  int *tempo;
  Item *Items;
  User user;
  int nitems;
  int continua;
  int bf;
  int *index;
  char userfilename[50];
  char promfilename[50];
  char itemfilename[50];
  bool *available;
  char **nomecliente;
  int *cliente;
  int *nclientes;
  char **nomeprom; //ficheiro
  int *prom; //pid
  int *nprom; //quantidade
  pthread_mutex_t *wait;
} USER_DATA;

void *trata_comandos(void *pdata)
{
  USER_DATA *data = pdata;
  data->Items = malloc(0);
  int n;
  int fdcli;
  int feedback;
  int lastID;
  int amountDiscount = 0;
  double calcPercentagem = 0;
  int duracaoDiscount = 0;
  bool activeDiscount = false;
  char NomeCli[10];
  char comando[20];
  char Nomes[20][20];
  data->nitems = loadItemsFile(data->itemfilename, &data->Items, &lastID);
  loadUsersFile(data->userfilename);

  do
  {
    n = read(data->bf, &data->user, sizeof(User)); 
    if(n == sizeof(User))
    {
      sprintf(NomeCli, "CLI%d", data->user.pid);
      if(isUserValid(data->user.Username, data->user.Password) == 1 && *(data->nclientes) < 20 && strcmp(data->user.input[0], "login") == 0)
      {
        *(data->nclientes) = *(data->nclientes) + 1;
        strcpy(Nomes[*(data->nclientes) - 1], data->user.Username);
        data->nomecliente[*(data->nclientes) - 1] = Nomes[*(data->nclientes) - 1];
        data->cliente[*(data->nclientes) - 1] = data->user.pid;
        feedback = 1;
        fdcli = open(NomeCli, O_WRONLY);
        write(fdcli, &feedback, sizeof(feedback));
        close(fdcli);
      }
      else if(strcmp(data->user.input[0], "exit") == 0)
      {
        for(int i = 0; i < *(data->nclientes); i++)
        {
          if(data->cliente[i] == data->user.pid)
          {
            int k = 0;
            data->cliente[i] = 0;
            for(int j = 0; k < *(data->nclientes); j++)
            {
              if(j == i)
                k++;
              strcpy(Nomes[j], Nomes[k]);
              k++;
            }

            *(data->nclientes) = *(data->nclientes) - 1;
            fdcli = open(NomeCli, O_WRONLY);
            feedback = -1;
            write(fdcli, &feedback, sizeof(feedback));
            close(fdcli);
            break;
          }
        }
      }
      else if((isUserValid(data->user.Username, data->user.Password) == 0 || *(data->nclientes) > 20) && strcmp(data->user.input[0], "login") == 0)
      {
        feedback = 0;
        fdcli = open(NomeCli, O_WRONLY);
        write(fdcli, &feedback, sizeof(feedback));
        close(fdcli);
      }
      else
      {
        fdcli = open(NomeCli, O_WRONLY);
        if(strcmp(data->user.input[0], "add") == 0)
        {
          feedback = getUserBalance(data->user.Username) + atoi(data->user.input[1]);
          updateUserBalance(data->user.Username, feedback);
          saveUsersFile(data->userfilename); 
          write(fdcli, data->user.input[0], sizeof(data->user.input[0]));
          write(fdcli, &feedback, sizeof(feedback));
        }
        else if(strcmp(data->user.input[0], "cash") == 0)
        {
          feedback = getUserBalance(data->user.Username);
          write(fdcli, data->user.input[0], sizeof(data->user.input[0]));
          write(fdcli, &feedback, sizeof(feedback));
        }
        else if(strcmp(data->user.input[0], "time") == 0)
        {
          feedback = *data->tempo;
          write(fdcli, data->user.input[0], sizeof(data->user.input[0]));
          write(fdcli, &feedback, sizeof(feedback));
        }
        else if(strcmp(data->user.input[0], "sell") == 0)
        {
          for(int z = 0; z < data->nitems; z++)
          {
            if(strcmp(data->Items[z].Categoria, data->user.input[2]) == 0)
            {
              activeDiscount = data->Items[z].activeDiscount;
              amountDiscount = data->Items[z].percentagem;
              duracaoDiscount = data->Items[z].duracaoDiscount;
              break;
            }
          }

          data->nitems = data->nitems + 1;
          data->Items = realloc(data->Items, data->nitems * sizeof(Item));
          data->Items[data->nitems - 1].ID = ++lastID;
          strcpy(data->Items[data->nitems - 1].Nome, data->user.input[1]); 
          strcpy(data->Items[data->nitems - 1].Categoria, data->user.input[2]);
          if(activeDiscount)
          {
            calcPercentagem = 1 - (double)amountDiscount / 100;
            data->Items[data->nitems - 1].preco_base = atoi(data->user.input[3]) / calcPercentagem;
            data->Items[data->nitems - 1].preco_agora = atoi(data->user.input[4]) / calcPercentagem;
          }
          else
          {
            data->Items[data->nitems - 1].preco_base = atoi(data->user.input[3]);
            data->Items[data->nitems - 1].preco_agora = atoi(data->user.input[4]);
          }
          data->Items[data->nitems - 1].activeDiscount = activeDiscount;
          data->Items[data->nitems - 1].percentagem = amountDiscount;
          data->Items[data->nitems - 1].duracaoDiscount = duracaoDiscount;
          data->Items[data->nitems - 1].duracao = atoi(data->user.input[5]);
          strcpy(data->Items[data->nitems - 1].seller, data->user.Username);
          strcpy(data->Items[data->nitems - 1].highestbidder, "-");

          saveItemsFile(data->itemfilename, data->Items, data->nitems);
          write(fdcli, data->user.input[0], sizeof(data->user.input[0]));
          write(fdcli, &data->Items[data->nitems - 1].ID, sizeof(data->Items[data->nitems - 1].ID));
          close(fdcli);

          Item *temp = malloc(sizeof(Item));
          *temp = data->Items[data->nitems - 1];
          for(int i = 0; i < *(data->nclientes); i++)
          {
            if(data->cliente[i] == data->user.pid)
              continue;
            sprintf(NomeCli, "CLI%d", data->cliente[i]);
            fdcli = open(NomeCli, O_WRONLY);
            strcpy(comando, "newitem");
            write(fdcli, comando, sizeof(comando));
            write(fdcli, temp, sizeof(Item));
            close(fdcli);
          }
          free(temp);
        }
        else if(strcmp(data->user.input[0], "list") == 0)
        {
          write(fdcli, data->user.input[0], sizeof(data->user.input[0]));
          write(fdcli, &data->nitems, sizeof(data->nitems));
          write(fdcli, data->user.input[0], sizeof(data->user.input[0]));
          write(fdcli, data->Items, sizeof(Item) * data->nitems);
        }
        else if(strcmp(data->user.input[0], "licat") == 0)
        {
          Item *temp = malloc(0);
          int ntemp = 0;
          
          for(int i = 0; i < data->nitems; i++)
          {
            if(strcmp(data->Items[i].Categoria, data->user.input[1]) == 0)
            {
              ntemp++;
              temp = realloc(temp, sizeof(Item) * ntemp);
              temp[ntemp - 1] = data->Items[i];
            }
          }
          write(fdcli, data->user.input[0], sizeof(data->user.input[0]));
          write(fdcli, &ntemp, sizeof(ntemp));
          write(fdcli, data->user.input[1], sizeof(data->user.input[1]));
          write(fdcli, temp, sizeof(Item) * ntemp);
          free(temp);
        }
        else if(strcmp(data->user.input[0], "lisel") == 0)
        {
          Item *temp = malloc(0);
          int ntemp = 0;
          
          for(int i = 0; i < data->nitems; i++)
          {
            if(strcmp(data->Items[i].seller, data->user.input[1]) == 0)
            {
              ntemp++;
              temp = realloc(temp, sizeof(Item) * ntemp);
              temp[ntemp - 1] = data->Items[i];
            }
          }
          write(fdcli, data->user.input[0], sizeof(data->user.input[0]));
          write(fdcli, &ntemp, sizeof(ntemp));
          write(fdcli, data->user.input[1], sizeof(data->user.input[1]));
          write(fdcli, temp, sizeof(Item) * ntemp);
          free(temp);
        }
        else if(strcmp(data->user.input[0], "lival") == 0)
        {
          Item *temp = malloc(0);
          int ntemp = 0;
          
          for(int i = 0; i < data->nitems; i++)
          {
            if(data->Items[i].preco_base <= atoi(data->user.input[1]))
            {
              ntemp++;
              temp = realloc(temp, sizeof(Item) * ntemp);
              temp[ntemp - 1] = data->Items[i];
            }
          }
          write(fdcli, data->user.input[0], sizeof(data->user.input[0]));
          write(fdcli, &ntemp, sizeof(ntemp));
          write(fdcli, data->user.input[1], sizeof(data->user.input[1]));
          write(fdcli, temp, sizeof(Item) * ntemp);
          free(temp);
        }
        else if(strcmp(data->user.input[0], "litime") == 0)
        {
          Item *temp = malloc(0);
          int ntemp = 0;
          
          for(int i = 0; i < data->nitems; i++)
          {
            if(data->Items[i].duracao <= atoi(data->user.input[1]))
            {
              ntemp++;
              temp = realloc(temp, sizeof(Item) * ntemp);
              temp[ntemp - 1] = data->Items[i];
            }
          }
          write(fdcli, data->user.input[0], sizeof(data->user.input[0]));
          write(fdcli, &ntemp, sizeof(ntemp));
          write(fdcli, data->user.input[1], sizeof(data->user.input[1]));
          write(fdcli, temp, sizeof(Item) * ntemp);
          free(temp);
        }
        else if(strcmp(data->user.input[0], "buy") == 0)
        {
          char feedback2[20] = "Not Found";

          if(getUserBalance(data->user.Username) <= 0)
          {
            strcpy(feedback2, "Broke");
          }
          else
          {
            for(int i = 0; i < data->nitems; i++)
            {
              if(data->Items[i].ID == atoi(data->user.input[1]) && atoi(data->user.input[2]) >= data->Items[i].preco_agora && data->Items[i].preco_agora != 0)
              {
                if(strcmp(data->Items[i].seller, data->user.Username) == 0)
                  strcpy(feedback2, "Own Buy");
                else
                {
                  strcpy(feedback2, "Success Bought");
                  int j;
                  int k = 0;
                  
                  updateUserBalance(data->user.Username, getUserBalance(data->user.Username) - data->Items[i].preco_agora);
                  if(data->Items[i].activeDiscount == true)
                  {
                    calcPercentagem = 1 - (double)data->Items[i].percentagem / 100;
                    updateUserBalance(data->Items[i].seller, getUserBalance(data->Items[i].seller) + (data->Items[i].preco_base / calcPercentagem));
                  }
                  
                  Item *temp = malloc(sizeof(Item));

                  for(j = 0; k < data->nitems; j++)
                  {
                    if(j == i)
                    {
                      k++;
                      strcpy(data->Items[j].highestbidder, data->user.Username);
                      *temp = data->Items[j];
                    }
                      
                    data->Items[j] = data->Items[k];
                    k++;
                  }

                  data->nitems = data->nitems - 1;
                  for(int i = 0; i < *(data->nclientes); i++)
                  {
                    if(data->cliente[i] == data->user.pid)
                      continue;
                    sprintf(NomeCli, "CLI%d", data->cliente[i]);
                    fdcli = open(NomeCli, O_WRONLY);
                    strcpy(comando, "solditem");
                    write(fdcli, comando, sizeof(comando));
                    write(fdcli, temp, sizeof(Item));
                    close(fdcli);
                  }
                  free(temp);
                  data->Items = realloc(data->Items, data->nitems * sizeof(Item));
                  saveItemsFile(data->itemfilename, data->Items, data->nitems);
                  saveUsersFile(data->userfilename);
                }
                break;
              }
              else if(data->Items[i].ID == atoi(data->user.input[1]) && (atoi(data->user.input[2]) < data->Items[i].preco_agora || data->Items[i].preco_agora == 0) && atoi(data->user.input[2]) > data->Items[i].preco_base)
              {
                if(strcmp(data->Items[i].seller, data->user.Username) == 0)
                  strcpy(feedback2, "Own Buy");
                else
                {
                  strcpy(feedback2, "Success");
                  strcpy(data->Items[i].highestbidder, data->user.Username);
                  data->Items[i].preco_base = atoi(data->user.input[2]);
                  saveItemsFile(data->itemfilename, data->Items, data->nitems); 
                }
              }
              else if(data->Items[i].ID == atoi(data->user.input[1]) && atoi(data->user.input[2]) <= data->Items[i].preco_base)
              {
                if(strcmp(data->Items[i].seller, data->user.Username) == 0)
                  strcpy(feedback2, "Own Buy");
                else
                  strcpy(feedback2, "Low price");
                
                break;
              }
            }
          }
          write(fdcli, data->user.input[0], sizeof(data->user.input[0]));
          write(fdcli, feedback2, sizeof(feedback2));
        }
        close(fdcli);
      }
    }
  } while (data->continua);

  pthread_exit(NULL);
}

void *trata_segundos(void *pdata)
{
  USER_DATA *data = pdata;
  FILE *fp;
  char buffer[100];
  double calcPercentagem;

  loadUsersFile(data->userfilename);
  fp = fopen("Ficheiros/tempo.txt", "r");
  
  if(fp == NULL)
    printf("Primeira vez que plataforma esta a ser iniciada...\n");
  else
  {
    while(fgets(buffer, sizeof(buffer), fp) != NULL)
      sscanf(buffer, "%d", data->tempo);
    fclose(fp);
  }
  
  do
  {
    int j, k = 0, fdcli;
    char comando[20], NomeCli[10];
    sleep(1);
    *data->tempo = *data->tempo + 1;
    for(int i = 0; i < data->nitems; i++)
    {
      if(data->Items[i].duracao > 0)
        data->Items[i].duracao = data->Items[i].duracao - 1;
      if(data->Items[i].duracaoDiscount > 0)
        data->Items[i].duracaoDiscount = data->Items[i].duracaoDiscount - 1;
      
      if(data->Items[i].duracao == 0)
      {
        if(strcmp(data->Items[i].highestbidder, "-") != 0)
        {
          strcpy(comando, "solditem");
          updateUserBalance(data->Items[i].highestbidder, getUserBalance(data->Items[i].highestbidder) - data->Items[i].preco_base);
          if(data->Items[i].activeDiscount == true)
          {
            calcPercentagem = 1 - (double)data->Items[i].percentagem / 100;
            updateUserBalance(data->Items[i].seller, getUserBalance(data->Items[i].seller) + (data->Items[i].preco_base / calcPercentagem));
          }
        }
        else
          strcpy(comando, "expireditem");

        Item *temp = malloc(sizeof(Item));

        for (j = 0; k < data->nitems; j++)
        {
          if (j == i)
          {
            k++;
            *temp = data->Items[j];
          }

          data->Items[j] = data->Items[k];
          k++;
        }

        data->nitems = data->nitems - 1;
        for (int i = 0; i < *(data->nclientes); i++)
        {
          sprintf(NomeCli, "CLI%d", data->cliente[i]);
          fdcli = open(NomeCli, O_WRONLY);
          write(fdcli, comando, sizeof(comando));
          write(fdcli, temp, sizeof(Item));
          close(fdcli);
        }
        free(temp);
        data->Items = realloc(data->Items, data->nitems * sizeof(Item));
        saveItemsFile(data->itemfilename, data->Items, data->nitems);
        saveUsersFile(data->userfilename);
      }
      else if(data->Items[i].duracaoDiscount == 0 && data->Items[i].activeDiscount)
      {
        calcPercentagem = 1 - (double)data->Items[i].percentagem / 100;
        data->Items[i].activeDiscount = false;
        data->Items[i].percentagem = 0;
        data->Items[i].preco_base = data->Items[i].preco_base / calcPercentagem;
        data->Items[i].preco_agora = data->Items[i].preco_agora / calcPercentagem;
        strcpy(comando, "expireddiscount");
        for (int j = 0; j < *(data->nclientes); j++)
        {
          sprintf(NomeCli, "CLI%d", data->cliente[j]);
          fdcli = open(NomeCli, O_WRONLY);
          write(fdcli, comando, sizeof(comando));
          write(fdcli, data->Items[i].Categoria, sizeof(data->Items[i].Categoria));
          close(fdcli);
        }
      }
    }
  } while (data->continua);

  fp = fopen("Ficheiros/tempo.txt", "w");
  if(fp == NULL)
    printf("Nao foi possivel guardar o tempo da plataforma...\n");
  else
  {
    fprintf(fp, "%d", *data->tempo);
    fclose(fp);
  }
  
  pthread_exit(NULL);
}

void *trata_promotor(void *pdata)
{
  USER_DATA *data = pdata;
  Discount discount;
  int PID_Promotor, nbytes, fdcli;
  int prom[2];
  char output[20], NomeCli[50], buffer[100], ficheiro[100];
  char *token;
  double calcPercentagem;
  bool change = true;
  pipe(prom);
  strcpy(ficheiro, "Promotor/");
  strcat(ficheiro, data->nomeprom[*data->index]);

  PID_Promotor = fork();
  if(PID_Promotor == 0)
  {
    close(prom[0]); //close read
    close(1);
    dup(prom[1]);
    close(prom[1]);
    execl(ficheiro, ficheiro, NULL);
  }

  close(prom[1]); //close write
  data->prom[*data->index] = PID_Promotor;
  
  while(kill(PID_Promotor, 0) == 0 && data->continua)
  {
    nbytes = read(prom[0], &buffer, sizeof(buffer));
    if(nbytes != 0)
    {
      buffer[nbytes] = '\0';
      
      if(nbytes != 1)
      {
        token = strtok(buffer, " ");
        strcpy(discount.Categoria, token);
        token = strtok(NULL, " ");
        discount.percentagem = atoi(token);
        calcPercentagem = 1 - (double)discount.percentagem / 100;
        token = strtok(NULL, " ");
        discount.duracao = atoi(token);

        for(int i = 0; i < data->nitems; i++)
        {
          if(strcmp(discount.Categoria, data->Items[i].Categoria) == 0 && discount.percentagem <= data->Items[i].percentagem && data->Items[i].activeDiscount == true)
          {
            change = false;
            break;
          }
        }
        
        if(change)
        {
          strcpy(output, "discount");

          for(int i = 0; i < data->nitems; i++)
          {
            if(strcmp(discount.Categoria, data->Items[i].Categoria) == 0)
            {
              data->Items[i].activeDiscount = true;
              data->Items[i].percentagem = discount.percentagem;
              data->Items[i].duracaoDiscount = discount.duracao;
              data->Items[i].preco_base = data->Items[i].preco_base * calcPercentagem;
              data->Items[i].preco_agora = data->Items[i].preco_agora * calcPercentagem;
            }
          }

          for (int i = 0; i < *data->nclientes; i++)
          {
            sprintf(NomeCli, "CLI%d", data->cliente[i]);
            fdcli = open(NomeCli, O_WRONLY);
            write(fdcli, output, sizeof(output));
            write(fdcli, &discount, sizeof(Discount));
            close(fdcli);
          }
        }
      }
    }
    else
      break;
  }

  close(prom[0]);
  wait(NULL);
  pthread_exit(NULL);
}

int main(int argc, char *argv[], char *env[])
{
  int nclientes = 0, index = 0, nproms = 0, bf, tempo;
  int cliente[20] = {0}, prom[20] = {0};
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
    comando[strcspn(comando, "\n")] = '\0'; //retira a newline do fgets;
    
    if(!VerificaComandoAdmin(comando, &user))
      printf("Comando inválido\n");
    else
    {
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
          printf("|%-2.2d|%-12.12s|%-9.9s|%-6.6d|%-6.6d|%-3.3d|%-6.6s|%-7.7s|\n", data.Items[i].ID, data.Items[i].Nome, data.Items[i].Categoria, data.Items[i].preco_base, data.Items[i].preco_agora, data.Items[i].duracao, data.Items[i].seller, data.Items[i].highestbidder);
        }
      }
      else if(strcmp(comando, "prom2") == 0)
      {
        for(int i = 0; i < data.nitems; i++)
        {
          printf("Categoria = %s %d\n", data.Items[i].Categoria, data.Items[i].duracaoDiscount);
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
  } while(strcmp(comando, "close") != 0);

  data.continua = 0;
  write(bf, &temp, sizeof(User));

  pthread_join(thread[0], NULL);
  pthread_join(thread[1], NULL);

  for(int i = 0; i < 10; i++)
  {
    if(prom[i] != 0)
    {
      kill(prom[i], SIGUSR1);
      pthread_join(promotor[i], NULL);
      available[i] = true;
    }
  }
  
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