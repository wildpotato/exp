
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "NuThread.h"
#include "NuHangerQ.h"
#include "NuTuneTools.h"

#define THREAD_NUM  1
#define DATA_NUM  10 

typedef struct _Argu_t 
{
	int    Quit;
	int    blocking;
	NuHQ_t *Q;
} Argu_t;

int _DeQCB(void *Argu)
{
	Argu_t *pArgu = (Argu_t *)Argu;
	if (pArgu->Quit)
	{
		return -1;
	}

	return 0;
}

void *_ThdWork(void *Argu)
{
	Argu_t *pArgu = (Argu_t *)Argu;
	void *ptr = NULL;
	size_t len = 0;

	while(!(pArgu->Quit) ||
          !NuHQIsEmpty(pArgu->Q))
	{
		if (NuHQDequeueWaitCB(pArgu->Q, &ptr, &len, &_DeQCB, Argu) < 0)
		{
//			printf("break\n");
			break;
		}

//	printf("%ld  recv : [%.*s]\n", NuThdSelf(), (int)len, (char *)ptr);

        NuTuneSetStop();
        printf("%ld sec, %ld nsec\n", NuTuneGetSecElapse(), NuTuneGetNanoSecElapse());
        free(ptr);
	}

	return NULL;
}


int main()
{
	int iRC = 0;
	int i = 0;
	NuThread_t thd[THREAD_NUM];
	Argu_t argu = { 
		            .Quit = 0,
					.Q = NULL
		          };

	NuHQ_t *Que = NULL;
	char *ptr = NULL;
	int len = 0;

	iRC = NuHQNew(&Que);
	assert(iRC == 0);

	argu.Q = Que;

	/* create thread */
	for(i = 0; i < THREAD_NUM; i++)
	{
		NuThdCreate(&_ThdWork, &argu, &thd[i]);
	}

	NuTuneSetStart();
	for(i = 0; i < DATA_NUM; i++)
	{
		ptr = (char *)malloc(sizeof(char) * 128);
		len = sprintf(ptr, "data => no [%d]", i);
		NuHQEnqueue(argu.Q, (void *)ptr, len);
	}

	NuHQWakeDeQ(argu.Q);
	argu.Quit = 1;
	/* join thread */
	for(i = 0; i < THREAD_NUM; i++)
	{
		NuThdJoin(thd[i]);

		printf("join\n");
	}

	NuHQFree(Que);
	return 0;
}
