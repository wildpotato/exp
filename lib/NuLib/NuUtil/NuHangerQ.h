
#include "NuCommon.h"
#include "NuMutex.h"
#include "NuCondVar.h"

#ifndef _NUHQ_H
#define _NUHQ_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _NuHQ_t NuHQ_t;

/* Initiate and terminate functions. */
int NuHQNew(NuHQ_t **Q);
int NuHQNew2(NuHQ_t **Q, int HQNo);

void NuHQFree(NuHQ_t *Q);

int NuHQEnqueue(NuHQ_t *Q, void *Item, size_t Len);
void *NuHQDequeueItem(NuHQ_t *Q);

int NuHQDequeue(NuHQ_t *Q, void **Item, size_t *Len);

/* DeQCBFn  return < 0 exit, else continue */
typedef int (*DeQCBFn)(void *);
int NuHQDequeueWaitCB(NuHQ_t *Q, void **Item, size_t *Len, DeQCBFn CBFn, void *Argu);
void NuHQWakeDeQ(NuHQ_t *Q);

/* Clear the queue. */
void NuHQClear(NuHQ_t *Q);

int NuHQIsFull(NuHQ_t *Q);
int NuHQIsEmpty(NuHQ_t *Q);


#ifdef __cplusplus
}
#endif

#endif /* _NUHQ_H */

