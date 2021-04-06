#include "stdio.h"

#define numOfRecords             2
#define lenOfRecord             18
const char rawData[numOfRecords][lenOfRecord] =
{
                    {"F123456789M0800720"},
                    {"F987654321F0740310"},
};

int main(int argc, char **argv) {
    for (int i = 0; i < numOfRecords; ++i) {
        for (int j = 0; j < lenOfRecord; ++j) {
            printf("%c", rawData[i][j]);
        }
        printf("\n");
    }
    return 0;
}
