
#include "NuHangerQ.h"

/* Queue function */
/* ====================================================================== */
#define NuHQ_DEFAULT_NUM    32

//typedef struct _NuHQ_Msg_t NuHQ_Msg_t;
typedef struct _NuHQ_Msg_t
{
    void                *Item;
    size_t              DataLen;
    struct _NuHQ_Msg_t  *Next;
} NuHQ_Msg_t;

struct _NuHQ_t
{
    NuHQ_Msg_t      *EnQ;
    NuHQ_Msg_t      *DeQ;
    NuMutex_t       EnQLock;
    NuMutex_t       DeQLock;
    NuCondVar_t     *QFullBlocking;
    NuCondVar_t     *QEmptyBlocking;
    NuHQ_Msg_t      Msg[1];
};

/* Queue function */
/* ====================================================================== */
int NuHQNew(NuHQ_t **Q)
{
    return NuHQNew2(Q, NuHQ_DEFAULT_NUM);
}

int NuHQNew2(NuHQ_t **Q, int HQNo)
{
    int         iRC = NU_OK;
    int         Cnt = 0;
    NuHQ_Msg_t  *pMsg = NULL;

    if(!((*Q) = (NuHQ_t *)malloc(sizeof(NuHQ_t) + sizeof(NuHQ_Msg_t) * (HQNo))))
    {
        return NU_MALLOC_FAIL;
    }

    pMsg = (*Q)->Msg;
    (*Q)->EnQ = pMsg;

    for(Cnt = 0; Cnt < HQNo; ++ Cnt)
    {
        pMsg->Item = NULL;
        pMsg->DataLen = 0;
        pMsg->Next = pMsg + 1;
        ++ pMsg;
    }

    pMsg->Item = NULL;
    pMsg->DataLen = 0;
    pMsg->Next = (*Q)->EnQ;

    (*Q)->DeQ = pMsg;

    iRC = NuMutexInit(&((*Q)->EnQLock));
    NUCHKRC(iRC, EXIT);

    iRC = NuMutexInit(&((*Q)->DeQLock));
    NUCHKRC(iRC, EXIT);

    iRC = NuCondVarNew(&((*Q)->QFullBlocking));
    NUCHKRC(iRC, EXIT);

    iRC = NuCondVarNew(&((*Q)->QEmptyBlocking));
    NUCHKRC(iRC, EXIT);

EXIT:
    if(iRC < 0)
    {
        NuHQFree(*Q);
    }

    return iRC;
}

void NuHQFree(NuHQ_t *Q)
{
    if(Q != NULL)
    {
        NuMutexDestroy(&(Q->EnQLock));
        NuMutexDestroy(&(Q->DeQLock));
        NuCondVarFree(Q->QFullBlocking);
        NuCondVarFree(Q->QEmptyBlocking);

        free(Q);
    }

    return;
}

int NuHQEnqueue(NuHQ_t *Q, void *Item, size_t Len)
{
    NuHQ_Msg_t  *pEnQ = NULL;

    NuMutexLock(&(Q->EnQLock));

    while(NuHQIsFull(Q))
    {
        NuCondVarBlock(Q->QFullBlocking);
    }

    pEnQ = Q->EnQ;

    pEnQ->Item = Item;
    pEnQ->DataLen = Len;

    Q->EnQ = pEnQ->Next;

    NuCondVarWake(Q->QEmptyBlocking);

    NuMutexUnLock(&(Q->EnQLock));

    return NU_OK;
}

void *NuHQDequeueItem(NuHQ_t *Q)
{
    void        *ptr = NULL;
    size_t      Len = 0;

    NuHQDequeue(Q, &ptr, &Len);
    return ptr;
}

int NuHQDequeue(NuHQ_t *Q, void **Item, size_t *Len)
{
    NuHQ_Msg_t  *Tmp = NULL;

    NuMutexLock(&(Q->DeQLock));

    while(NuHQIsEmpty(Q))
    {
        NuCondVarBlock(Q->QEmptyBlocking);
    }

    Tmp = Q->DeQ->Next;
    *Len = Tmp->DataLen;
    *Item = Tmp->Item;

    Q->DeQ = Tmp;

    NuCondVarWake(Q->QFullBlocking);

    NuMutexUnLock(&(Q->DeQLock));

    return NU_OK;
}

int NuHQDequeueWaitCB(NuHQ_t *Q, void **Item, size_t *Len, DeQCBFn CBFn, void *Argu)
{
	int         iRC = 0;
    NuHQ_Msg_t  *Tmp = NULL;

    NuMutexLock(&(Q->DeQLock));

    while(NuHQIsEmpty(Q))
    {
		if ( CBFn(Argu) < 0 )
		{
			*Len = 0;
			*Item = NULL;
			NUGOTO(iRC, NU_FAIL, EXIT);
		}
        NuCondVarBlock(Q->QEmptyBlocking);
    }

    Tmp = Q->DeQ->Next;
    *Len = Tmp->DataLen;
    *Item = Tmp->Item;

    Q->DeQ = Tmp;

    NuCondVarWake(Q->QFullBlocking);
	iRC = NU_OK;

EXIT:
    NuMutexUnLock(&(Q->DeQLock));
    return iRC;
}

void NuHQWakeDeQ(NuHQ_t *Q)
{
    NuCondVarWake(Q->QEmptyBlocking);
	return;
}

void NuHQClear(NuHQ_t *Q)
{
    NuMutexLock(&(Q->EnQLock));
    NuMutexLock(&(Q->DeQLock));

    Q->EnQ = Q->DeQ->Next;

    NuMutexUnLock(&(Q->DeQLock));
    NuMutexUnLock(&(Q->EnQLock));

    return;
}

int NuHQIsFull(NuHQ_t *Q)
{
	return (Q->EnQ == Q->DeQ);
}

int NuHQIsEmpty(NuHQ_t *Q)
{
	return (Q->DeQ->Next == Q->EnQ);
}

