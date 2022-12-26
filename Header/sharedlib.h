#ifndef SHAREDLIB_H
#define SHAREDLIB_H

typedef struct 
{
  int pid, ninput;
  char Username[20], Password[20];
  char input[6][20];
} User;

#endif