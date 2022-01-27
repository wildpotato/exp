
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "NuThread.h"
#include "NuMMapQ.h"
#include "NuTuneTools.h"

#define THREAD_NUM  5
#define DATA_NUM  10
#define DATA_LEN  5

typedef struct _Argu_t 
{
	int    Quit;
	int    blocking;
	NuMQ_t *Q;
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
    char szData[DATA_LEN + 1];
	size_t len = 0;

	while(!(pArgu->Quit) ||
          !NuMQIsEmpty(pArgu->Q))
	{
		if (NuMQDequeueWaitCB(pArgu->Q, szData, sizeof(szData), &len, &_DeQCB, Argu) < 0)
		{
			break;
		}

    	printf("%ld  recv : [%.*s]\n", NuThdSelf(), (int)len, (char *)szData);

//        NuTuneSetStop();
//        printf("%ld sec, %ld nsec\n", NuTuneGetSecElapse(), NuTuneGetNanoSecElapse());
	}

	return NULL;
}


int main()
{
	int iRC = 0;
	int i = 0;
    char szData[DATA_LEN+1] = {0};
	NuThread_t thd[THREAD_NUM];
	Argu_t argu = { 
		            .Quit = 0,
					.Q = NULL
		          };

	NuMQ_t *Que = NULL;
	int len = 0;

	iRC = NuMQNew(&Que, 6, 5, "./test.mq");
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
		len = sprintf(szData, "A%04d", i);
		NuMQEnqueue(argu.Q, szData, len);
	}

	NuMQWakeDeQ(argu.Q);
	argu.Quit = 1;
	/* join thread */
	for(i = 0; i < THREAD_NUM; i++)
	{
		NuThdJoin(thd[i]);

		printf("join\n");
	}

    NuMQClear(Que);
	NuMQFree(Que);
	return 0;
}
