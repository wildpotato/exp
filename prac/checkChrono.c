#include "stdio.h"
#include "stdbool.h"

int main(void) {
    long long int curr = 0, prev = 0;
    int iter = 1000000, i = 0;
    bool is_chrono = true;
    for (;i < iter; ++i) {
        scanf("%lld", &curr);
        if (curr - prev <= 0) {
            is_chrono = false;
            printf("Error at line %d\n", i);
        }
        prev = curr;
    }
    if (is_chrono) {
        printf("Is chronological!\n");
    }
    return 0;
}
