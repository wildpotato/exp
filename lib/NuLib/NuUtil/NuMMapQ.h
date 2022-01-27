
#include "NuCommon.h"
#include "NuMMap.h"
#include "NuMutex.h"
#include "NuCondVar.h"

#ifndef  _NUMMAPQ_H_
#define  _NUMMAPQ_H_

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct _NuMQ_Msg_t NuMQ_Msg_t;
struct _NuMQ_Msg_t {
    struct _NuMQ_Msg_t  *Next;
    size_t               Idx;
    size_t               DataLen;
    char                 Data[1];
};

typedef struct _NuMQ_Hdr_t {
    size_t StartIndex;
    size_t StopIndex;
    size_t Capacity;
    size_t MaxLen;
} NuMQ_Hdr_t;

typedef struct _NuMQ_t {
    NuMQ_Msg_t  *EnQ;
    NuMQ_Msg_t  *DeQ;
    NuMutex_t    EnQLock;
    NuMutex_t    DeQLock;
    NuCondVar_t *QFullBlocking;
    NuCondVar_t *QEmptyBlocking;

    NuMQ_Hdr_t  *Hdr;
    char        *Body;
    NuMQ_Msg_t  **Arrays;
    NuMMap_t    *MFile;
} NuMQ_t;

/* Initiate and terminate functions. */
int NuMQNew(NuMQ_t **Q, int Size, int Cnt, const char *FilePath);
void NuMQFree(NuMQ_t *Q);

int NuMQBlockingEnqueue(NuMQ_t *Q, void *Item, size_t Len);
int NuMQBlockingDequeue(NuMQ_t *Q, void *Item, size_t ItemLen, size_t *DataLen);

int NuMQEnqueue(NuMQ_t *Q, void *Item, size_t Len);
int NuMQDequeue(NuMQ_t *Q, void *Item, size_t ItemLen, size_t *DataLen);

/* DeQCBFn  return < 0 exit, else continue */
typedef int (*DeMQCBFn)(void *);
int NuMQDequeueWaitCB(NuMQ_t *Q, void *Item, size_t ItemLen, size_t *DataLen, DeMQCBFn CBFn, void *Argu);
void NuMQWakeDeQ(NuMQ_t *Q);

/* Clear the queue. */
void NuMQClear(NuMQ_t *Q);

int NuMQIsFull(NuMQ_t *Q);
int NuMQIsEmpty(NuMQ_t *Q);
int NuMQCount(NuMQ_t *Q);

typedef int (*MQDumpCBFn)(NuMQ_Msg_t *, void *);
void NuMQDumpFull(NuMQ_t *Q, MQDumpCBFn CBFn, void *Argu);
void NuMQDump(NuMQ_t *Q, MQDumpCBFn CBFn, void *Argu);

#ifdef __cplusplus
}
#endif

#endif /* _NUMMAPQ_H */

