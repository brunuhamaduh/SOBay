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

void *trata_pipe(void *pdata)
{
  USER_DATA *data = pdata;
  User user;
  int n;
  int nclientes = 0;
  loadUsersFile("Ficheiros/Users.txt");
  do
  {
    n = read(data->bf, &user, sizeof(User));
    if(n == sizeof(User))
    {
      pthread_mutex_lock(data->wait);
      int check = isUserValid(user.Username, user.Password);
      if(check == 1 && nclientes < 20 && user.intent == 1)
      {
        data->cliente[nclientes++] = user.pid;
      }
      else if(user.intent == 0)
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
      pthread_mutex_unlock(data->wait);
    }
  } while (data->continua);
  pthread_exit(NULL);
}

int main(int argc, char *argv[], char *env[])
{
  char comando[MAX], input_username[20], input_password[20], Userfilename[30], Itemfilename[30];
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

  pthread_create(&thread[0], NULL, trata_pipe, &data[0]);
  
  printf("Comandos disponiveis\n");
  printf("------------------------\n");
  printf("users\n");
  printf("list\n"); //ONLY ONE (id, nome item, categoria, preço atual, preço compre já, vendedor, licitador mais elevado ou menos elevado)
  printf("kick <username>\n");
  printf("prom\n");
  printf("reprom\n");
  printf("cancel <nome-do-executavel-do-promotor>\n");
  printf("items\n");
  printf("close\n");
  printf("------------------------\n");
  
  do
  {
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
          int estado, PID_Promotor, prom[2];
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
        int Num_Users = loadUsersFile(Userfilename);
        if(Num_Users == -1)
        {
          printf("Erro ao ler ficheiro\n");
          exit(-1);
        }
        printf("[Users] Lido com sucesso\n");
        if(isUserValid("Cristina", "Ferreira") == 1)
          updateUserBalance("Cristina", getUserBalance("Cristina") - 1);

        if(saveUsersFile(Userfilename) == -1)
        {
          printf("Erro ao escrever ficheiro\n");
          exit(-1);
        }
        printf("[Users] Escrito com sucesso\n");
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