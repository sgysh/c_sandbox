#include <stdio.h>

int main(void) {
#ifdef __STDC__
  printf("__STDC__: %d\n", __STDC__);
#else
  printf("__STDC__: undefined\n");
#endif

#ifdef __STDC_VERSION__
  printf("__STDC_VERSION__: %ld\n", __STDC_VERSION__);
#else
  printf("__STDC_VERSION__: undefined\n");
#endif

  return 0;
}
