

#ifndef _NUASYNC_H
#define _NUASYNC_H

#include "NuCommon.h"
#include "NuUtil.h"
#include "NuTime.h"
#include "NuThread.h"
#include "NuCondVar.h"
#include "NuMutex.h"
#include "NuRingBuffer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _NuAsync_Delegate_t {
	NuEventFn CBFn;
	void      *Argu;
} NuAsync_Delegate_t;

typedef struct _NuAsyncQue_t {
    bool         isOpen;
	NuRBufBase_t *rBuf;
	int          caps;

	NuMutex_t    wLock;
	NuMutex_t    rLock;
    NuCondVar_t  fullBlocking;
    NuCondVar_t  emptyBlocking;
} NuAsyncQue_t;

int  NuAsyncQueNew(NuAsyncQue_t **que, int queCnt);
void NuAsyncQueFree(NuAsyncQue_t *que);
void NuAsyncQueClose(NuAsyncQue_t *que);

int NuAsyncQueEnqueue(NuAsyncQue_t *que, NuAsync_Delegate_t *dlg);
int NuAsyncQueDequeue(NuAsyncQue_t *que, NuAsync_Delegate_t *dlg);

/* --------------------------------------------------------------------- */
typedef struct _NuAsync_t {
	bool          isStop;
	NuAsyncQue_t  *Queue;
	base_vector_t vThds;
} NuAsync_t;

int NuAsyncNew(NuAsync_t **async, int queCnt);
void NuAsyncFree(NuAsync_t *async);

int NuAsyncAddThread(NuAsync_t *async, int cnt);

int NuAsyncStart(NuAsync_t *async);
void NuAsyncStop(NuAsync_t *async);

int NuAsyncInvoke(NuAsync_t *async, NuEventFn fn, void *argu);

//void NuAsyncInvokeWithDelegate(NuAsync_t *async, NuAsync_Delegate_t *dlg);

#ifdef __cplusplus
}
#endif

#endif /* _NUASYNC_H */

