#include <assert.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "NuMMap.h"


int main(int argc, char **argv)
{
    int rc = 0;
    NuMMap_t *mmap = NULL;
    char *fpath = NULL;
    void *ptr = NULL;
    if (argc < 2)
    {
        printf("please input mmap file path at parameter\n");
        return 0;
    }
    fpath = argv[1];

    rc = NuMMapNew(&mmap, fpath,
            //"/home/antoms/workspace/go/src/goUtil/Util/example/test.mmap", 
            "a+", 1024, PROT_READ|PROT_WRITE, MAP_SHARED);

    printf("rc = %d\n", rc);

    rc = NuMMapGet(mmap, 1024, &ptr);
    if (rc == 0)
    {
        while(true)
        {
            printf("[%s]\n", ((char *)ptr));
            sleep(1);
        }
    }
    printf("rc = %d\n", rc);

    //NuMMapFree(mmap);
	return 0;
}

