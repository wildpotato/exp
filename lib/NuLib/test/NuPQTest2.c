
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "NuThread.h"
#include "NuTime.h"
#include "NuPosixQ.h"

#define MAXBUFSZ 1024

void deQ(NuPosixQ_t *pmq, int cnt) {
    int i = 0; 
    ssize_t len = 0;
    char *buf = NULL;
    size_t bufLen = 0;

    //char buf[pmq->maxMsgSize];
    char time[15+1] = {0};

    bufLen = NuPosixQueueAlloc(pmq, &buf);
    if (bufLen <= 0) {
        printf("error : %d [%s]\n", errno, strerror(errno));
        return;
    }

    do
    {
        len = NuPosixQueueReceive(pmq, buf, bufLen);
        if (len < 0) {
            printf("len=%ld, [%.*s]\n", len, (int)len, buf);
            printf("error : %d [%s]\n", errno, strerror(errno));
            break;
        } else if (len == 0){
            printf("continue\n");
        } else {
            printf("len=%ld, [%.*s]\n", len, (int)len, buf);
            printf(" i = %d, len=%ld [%s]\n", i, len, strerror(errno));
        }
        ++i;
    } while(i < cnt);

    NuGetTime(time);

    printf("end [%s]\n", time);

    NuPosixQueueFree(buf);
}

void *_ThdWork(void *Argu)
{
    NuPosixQ_t *pmq = (NuPosixQ_t *)Argu;
    deQ(pmq, 10);

    return NULL;
}


int main(int argc, char **argv)
{
    int iRC = 0;

	NuThread_t thd;
    NuPosixQ_t mq;

    iRC = NuPosixQueueOpen2(&mq, "mq_test", 32, 2048, true);
    //iRC = NuPosixQueueOpen2(&mq, "mq_test", 10, 2048, true);
    if (iRC < 0) {
        printf("queue open fail. rc = %d, errno = %d\n", iRC, errno);
        return 0;
    }
    printf("%s, blkMode = %ld, %ld, %ld\n", mq.Name, mq.blockingMode, mq.maxMsgCnt, mq.maxMsgSize);

    NuThdCreate(&_ThdWork, (void *)&mq, &thd);

    sleep(5);

    NuPosixQueueClose(&mq);

    printf("before join\n");
    NuThdJoin(thd);
    printf("after join\n");

	return 0;
}
