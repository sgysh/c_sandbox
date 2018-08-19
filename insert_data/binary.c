#include <stdio.h>
#include <stdlib.h>

extern char __start_bindata;
extern char __stop_bindata;

int main()
{
  char * p;

  printf("__start_bindata = 0x%016qx\n", (long long)(&__start_bindata));
  printf("__stop_bindata = 0x%016qx\n", (long long)(&__stop_bindata));

  for (p = &__start_bindata; p < &__stop_bindata; p++)
    putchar(*p);

  exit(0);
}
