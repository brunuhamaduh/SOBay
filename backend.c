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
  int continua;
  int bf;
  int *nclientes;
  int *cliente;
  char **nomecliente;
  pthread_mutex_t *wait;
} USER_DATA;

void *trata_login(void *pdata)
{
  USER_DATA *data = pdata;
  Item *Items = malloc(0);
  User user;
  int n;
  int nitems = 0;
  int fdcli;
  int feedback;
  int lastID;
  char NomeCli[10];
  nitems = loadItemsFile("Ficheiros/Items.txt", &Items, &lastID);
  loadUsersFile("Ficheiros/Users.txt");

  do
  {
    n = read(data->bf, &user, sizeof(User));
    if(n == sizeof(User))
    {
      sprintf(NomeCli, "CLI%d", user.pid);
      if(isUserValid(user.Username, user.Password) == 1 && *(data->nclientes) < 20 && strcmp(user.input[0], "login") == 0)
      {
        *(data->nclientes) = *(data->nclientes) + 1;
        data->cliente[*(data->nclientes) - 1] = user.pid;
        data->nomecliente[*(data->nclientes) - 1] = user.Username;
        feedback = 1;
        fdcli = open(NomeCli, O_WRONLY);
        write(fdcli, &feedback, sizeof(feedback));
        close(fdcli);
      }
      else if(strcmp(user.input[0], "exit") == 0)
      {
        for(int i = 0; i < *(data->nclientes); i++)
        {
          if(data->cliente[i] == user.pid)
          {
            data->cliente[i] = 0;
            data->nomecliente[i] = 0;
            *(data->nclientes) = *(data->nclientes) - 1;
            break;
          }
        }
      }
      else if((isUserValid(user.Username, user.Password) == 0 || *(data->nclientes) > 20) && strcmp(user.input[0], "login") == 0)
      {
        feedback = 0;
        fdcli = open(NomeCli, O_WRONLY);
        write(fdcli, &feedback, sizeof(feedback));
        close(fdcli);
      }
      else
      {
        fdcli = open(NomeCli, O_WRONLY);
        if(strcmp(user.input[0], "add") == 0)
        {
          feedback = getUserBalance(user.Username) + atoi(user.input[1]);
          updateUserBalance(user.Username, feedback);
          saveUsersFile("Ficheiros/Users.txt"); 
          write(fdcli, user.input[0], sizeof(user.input[0]));
          write(fdcli, &feedback, sizeof(feedback));
        }
        else if(strcmp(user.input[0], "cash") == 0)
        {
          feedback = getUserBalance(user.Username);
          write(fdcli, user.input[0], sizeof(user.input[0]));
          write(fdcli, &feedback, sizeof(feedback));
        }
        else if(strcmp(user.input[0], "time") == 0)
        {
          time_t now = time(NULL);
          struct tm *tm_struct = localtime(&now);
          int hour = tm_struct->tm_hour;
          int minute = tm_struct->tm_min;
          int sec = tm_struct->tm_sec;
          feedback = (hour * 60 + minute) * 60 + sec;
          write(fdcli, user.input[0], sizeof(user.input[0]));
          write(fdcli, &feedback, sizeof(feedback));
        }
        else if(strcmp(user.input[0], "sell") == 0)
        {
          nitems++;
          Items = realloc(Items, nitems * sizeof(Item));
          Items[nitems - 1].ID = ++lastID;
          strcpy(Items[nitems - 1].Nome, user.input[1]); 
          strcpy(Items[nitems - 1].Categoria, user.input[2]);
          Items[nitems - 1].preco_base = atoi(user.input[3]);
          Items[nitems - 1].preco_agora = atoi(user.input[4]);
          Items[nitems - 1].duracao = atoi(user.input[5]);
          strcpy(Items[nitems - 1].seller, user.Username);
          strcpy(Items[nitems - 1].highestbidder, "-");
          saveItemsFile("Ficheiros/Items.txt", Items, nitems);
          write(fdcli, user.input[0], sizeof(user.input[0]));
          write(fdcli, &Items[nitems - 1].ID, sizeof(Items[nitems - 1].ID));
        }
        else if(strcmp(user.input[0], "list") == 0)
        {
          write(fdcli, user.input[0], sizeof(user.input[0]));
          write(fdcli, &nitems, sizeof(nitems));
          write(fdcli, user.input[0], sizeof(user.input[0]));
          write(fdcli, Items, sizeof(Item) * nitems);
        }
        else if(strcmp(user.input[0], "licat") == 0)
        {
          Item *temp = malloc(0);
          int ntemp = 0;
          
          for(int i = 0; i < nitems; i++)
          {
            if(strcmp(Items[i].Categoria, user.input[1]) == 0)
            {
              ntemp++;
              temp = realloc(temp, sizeof(Item) * ntemp);
              temp[ntemp - 1] = Items[i];
            }
          }
          
          write(fdcli, user.input[0], sizeof(user.input[0]));
          write(fdcli, &ntemp, sizeof(ntemp));
          write(fdcli, user.input[1], sizeof(user.input[1]));
          write(fdcli, temp, sizeof(Item) * ntemp);
          free(temp);
        }
        else if(strcmp(user.input[0], "lisel") == 0)
        {
          Item *temp = malloc(0);
          int ntemp = 0;
          
          for(int i = 0; i < nitems; i++)
          {
            if(strcmp(Items[i].seller, user.input[1]) == 0)
            {
              ntemp++;
              temp = realloc(temp, sizeof(Item) * ntemp);
              temp[ntemp - 1] = Items[i];
            }
          }
          
          write(fdcli, user.input[0], sizeof(user.input[0]));
          write(fdcli, &ntemp, sizeof(ntemp));
          write(fdcli, user.input[1], sizeof(user.input[1]));
          write(fdcli, temp, sizeof(Item) * ntemp);
          free(temp);
        }
        else if(strcmp(user.input[0], "lival") == 0)
        {
          Item *temp = malloc(0);
          int ntemp = 0;
          
          for(int i = 0; i < nitems; i++)
          {
            if(Items[i].preco_agora <= atoi(user.input[1]))
            {
              ntemp++;
              temp = realloc(temp, sizeof(Item) * ntemp);
              temp[ntemp - 1] = Items[i];
            }
          }
          
          write(fdcli, user.input[0], sizeof(user.input[0]));
          write(fdcli, &ntemp, sizeof(ntemp));
          write(fdcli, user.input[1], sizeof(user.input[1]));
          write(fdcli, temp, sizeof(Item) * ntemp);
          free(temp);
        }
        else if(strcmp(user.input[0], "litime") == 0)
        {
          Item *temp = malloc(0);
          int ntemp = 0;
          
          for(int i = 0; i < nitems; i++)
          {
            if(Items[i].duracao <= atoi(user.input[1]))
            {
              ntemp++;
              temp = realloc(temp, sizeof(Item) * ntemp);
              temp[ntemp - 1] = Items[i];
            }
          }
          
          write(fdcli, user.input[0], sizeof(user.input[0]));
          write(fdcli, &ntemp, sizeof(ntemp));
          write(fdcli, user.input[1], sizeof(user.input[1]));
          write(fdcli, temp, sizeof(Item) * ntemp);
          free(temp);
        }
        else if(strcmp(user.input[0], "buy") == 0)
        {
          char feedback2[20] = "Not Found";

          if(getUserBalance(user.Username) <= 0)
          {
            strcpy(feedback2, "Broke");
          }
          else
          {
            for(int i = 0; i < nitems; i++)
            {
              if(Items[i].ID == atoi(user.input[1]) && atoi(user.input[2]) >= Items[i].preco_agora && Items[i].preco_agora != 0)
              {
                if(strcmp(Items[i].seller, user.Username) == 0)
                  strcpy(feedback2, "Own Buy");
                else
                {
                  strcpy(feedback2, "Success Bought");
                  int j;
                  Item *temp;
                  
                  updateUserBalance(user.Username, getUserBalance(user.Username) - Items[i].preco_agora);
                  nitems--;
                  temp = malloc(nitems * sizeof(Item));
                  
                  for(j = 0; j < i; j++)
                  {
                    temp[j] = Items[j];
                  }

                  for(int k = j+1; j < nitems; j++, k++)
                  {
                    temp[j] = Items[k];
                  }

                  Items = realloc(Items, nitems * sizeof(Item));
                  Items = temp;
                  free(temp);

                  saveItemsFile("Ficheiros/Items.txt", Items, nitems);
                  saveUsersFile("Ficheiros/Users.txt");
                }
                break;
              }
              else if(Items[i].ID == atoi(user.input[1]) && (atoi(user.input[2]) < Items[i].preco_agora || Items[i].preco_agora == 0) && atoi(user.input[2]) > Items[i].preco_base)
              {
                if(strcmp(Items[i].seller, user.Username) == 0)
                  strcpy(feedback2, "Own Buy");
                else
                {
                  strcpy(feedback2, "Success");
                  strcpy(Items[i].highestbidder, user.Username);
                  Items[i].preco_base = atoi(user.input[2]);
                  saveItemsFile("Ficheiros/Items.txt", Items, nitems); 
                }
              }
              else if(Items[i].ID == atoi(user.input[1]) && atoi(user.input[2]) <= Items[i].preco_base)
              {
                if(strcmp(Items[i].seller, user.Username) == 0)
                  strcpy(feedback2, "Own Buy");
                else
                  strcpy(feedback2, "Low price");
                
                break;
              }
            }
          }
          write(fdcli, user.input[0], sizeof(user.input[0]));
          write(fdcli, feedback2, sizeof(feedback2));
        }
        close(fdcli);
      }
    }
  } while (data->continua);
  
  free(Items);
  pthread_exit(NULL);
}

int main(int argc, char *argv[], char *env[])
{
  char comando[MAX];
  int cliente[20] = {0};
  int nclientes = 0;
  char *nomecliente[20] = {'\0'};
  int Res, bf;
  
  USER_DATA data;
  pthread_mutex_t wait;
  pthread_t thread;
  User temp;
  
  //getFileNames(env, Userfilename, Itemfilename);

  mkfifo("BF", 0666);
  bf = open("BF", O_RDWR);

  pthread_mutex_init(&wait, NULL);
  
  data.continua = 1;
  data.bf = bf;
  data.cliente = cliente;
  data.nclientes = &nclientes;
  data.nomecliente = nomecliente;
  data.wait = &wait;

  pthread_create(&thread, NULL, trata_login, &data);
  
  do
  {
    printf("Comando: ");
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
          printf("%s ", nomecliente[i]);
          fflush(stdout);
        }
        printf("\n");
      }
    }
  } while(strcmp(comando, "close") != 0);

  data.continua = 0;
  write(bf, &temp, sizeof(User));
  pthread_join(thread, NULL);

  pthread_mutex_destroy(&wait);

  close(bf); //Fecha pipe
  unlink("BF"); //Apaga pipe
  return 0;
}