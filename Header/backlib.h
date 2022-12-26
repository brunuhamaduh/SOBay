#ifndef BACKLIB_H
#define BACKLIB_H

typedef struct
{
  char Nome[20], Categoria[20];
  int preco_base, preco_agora;
  int ID, duracao;
} Item;

int VerificaArgumentos(char *token);
int VerificaComando(char *string);
int saveItemsFile(char *filename, Item *Items, int Num_Items);
int loadItemsFile(char *pathname, Item **Items);
void getFileNames(char *env[], char *filename1, char *filename2);
#endif