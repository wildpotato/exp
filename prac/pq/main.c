#include <stdio.h>
#include <time.h>   // clock_t, clock()
#include <stdlib.h> // rand()

const int SIZE_OF_N = 7;
const int NUM_ELEMENTS[] = {100, 500, 1000, 2000, 3000, 4000, 5000}; // n
const int NUM_OF_OPERATIONS = 5000;                                  // m


#define DELETE_MIN  0
#define INSERT_MIN  1

void test() {
    int i = 0;
    clock_t clock_start = clock();
    long double time_elapsed = 0.0;
    int OPERATIONS[NUM_OF_OPERATIONS];
    for ( ;i < NUM_OF_OPERATIONS; ++i) {
        OPERATIONS[i] = rand() % 2;
        printf("%d ", NUM_ELEMENTS[rand() % SIZE_OF_N]);
    }
    for ( i = 0; i < NUM_OF_OPERATIONS; ++i) {
        printf("%d", OPERATIONS[i]);
    }
    printf("\n");
    clock_t clock_end = clock();
    time_elapsed = ((long double)(clock_end - clock_start)) / CLOCKS_PER_SEC;
    printf("time elapsed: %Lf\n", time_elapsed);
}

int main() {
    test();
    return 0;
}
