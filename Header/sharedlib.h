#ifndef SHAREDLIB_H
#define SHAREDLIB_H

typedef struct 
{
  int pid, intent;
  char Username[20], Password[20];
} User;

#endif