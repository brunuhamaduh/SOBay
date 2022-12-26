#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <signal.h>
#include <pthread.h>
#include "Header/users_lib.h"
#include "Header/backlib.h"
#include "Header/sharedlib.h"
#define MAX 100

typedef struct
{
  int continua;
  int bf;
  int *cliente;
  pthread_mutex_t *wait;
} USER_DATA;

void *trata_login(void *pdata)
{
  USER_DATA *data = pdata;
  User user;
  int n;
  int nclientes = 0;
  int fdcli;
  int feedback;
  char NomeCli[10];
  loadUsersFile("Ficheiros/Users.txt");

  do
  {
    n = read(data->bf, &user, sizeof(User));
    if(n == sizeof(User))
    {
      sprintf(NomeCli, "CLI%d", user.pid);
      if(isUserValid(user.Username, user.Password) == 1 && nclientes < 20 && strcmp(user.input[0], "login") == 0)
      {
        data->cliente[nclientes++] = user.pid;
        feedback = 1;
        fdcli = open(NomeCli, O_WRONLY);
        write(fdcli, &feedback, sizeof(feedback));
        close(fdcli);
      }
      else if(strcmp(user.input[0], "exit") == 0)
      {
        for(int i = 0; i < nclientes; i++)
        {
          if(data->cliente[i] == user.pid)
          {
            data->cliente[i] = 0;
            break;
          }
        }
      }
      else if((isUserValid(user.Username, user.Password) == 0 || nclientes > 20) && strcmp(user.input[0], "login") == 0)
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
        close(fdcli);
      }
    }
  } while (data->continua);
  pthread_exit(NULL);
}

int main(int argc, char *argv[], char *env[])
{
  char comando[MAX], Userfilename[30], Itemfilename[30];
  int cliente[20] = {0};
  int Res, Num_Items, bf;
  
  USER_DATA data[2];
  pthread_mutex_t wait;
  pthread_t thread[2];
  User temp;
  
  struct Item *Items = malloc(0);
  getFileNames(env, Userfilename, Itemfilename);

  mkfifo("BF", 0666);
  bf = open("BF", O_RDWR);

  pthread_mutex_init(&wait, NULL);
  
  data[0].continua = 1;
  data[0].bf = bf;
  data[0].cliente = cliente;
  data[0].wait = &wait;

  pthread_create(&thread[0], NULL, trata_login, &data[0]);
  
  do
  {
    printf("Comando: ");
    fflush(stdout);
    fgets(comando, MAX, stdin);
    comando[strcspn(comando, "\n")] = '\0'; //retira a newline do fgets;
    
    Res = VerificaComando(comando);
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
        printf("CLIENTES = [");
        for(int i = 0; i < 20; i++)
        { 
            printf(" %d", cliente[i]);
        }
        printf(" ]\n");
      }

      else if(strcmp(comando, "items") == 0)
      {
        Num_Items = loadItemsFile(Itemfilename, &Items);
        if(Num_Items == -1)
          exit(-1);
        
        printf("[Items] Lido com sucesso\n");
        printf("ID Nome Categoria Preco_Base Preco_Agora Duracao\n");
        for(int i = 0; i < Num_Items; i++)
          printf("%d %s %s %d %d %d\n", Items[i].ID, Items[i].Nome, Items[i].Categoria, Items[i].preco_base, Items[i].preco_agora, Items[i].duracao);
      }
    }
  } while(strcmp(comando, "close") != 0);

  data[0].continua = 0;
  write(bf, &temp, sizeof(User));
  pthread_join(thread[0], NULL);

  pthread_mutex_destroy(&wait);

  close(bf); //Fecha pipe
  unlink("BF"); //Apaga pipe
  return 0;
}