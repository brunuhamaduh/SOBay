#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <signal.h>
#include <pthread.h>
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
  int *nclientes;
  int *cliente;
  char **nomecliente;
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
  char NomeCli[10];
  char comando[20];
  char Nomes[20][20];
  data->nitems = loadItemsFile("Ficheiros/Items.txt", &data->Items, &lastID);
  loadUsersFile("Ficheiros/Users.txt");

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
          saveUsersFile("Ficheiros/Users.txt"); 
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
          data->nitems = data->nitems + 1;
          data->Items = realloc(data->Items, data->nitems * sizeof(Item));
          data->Items[data->nitems - 1].ID = ++lastID;
          strcpy(data->Items[data->nitems - 1].Nome, data->user.input[1]); 
          strcpy(data->Items[data->nitems - 1].Categoria, data->user.input[2]);
          data->Items[data->nitems - 1].preco_base = atoi(data->user.input[3]);
          data->Items[data->nitems - 1].preco_agora = atoi(data->user.input[4]);
          data->Items[data->nitems - 1].duracao = atoi(data->user.input[5]);
          strcpy(data->Items[data->nitems - 1].seller, data->user.Username);
          strcpy(data->Items[data->nitems - 1].highestbidder, "-");
          saveItemsFile("Ficheiros/Items.txt", data->Items, data->nitems);
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
                  saveItemsFile("Ficheiros/Items.txt", data->Items, data->nitems);
                  saveUsersFile("Ficheiros/Users.txt");
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
                  saveItemsFile("Ficheiros/Items.txt", data->Items, data->nitems); 
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

  loadUsersFile("Ficheiros/Users.txt");
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
      data->Items[i].duracao = data->Items[i].duracao - 1;
      if(data->Items[i].duracao == 0)
      {
        if(strcmp(data->Items[i].highestbidder, "-") != 0)
        {
          strcpy(comando, "solditem");
          updateUserBalance(data->Items[i].highestbidder, getUserBalance(data->Items[i].highestbidder) - data->Items[i].preco_base);
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
        saveItemsFile("Ficheiros/Items.txt", data->Items, data->nitems);
        saveUsersFile("Ficheiros/Users.txt");
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

int main(int argc, char *argv[], char *env[])
{
  char comando[MAX];
  int cliente[20] = {0};
  int nclientes = 0;
  char *nomecliente[20] = {'\0'};
  int Res, bf;
  int tempo;
  
  USER_DATA data;
  pthread_mutex_t wait;
  pthread_t thread[2];
  User temp;
  
  //getFileNames(env, Userfilename, Itemfilename);

  mkfifo("BF", 0666);
  bf = open("BF", O_RDWR);

  pthread_mutex_init(&wait, NULL);
  
  data.continua = 1;
  data.bf = bf;
  data.tempo = &tempo;
  data.cliente = cliente;
  data.nclientes = &nclientes;
  data.nomecliente = nomecliente;
  data.wait = &wait;

  pthread_create(&thread[0], NULL, trata_comandos, &data);
  pthread_create(&thread[1], NULL, trata_segundos, &data);

  printf("Welcome Admin\n");

  do
  {
    fflush(stdout);
    fgets(comando, MAX, stdin);
    comando[strcspn(comando, "\n")] = '\0'; //retira a newline do fgets;
    
    Res = VerificaComandoAdmin(comando);
    if(Res == 0 && strcmp(comando, "close") != 0)
      printf("Comando inválido\n");
    else if(Res == 1)
      printf("Número de argumentos incorrecto\n");
    else if(Res == 2)
    {
      if(strcmp(comando, "reprom") == 0)
      {
        if(fork() == 0)
        {
          int PID_Promotor, prom[2];
          union sigval stop;

          pipe(prom);
          PID_Promotor = fork();

          if(PID_Promotor == 0)
          {
            close(prom[0]); //close read
            close(1);
            dup(prom[1]);
            close(prom[1]);
            execl("Promotor/promotor_oficial", "Promotor/promotor_oficial", NULL);
          }
          close(prom[1]); //close write
          
          while(kill(PID_Promotor, 0) == 0) //enquanto estiver a correr (META 1)
          {
            char buffer[100];
            int nbytes = read(prom[0], buffer, sizeof(buffer));
            buffer[nbytes] = '\0';
            if(nbytes > 1)
              printf("Promoção na categoria ");
            printf("%s", buffer);
            sigqueue(PID_Promotor, SIGUSR1, stop); //APENAS META 1
          }
          close(prom[0]);
          exit(0);
        }
      }
      else if(strcmp(comando, "users") == 0)
      {
        printf("Clientes Online\n");
        for(int i = 0; i < nclientes; i++)
        {
          printf("%s\n", nomecliente[i]);
        }
      }
    }
  } while(strcmp(comando, "close") != 0);

  data.continua = 0;
  write(bf, &temp, sizeof(User));
  pthread_join(thread[0], NULL);
  pthread_join(thread[1], NULL);
  pthread_mutex_destroy(&wait);

  char NomeCli[20];
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