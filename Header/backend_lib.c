#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "users_lib.h"
#include "backend_lib.h"

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

  if(strcmp(final[0], "users") == 0 || strcmp(final[0], "prom") == 0 || strcmp(final[0], "reprom") == 0 || strcmp(final[0], "items") == 0 || strcmp(final[0], "list") == 0 || strcmp(final[0], "close") == 0)
  {
    if(quantidade == 1)
    {
      strcpy(user->input[0], final[0]);
      valid = true;
    }
  }

  else if(strcmp(final[0], "kick") == 0 || strcmp(final[0], "cancel") == 0)
  {
    if(quantidade == 2)
    {
      strcpy(user->input[0], final[0]);
      strcpy(user->input[1], final[1]);
      valid = true;
    }
  }

  free(token);
  free(final);
  return valid;
}

int saveItemsFile(char * filename, Item *Items, int Num_Items)
{
  FILE *fp;
  double calcPercentagem;
  fp = fopen(filename, "w");
  if(fp == NULL)
    return -1;

  for(int i = 0; i < Num_Items; i++)
  {
    if(!Items[i].activeDiscount)
    {
      fprintf(fp, "%d %s %s %d %d %d %s %s\n", Items[i].ID, Items[i].Nome, Items[i].Categoria, Items[i].preco_base, Items[i].preco_agora, Items[i].duracao, Items[i].seller, Items[i].highestbidder);
    }
    else
    {
      calcPercentagem = 1 - (double)Items[i].percentagem / 100;
      fprintf(fp, "%d %s %s %d %d %d %s %s\n", Items[i].ID, Items[i].Nome, Items[i].Categoria, Items[i].preco_base / calcPercentagem, Items[i].preco_agora / calcPercentagem, Items[i].duracao, Items[i].seller, Items[i].highestbidder);
    }
  }

  fclose(fp);
  return 0;
}

int loadItemsFile(char *pathname, Item **Items, int *lastID)
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
    sscanf(buffer, "%d%s%s%d%d%d%s%s", &(*Items)[i].ID, (*Items)[i].Nome, (*Items)[i].Categoria, &(*Items)[i].preco_base, &(*Items)[i].preco_agora, &(*Items)[i].duracao, (*Items)[i].seller, (*Items)[i].highestbidder);
    (*Items)[i].percentagem = 0;
    (*Items)[i].duracaoDiscount = 0;
    (*Items)[i].activeDiscount = false;
    *lastID = (*Items)[i].ID;
    i++;
  }

  fclose(fp);
  return i;
}

void getFileNames(char *env[], char filename[][50])
{
  char temp[20];

  strcpy(temp, "Ficheiros/");

  if(getenv("FUSERS") != NULL)
    strcat(temp, getenv("FUSERS"));
  else
    strcat(temp, "Users");
  
  strcat(temp,".txt");
  strcpy(filename[0], temp);

  strcpy(temp, "Ficheiros/");

  if(getenv("FITEMS") != NULL)
    strcat(temp, getenv("FITEMS"));
  else
    strcat(temp, "Items");

  strcat(temp,".txt");
  strcpy(filename[1], temp);

  strcpy(temp, "Ficheiros/");

  if(getenv("FPROMOTERS") != NULL)
    strcat(temp, getenv("FPROMOTERS"));
  else
    strcat(temp, "Promoters");

  strcat(temp,".txt");
  strcpy(filename[2], temp);
}

void Abort(char *msg)
{
  printf("%s", msg);
  exit(1);
}

void *trata_comandos(void *pdata)
{
  USER_DATA *data = pdata;
  data->Items = malloc(0);
  int n, fdcli, feedback, lastID, amountDiscount = 0, duracaoDiscount = 0;
  double calcPercentagem = 1;
  bool activeDiscount = false;
  char NomeCli[10], comando[20];
  char Nomes[20][20];
  data->nitems = loadItemsFile(data->itemfilename, &data->Items, &lastID);
  loadUsersFile(data->userfilename);

  do
  {
    n = read(data->bf, &data->user, sizeof(User)); 
    pthread_mutex_lock(data->wait);
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
            calcPercentagem = 1 - (double)amountDiscount / 100;

          data->Items[data->nitems - 1].preco_base = atoi(data->user.input[3]) * calcPercentagem;
          data->Items[data->nitems - 1].preco_agora = atoi(data->user.input[4]) * calcPercentagem;
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
        else if(strcmp(data->user.input[0], "HEARTBEAT") == 0)
        {
          for(int i = 0; i < *data->nclientes; i++)
          {
            if(data->user.pid == data->cliente[i])
            {
              data->heartbeat[i] = 0;
              break;
            }
          }
        }
        close(fdcli);
      }
    }
    pthread_mutex_unlock(data->wait);
  } while (data->continua);
  
  saveItemsFile(data->itemfilename, data->Items, data->nitems);
  pthread_exit(NULL);
}

void *trata_segundos(void *pdata)
{
  USER_DATA *data = pdata;
  FILE *fp;
  char buffer[100], comando[20], NomeCli[10];
  double calcPercentagem;
  int j, k, fdcli;

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
    k = 0;
    sleep(1);
    *data->tempo = *data->tempo + 1;

    pthread_mutex_lock(data->wait);
    
    for(int i = 0; i < *data->nclientes; i++)
    {
      data->heartbeat[i] = data->heartbeat[i] + 1;
    }

    for(int i = 0; i < *data->nclientes; i++)
    {
      if(data->heartbeat[i] > data->maxheartbeat)
      {
        sprintf(NomeCli, "CLI%d", data->cliente[i]);
        printf("Cliente %s saiu sem avisar!\n", data->nomecliente[i]);
        unlink(NomeCli);
        for(int j = i; j < *(data->nclientes) - 1; j++)
        {
          data->nomecliente[j] = data->nomecliente[j + 1];
          data->heartbeat[j] = data->heartbeat[j + 1];
          data->cliente[j] = data->cliente[j + 1];
        }
        *(data->nclientes) = *(data->nclientes) - 1;
        i = 0;
      }
    }

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
    pthread_mutex_unlock(data->wait);
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

        pthread_mutex_lock(data->wait);
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
        pthread_mutex_unlock(data->wait);
      }
    }
    else
      break;
  }

  close(prom[0]);
  wait(NULL);
  pthread_exit(NULL);
}