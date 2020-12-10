#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>

/*
 *  struct timeval {
 *      time_t tv_sec;       // used for seconds
 *      suseconds_t tv_usec; // used for microseconds
 *  }
 */

int main() {
    struct timeval current_time;
    int iter = 1000000, i = 0;
    for (; i < iter; ++i) {
        gettimeofday(&current_time, NULL);
        printf("%010ld%06ld\n",
            current_time.tv_sec, current_time.tv_usec);
        usleep(1);
    }
    return 0;
}
