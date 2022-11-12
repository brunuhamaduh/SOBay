#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <stdbool.h>

typedef struct
{
  char User[50], Password[20];
} Utilizador;

int main(void)
{
  char User[10], Password[10];
  Utilizador Admin = {"Admin", "Admin123"};
  Utilizador User1 = {"MariaLeal","TaDemais"};
  Utilizador Utilizadores[] = {Admin, User1};
  int Match = 0;

  scanf("%s %s", User, Password);

  for(int i = 0; i < sizeof(Utilizadores)/sizeof(Utilizadores[0]); i++)
  {
    if(strcmp(Utilizadores[i].User, User) == 0)
    {
       if(strcmp(Utilizadores[i].Password, Password) == 0)
       {
          Match = 1;
       }
    }
  }

  printf("%d", Match);
  return 0;
}