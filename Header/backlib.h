#ifndef BACKLIB_H
#define BACKLIB_H

typedef struct
{
  char Nome[20], Categoria[20], seller[20], highestbidder[20];
  int preco_base, preco_agora;
  int ID, duracao;

} Item;

typedef struct 
{
  int pid, ninput;
  char Username[20], Password[20];
  char input[6][20];
} User;

int VerificaComandoAdmin(char *string, User *user);
int saveItemsFile(char *filename, Item *Items, int Num_Items);
int loadItemsFile(char *pathname, Item **Items, int *lastID);
void getFileNames(char *env[], char filename[][50]);
void Abort(char *msg);
bool VerificaComando(char *string, User *user);
#endif