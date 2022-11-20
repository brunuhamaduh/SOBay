#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <signal.h>
#include "Header/users_lib.h"
#include "Header/backlib.h"
#define MAX 100

int main(int argc, char *argv[], char *env[])
{
  char comando[MAX], input_username[20], input_password[20], Userfilename[30], Itemfilename[30];
  int Res, Num_Items;
  struct Item *Items = malloc(0);
  getFileNames(env, Userfilename, Itemfilename);

  if(strcspn(argv[0], "/") != 1) //Verifica se foi executado diretamente ou não
  {
    int check;
    int Num_Users = loadUsersFile(Userfilename);
    Num_Items = loadItemsFile(Itemfilename, &Items);

    scanf("%s %s", input_username, input_password);
    check = isUserValid(input_username, input_password);
    if(check == 0)
      printf("Não existe/Password errada\n");
    else if(check == 1)
      printf("Utilizador existe\n");
    else
      printf("Erro!\n");
    free(Items);
    return 0;
  }

  //PARTE DO ADMINISTRADOR
  //SÓ ENTRA AQUI QUANDO FOR EXECUTADO DIRETAMENTE
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
          printf("%d %s %s %.2f %.2f %d\n", Items[i].ID, Items[i].Nome, Items[i].Categoria, Items[i].preco_base, Items[i].preco_agora, Items[i].duracao);
      }
    }
  } while(strcmp(comando, "close") != 0);
  return 0;
}