#include <assert.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <NuCStr.h>

typedef struct _data_t
{
    int No;
    char Data[32];
    float Price;
} data_t;

int main()
{
    data_t data;
    int data_len = sizeof(data_t);
    char Tmp[1024];

    memset(&data, 0x00, data_len);

    data.No = 1283456;
    strcpy(data.Data, "this is test data\n");
    data.Price = 123.456;

    NuHexDumpStr ((void *)&data, data_len, Tmp, sizeof(Tmp));

    printf("\n%s\n", Tmp);

    return 0; 
}

