#include <sys/time.h>
#include <stdio.h>

/*
 *  struct timeval {
 *      time_t tv_sec;       // used for seconds
 *      suseconds_t tv_usec; // used for microseconds
 *  }
 */

int main() {
    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    printf("seconds: %ld\nmicro seconds: %ld\n",
            current_time.tv_sec, current_time.tv_usec);
    return 0;
}
