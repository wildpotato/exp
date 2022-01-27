#define _GNU_SOURCE 

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sched.h>

#include "NuCStr.h"
#include "NuThread.h"
#include "NuProcess.h"

#define THREAD_NUM  5

typedef struct args_t {
    int SZ ;
    int CpuID[10];
} args_t;

typedef void (*NuCStrSplitFn)(unsigned int idx, const char *Msg, size_t MsgSize, void *Argu);
void NuCStrSplit(const char *CStr, char Sep, size_t Len, NuCStrSplitFn Fn, void *Argu);

void strSplit(unsigned int idx, const char *Msg, size_t MsgSize, void *Argu) {
    args_t *p = (args_t *)Argu;
    p->CpuID[p->SZ] = NuCStrToInt(Msg, MsgSize);
    p->SZ++;
}

void procBindCore1(char *str) {
    int i = 0;
    args_t args = {
        .SZ = 0
    };
    NuCStrSplit(str, ',', strlen(str), &strSplit, &args);

    for (i = 0; i < args.SZ; i++) {
        printf("%d\n", args.CpuID[i]);
    }

    NuProcessBindCoreWithArray(args.CpuID, args.SZ);
}

void procBindCore2(char *str) {
    NuProcessBindCoreWithString(str);
}

void *_ThdWork(void *Argu)
{
    while(1) {
        sleep(1);
        printf(" 1 cpu = %d, %ld\n", sched_getcpu(), NuThdSelf());
    }
	return NULL;
}

void *_ThdWork2(void *Argu)
{
    NuThdBindCoreWithString("13,14,15");
    while(1) {
        sleep(1);
        printf(" 2 cpu = %d, %ld\n", sched_getcpu(), NuThdSelf());
    }
	return NULL;
}

int main()
{
    int i = 0;
	NuThread_t thd[THREAD_NUM];

//    procBindCore1("3,4,5");
    procBindCore2("3,4");

	for(i = 0; i < THREAD_NUM; i++)
	{
        if (i >=3) {
            NuThdCreate(&_ThdWork2, NULL, &thd[i]);
        } else {
            NuThdCreate(&_ThdWork, NULL, &thd[i]);
        }
	}

    while(1) {
        sleep(1);
        printf(" cpu = %d, main\n", sched_getcpu());
    }
	return 0;
}
