#include "users_lib.h"
#include "backlib.h"

int saveUsersFile(char * filename)
{
  FILE *fp;

  fp = fopen(filename, "w");
  if(fp == NULL)
    return -1;

  for(int i = 0; i < Num_Users; i++)
  {
    fprintf(fp, "%s %s %d\n", Utilizadores[i].Username, Utilizadores[i].Password, Utilizadores[i].saldo);
  }

  fclose(fp);
  return 0;
}

int loadUsersFile(char *pathname)
{
  FILE *fp;
  int i = 0;
  char buffer[100];

  fp = fopen(pathname, "r");
  if(fp == NULL)
    return -1;
    
  while(fgets(buffer, sizeof(buffer), fp) != NULL)
  {
    Utilizadores = realloc(Utilizadores, (i+1) * sizeof(struct Utilizador));
    sscanf(buffer, "%s%s%d", Utilizadores[i].Username, Utilizadores[i].Password, &Utilizadores[i].saldo);
    i++;
  }

  fclose(fp);
  return i;
}

int isUserValid(char * username, char * password)
{
  for(int i = 0; i < Num_Users; i++)
  {
    if(strcmp(Utilizadores[i].Username, username) == 0 && strcmp(Utilizadores[i].Password, password) == 0)
      return 1;
  }
  return 0;
}

int getUserBalance(char * username)
{
  for(int i = 0; i < Num_Users; i++)
  {
    if(strcmp(Utilizadores[i].Username, username) == 0)
      return Utilizadores[i].saldo;
  }
  return -1; //caso não exista utilizador com o username dado
}

int updateUserBalance(char * username, int value)
{
  for(int i = 0; i < Num_Users; i++)
  {
    if(strcmp(Utilizadores[i].Username, username) == 0)
    {
      Utilizadores[i].saldo = value;
      return 1;
    }
  }
  return 0; //caso não exista utilizador com o username dado
}