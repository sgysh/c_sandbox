#include <stdio.h>

int __attribute__((cdecl)) func(void) {
  return 4;
}

int g = 0;

int main(void) {
  func();
  __asm__("movl %eax,g");

  printf("eax(return value): %d\n", g);

  return 0;
}
