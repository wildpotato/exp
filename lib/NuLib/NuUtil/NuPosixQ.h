
#ifndef _NUPOSIXQ_H
#define _NUPOSIXQ_H

#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>
#include <mqueue.h>
#include <time.h>

#include "NuCommon.h"
#include "NuUtil.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _NuPosixQ_t
{
    char   Name[63+1];
    mqd_t  mqHdl;
    long   blockingMode;
    long   maxMsgCnt;
    long   maxMsgSize;
    bool   isQuit;
} NuPosixQ_t;

/* max cnt, size 不能超過 /proc/sys/fs/mqueue/ 下的系統設定 */
int NuPosixQueueOpen(NuPosixQ_t *pmq, const char *name, long maxCnt, long maxSize);
int NuPosixQueueOpen2(NuPosixQ_t *pmq, const char *name, long maxCnt, long maxSize, bool isBlocking);
int NuPosixQueueClose(NuPosixQ_t *pmq);
int NuPosixQueueDel(const char *name);

int NuPosixQueueSendWithPrio(NuPosixQ_t *pmq, const char *msg, size_t msgLen, unsigned prio);
static inline int NuPosixQueueSend(NuPosixQ_t *pmq, const char *msg, size_t msgLen) { 
    return NuPosixQueueSendWithPrio(pmq, msg, msgLen, 0);
}


/* 注意. dequeue 傳入的buffer長度必須足夠存取最大訊息長度
 * 可呼叫 NuPosixQueueAlloc/NuPosixQueueFree 產生 buffer memory./
 * */
static inline size_t NuPosixQueueAlloc(NuPosixQ_t *pmq, char **buf) {
    (*buf) = (char *)malloc(sizeof(char) * pmq->maxMsgSize);
    if ((*buf) == NULL) {
        return -1;
    } 
    memset((*buf), 0x00, pmq->maxMsgSize);
    return pmq->maxMsgSize;
}

static inline void NuPosixQueueFree(char *buf) { 
    free(buf); 
}

/* return get message length */
ssize_t NuPosixQueueReceiveWithPrio(NuPosixQ_t *pmq, char *buf, size_t bufLen, unsigned *prio);

static inline 
ssize_t NuPosixQueueReceive(NuPosixQ_t *pmq, char *buf, size_t bufLen) { 
    unsigned prio = 0;
    return NuPosixQueueReceiveWithPrio(pmq, buf, bufLen, &prio);
}

static inline 
ssize_t NuPosixQueueBusyReceiveWithPrio(NuPosixQ_t *pmq, char *buf, size_t bufLen, unsigned *prio) {
	do {
		ssize_t rc = mq_receive(pmq->mqHdl, buf, bufLen, prio);
		if (rc < 0) {
			switch(errno) 
			{
				case EBADF:
				case EINVAL:
					return NU_FAIL;
				case EAGAIN:
				case ETIMEDOUT:
					continue;
			}
		} else {
			return rc;
		} 
	} while(1);
}

static inline 
ssize_t NuPosixQueueBusyReceive(NuPosixQ_t *pmq, char *buf, size_t bufLen) { 
    unsigned prio = 0;
    return NuPosixQueueBusyReceiveWithPrio(pmq, buf, bufLen, &prio);
}

#ifdef __cplusplus
}
#endif

#endif /* _NUPOSIXQ_H */

