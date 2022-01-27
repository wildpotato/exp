
#include "NuTuneTools.h"

int main()
{
    char *p = NULL;
    int i = 0;

	NuTuneSetStart();

    for (i = 0; i < 1000000; i++)
    {
        p = (char *)malloc(sizeof(char) * 1024);
        sprintf(p, "%d\n", i);
        free(p);
    }
//	sleep(0);

	NuTuneSetStop();

	printf("%ld sec, %ld nsec\n", NuTuneGetSecElapse(), NuTuneGetNanoSecElapse());

	return 0;
}

