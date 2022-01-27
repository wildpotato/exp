
#include "NuCommon.h"

#ifndef  _NUIPCQ_H_
#define  _NUIPCQ_H_

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct _NuIPCQ_t NuIPCQ_t;

typedef struct _NuIPCQMsgHdr_t
{
    long    MsgType;
} NuIPCQMsgHdr_t;

/* Create an IPC Queue instance. */
int NuIPCQNew(NuIPCQ_t **Q);

/* Attach/Detach an IPC queue, this will not create queue */
int NuIPCQAttach(NuIPCQ_t *Q, key_t *Key);
int NuIPCQDetach(NuIPCQ_t *Q);

/* Create/Delete an IPC queue, this will create queue if key not exist */
int NuIPCQCreate(NuIPCQ_t *Q, key_t *Key);
int NuIPCQDelete(NuIPCQ_t *Q);
/* Create/Attach an IPC queue only for forked processes using. */
int NuIPCQCreatePrivate(NuIPCQ_t *Q);

/* EnQueue. */
int NuIPCQEnqueue(NuIPCQ_t *Q, NuIPCQMsgHdr_t *Msg, size_t DataLen);
int NuIPCQBlockingEnqueue(NuIPCQ_t *Q, NuIPCQMsgHdr_t *Msg, size_t DataLen);

/* DeQueue. */
int NuIPCQDequeue(NuIPCQ_t *Q, NuIPCQMsgHdr_t *Msg, size_t DataLen);
int NuIPCQBlockingDequeue(NuIPCQ_t *Q, NuIPCQMsgHdr_t *Msg, size_t DataLen);

/* Free the IPC Queue instance. */
void NuIPCQFree(NuIPCQ_t *Q);

#ifdef __cplusplus
}
#endif

#endif /* _NUIPCQ_H */

