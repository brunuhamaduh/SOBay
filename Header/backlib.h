#ifndef BACKLIB_H
#define BACKLIB_H

struct Item
{
  char Nome[20], Categoria[20];
  float preco_base, preco_agora;
  int ID, duracao;
};

int VerificaArgumentos(char *token);
int VerificaComando(char *string);
int saveItemsFile(char * filename, struct Item *Items, int Num_Items);
int loadItemsFile(char *pathname, struct Item **Items);
void getFileNames(char *env[], char *filename1, char *filename2);
#endif