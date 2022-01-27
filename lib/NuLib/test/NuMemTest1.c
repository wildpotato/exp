#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "NuMemory.h"


void showMem(NuMemory_t *mem) {
    printf("cap=%ld,len=%ld,[%.*s]\n", NuMemoryCap(mem), NuMemoryLength(mem), (int)NuMemoryLength(mem), NuMemoryPtr(mem));
}

void Test1(NuMemory_t *mem) {
    showMem(mem);
    
    NuMemoryAppend(mem, "test123", 7);
    NuMemoryAppend(mem, "test123", 7);
    NuMemoryAppend(mem, "test123", 7);
    NuMemoryAppend(mem, "test123", 7);

    showMem(mem);
}

void Test2(NuMemory_t *mem) {
    char data[17] = {0};
    int len = 0;
    int i = 0;

    showMem(mem);
   
    for (i = 0; i < 100; i++) {
        len = sprintf(data, "%016d", i);
        NuMemoryAppend(mem, data, len);
    }

    showMem(mem);
}


int main(int argc, char **argv)
{
    int iRC = 0;
    NuMemory_t *mem = NULL;

    iRC = NuMemoryNew(&mem, 16);
    if (iRC < 0) {
        printf("new memory fail. %d\n", iRC);
        return 0;
    }

    /* ------------------------------------------ */
    Test1(mem); 

    NuMemoryClear(mem);

    Test2(mem); 
    /* ------------------------------------------ */
    NuMemoryFree(mem);
	return 0;
}
