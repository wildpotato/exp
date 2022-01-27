
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "NuTime.h"
#include "NuPosixQ.h"

#define MAXBUFSZ 1024

void deQ(NuPosixQ_t *pmq, int cnt) {
    int i = 0; 
    size_t len = 0;
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
        //len = NuPosixQueueReceive(pmq, buf, bufLen);
        len = NuPosixQueueBusyReceive(pmq, buf, bufLen);
        if (len < 0) {
            printf("error : %d [%s]\n", errno, strerror(errno));
        } else {
            //printf("len=%ld, [%.*s]\n", len, (int)len, buf);
            //printf(" i = %d, len=%ld\n", i, len);
        }
        ++i;
    } while(i < cnt);

    NuGetTime(time);

    printf("end [%s]\n", time);

    NuPosixQueueFree(buf);
}

void enQ(NuPosixQ_t *pmq, int cnt) {
    int i = 0; 
	int len = 0;
    char buf[MAXBUFSZ] = {0};
    char time[15+1] = {0};

    memset(buf, '#', sizeof(buf));

    NuGetTime(time);
    printf("start [%s]\n", time);
    for (i = 0; i < cnt; i++) {
        len = NuPosixQueueSend(pmq, buf, 512);
        printf("i = %d, send = %d\n", i, len);
        //printf("sned = %ld\n", len);
    }
    NuGetTime(time);
    printf("start [%s] end\n", time);

}

int main(int argc, char **argv)
{
    int iRC = 0;
    int i = 0;
    int cnt = 100;

    int action = 0; /* deQ = 0, enQ = 1 */

    for (i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "--cnt")) {
			++i;
            cnt = atoi(argv[i]);
        } else if (!strcmp(argv[i], "--act")) {
            ++i;
            if (!strcmp(argv[i], "enQ")) {
                action = 1;
            }
        }
    }

    printf("cnt = %d, act = %d\n", cnt, action);
    // --------------------------------------------------
    NuPosixQ_t mq;

    iRC = NuPosixQueueOpen2(&mq, "mq_test", 32, 2048, false);
    //iRC = NuPosixQueueOpen2(&mq, "mq_test", 10, 2048, true);
    if (iRC < 0) {
        printf("queue open fail. rc = %d, errno = %d\n", iRC, errno);
        return 0;
    }
    printf("%s, blkMode = %ld, %ld, %ld\n", mq.Name, mq.blockingMode, mq.maxMsgCnt, mq.maxMsgSize);

    switch(action) {
        case 0:
            deQ(&mq, cnt);
            break;
        case 1:
            enQ(&mq, cnt);
            break;
        default:
            break;
    }


    NuPosixQueueClose(&mq);

	return 0;
}
