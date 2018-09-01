#include <stdio.h>
#include <sys/time.h> /* struct timeval, gettimeofday() */
#include <time.h>     /* struct tm, localtime() */

int main(void) {
  struct timeval tv;
  struct tm *tm_ptr;

  const char *week[] = {
    "Sun",
    "Mon",
    "Tue",
    "Wed",
    "Thu",
    "Fri",
    "Sat"
  };

  int ret = gettimeofday(&tv, NULL);
  if (ret != 0)
    return ret;

  tm_ptr = localtime(&tv.tv_sec); /* tv_sec: seconds since Jan. 1, 1970 */

  printf("%4d/%02d/%02d %s %02d:%02d:%02d %s\n",
         (tm_ptr->tm_year) + 1900, tm_ptr->tm_mon + 1, tm_ptr->tm_mday,
         week[tm_ptr->tm_wday], tm_ptr->tm_hour, tm_ptr->tm_min,
         tm_ptr->tm_sec, tm_ptr->tm_zone);
  printf("daylight saving time(summer time): %s\n",
         tm_ptr->tm_isdst > 0 ? "in effect" :
         tm_ptr->tm_isdst == 0 ? "not in effect" : "the information is not available");
}

