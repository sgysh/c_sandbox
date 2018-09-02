#include <stdio.h>

void print_int(int v)            { printf("%d\n", v); }
void print_double(double v)      { printf("%f\n", v); }
void print_chars(const char *v)  { printf("%s\n", v); }

#define print(v) _Generic((v)   \
   , int:          print_int    \
   , double:       print_double \
   , char *:       print_chars  \
   , const char *: print_chars  \
)(v);

int main(void) {
   print(1);
   print(1.1);
   print("A");

   return 0;
}

