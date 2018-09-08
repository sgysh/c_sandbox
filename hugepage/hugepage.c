#include <stdio.h>
#include <stdlib.h>     /* system() */
#include <sys/mman.h>   /* mmap() */
#include <hugetlbfs.h>  /* gethugepagesize() */

#define CMD "cat /proc/meminfo | grep HugePages_"

int main(void) {
  long size = gethugepagesize();
  printf("default huge page size: %ld B\n\n", size);

  fflush(stdout);
  system(CMD);
  puts("");

  printf("### MMAP\n\n");
  void *addr = mmap(NULL, (size_t)size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
  if (addr == MAP_FAILED) {
    perror("mmap");
    return 1;
  }

  fflush(stdout);
  system(CMD);
  puts("");

  /* cf. demand paging */
  printf("### WRITE DATA\n\n");
  *(int *)addr = 0;

  fflush(stdout);
  system(CMD);
  puts("");

  printf("### MUNMAP\n\n");
  int ret = munmap(addr, size);
  if (ret) {
    perror("munmap");
    return ret;
  }

  fflush(stdout);
  system(CMD);

  return 0;
}
