#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NuCommon.h"
#include "NuUtil.h"
#include "NuAlloc.h"

void StringTest(NuAlloc_t *alloc)
{
	int i = 0, j = 0;;
	NuAllocPtr_t *ptr = NULL;
	char *str = NULL;
	char buf[2] = {0};

	for (i = 10; i < 100; i++)
	{
		buf[0] = 'A';
		ptr = NuAllocGet(alloc, i + 1);
		str = (char *)ptr;
		*str = '\0';

		for (j = 0; j < i; j++)
		{
			strcat(str, buf);
		}

		printf("%s\n", str);

		NuAllocPut(alloc, ptr);
	}


}

void StringTest2(NuAlloc_t *alloc)
{
	int i = 0, j = 0;;
	NuAllocPtr_t *ptr = NULL;
	char *str = NULL;
	char *buf = "B";
	base_vector_t *vec = NULL;

	base_vector_new(&vec, 16);

	for (i = 10; i < 100; i++)
	{
		ptr = NuAllocGet(alloc, i + 1);
		str = (char *)ptr;
		*str = '\0';

		for (j = 0; j < i; j++)
		{
			strcat(str, buf);
		}

		printf("%s\n", str);

		base_vector_push(vec, (void *)ptr);

	}

	while(base_vector_pop(vec, (void **)&ptr) != NU_EMPTY)
	{
		NuAllocPut(alloc, ptr);
	}

	base_vector_free(vec);
}


int main()
{
	NuAlloc_t *alloc;

	alloc = NuAllocOpen(16, 16);

	StringTest(alloc);
	StringTest2(alloc);

	NuAllocClose(alloc);

	return 0;	
}

