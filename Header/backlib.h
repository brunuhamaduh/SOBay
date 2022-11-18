#ifndef BACKLIB_H
#define BACKLIB_H

struct Item
{
  char Nome[20], Categoria[20];
  float preco_base, preco_agora;
  int ID, duracao;
};

struct Utilizador
{
  char Username[20], Password[20];
  int saldo;
};

extern struct Utilizador *Utilizadores;
extern int Num_Users;

int VerificaArgumentos(char *token);
int VerificaComando(char *string);
int saveItemsFile(char * filename, struct Item *Items, int Num_Items);
int loadItemsFile(char *pathname, struct Item **Items);
void getUserFileName(char *env[], char *filename);
void getItemFileName(char *env[], char *filename);
#endif