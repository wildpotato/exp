#define _GNU_SOURCE

#include <assert.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "NuCommon.h"


#define OPEN  1 << 0
#define CLOSE 1 << 1
#define START 1 << 2
#define END   1 << 3

int main()
{
	int i = 0; 
	NuBITSET(i, OPEN);
	printf("%d\n", i);
	
	NuBITSET(i, CLOSE);
	printf("%d\n", i);

	NuBITSET(i, START);
	printf("%d\n", i);

	NuBITSET(i, END);
	printf("%d\n", i);

	NuBITUNSET(i, START);
	printf("%d\n", i);


	if (NuBITCHK(i, END))
	{
		printf("true\n");
	}
	else
	{
		printf("false\n");
	}

	NuBITUNSET(i, END);
	printf("%d\n", i);
	if (NuBITCHK(i, END))
	{
		printf("true\n");
	}
	else
	{
		printf("false\n");
	}

	return 0;
}

