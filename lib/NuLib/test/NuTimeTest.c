#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "NuTime.h"

int main(int argc, char **argv)
{
	char data[32];
	NuDateTime_t *obj = NULL;

    NuDateTimeNew(&obj, false);
	
	NuDateTimeSet(obj);

    NuDateTimeGet4(obj, data);

	printf("[%s]\n", data);

    NuDateTimeGet6(obj, data);
	printf("[%s]\n", data);

    NuDateTimeGet5(obj, data);
	printf("[%s]\n", data);

    NuDateTimeGet3(obj, data);
	printf("[%s]\n", data);

    NuGetTime(data);
    printf("NuGetTime [%s]\n", data);

	return 0;
}
