
#include "NuMMapQ.h"

/* Queue function */
/* Private EnQ function, need lock EnQLock by caller. */
static int _EnQ(NuMQ_t *Q, void *Item, size_t Len) {
    NuMQ_Msg_t  *pEnQ = Q->EnQ;

    memcpy(pEnQ->Data, Item, Len);
    pEnQ->DataLen = Len;

    Q->EnQ = pEnQ->Next;
    Q->Hdr->StartIndex = Q->EnQ->Idx;
    NuCondVarWake(Q->QEmptyBlocking);

    return NU_OK;
}

/* Private DeQ function, need lock DeQLock by caller. */
static int _DeQ(NuMQ_t *Q, void *Item, size_t ItemLen, size_t *DataLen) {
    NuMQ_Msg_t  *Tmp = Q->DeQ->Next;

    *DataLen = Tmp->DataLen;
    if (ItemLen < Tmp->DataLen) {
        return NU_FAIL;
    }
    memcpy(Item, Tmp->Data, Tmp->DataLen);

    Q->DeQ = Tmp;
    Q->Hdr->StopIndex = Q->DeQ->Idx;
    NuCondVarWake(Q->QFullBlocking);

    return NU_OK;
}

/* ====================================================================== */
/* Queue function */
/* ====================================================================== */
int NuMQNew(NuMQ_t **Q, int Size, int Cnt, const char *FilePath) {
    int         iRC     = NU_OK;
    int         i       = 0;
    NuMQ_Msg_t *pMsg    = NULL;
    NuMQ_Hdr_t *pHdr    = NULL;
    size_t      mmapSz  = 0;
    size_t      blockSz = 0;
    char       *ptr     = NULL;

    (*Q) = (NuMQ_t *)malloc(sizeof(NuMQ_t));
    if ((*Q) == NULL) {
        return NU_MALLOC_FAIL;
    } 
    memset((*Q), 0x00, sizeof(NuMQ_t));

    blockSz = sizeof(NuMQ_Msg_t) + Size;
    mmapSz = NU_RALIGN( sizeof(char) * ( sizeof(NuMQ_Hdr_t) + 
                                         (blockSz * Cnt)
                                       ), 1024); 

    iRC = NuMMapNew(&(*Q)->MFile, FilePath, "a+", mmapSz, PROT_READ|PROT_WRITE, MAP_SHARED);
    if (iRC != NU_OK) {
        free((*Q));
        return NU_MMAPERR;
    }

    ptr = NuMMapGetAddr((*Q)->MFile);

    (*Q)->Arrays = (NuMQ_Msg_t **)malloc(sizeof(NuMQ_Msg_t *) * Cnt);

    /* initial header 
     * 不需要給 index 初始值, 預設0
     * */
    pHdr = (NuMQ_Hdr_t *)ptr;
    (*Q)->Hdr = pHdr;
    pHdr->Capacity = Cnt;
    pHdr->MaxLen = Size;

    if (pHdr->StartIndex == 0 && pHdr->StopIndex == 0) {
        pHdr->StartIndex = 1;
    }

    /* -------------------- */
    /* initial queue */
    (*Q)->Body = (char *)(ptr + sizeof(NuMQ_Hdr_t));

    ptr = (*Q)->Body;
    for (i = 0; i < Cnt; i++) {
        (*Q)->Arrays[i] = (NuMQ_Msg_t *)(ptr + (blockSz * i));
    }

    for (i = 0; i < Cnt; i++) {
        pMsg = ((*Q)->Arrays[i]);
        pMsg->Idx = i;
        if (i == Cnt - 1) {
            pMsg->Next = ((*Q)->Arrays[0]);
        } else {
            pMsg->Next = ((*Q)->Arrays[i + 1]);
        }
    }

    (*Q)->EnQ = ((*Q)->Arrays[pHdr->StartIndex]);
    (*Q)->DeQ = ((*Q)->Arrays[pHdr->StopIndex]);

    /* -------------------- */

    // create sync variable
    iRC = NuMutexInit(&((*Q)->EnQLock));
    NUCHKRC(iRC, EXIT);

    iRC = NuMutexInit(&((*Q)->DeQLock));
    NUCHKRC(iRC, EXIT);

    iRC = NuCondVarNew(&((*Q)->QFullBlocking));
    NUCHKRC(iRC, EXIT);

    iRC = NuCondVarNew(&((*Q)->QEmptyBlocking));
    NUCHKRC(iRC, EXIT);

EXIT:
    if(iRC < 0) {
        NuMQFree(*Q);
    }

    return iRC;
}

void NuMQFree(NuMQ_t *Q) {
    if(Q != NULL) {
        NuMMapFree(Q->MFile);

        NuMutexDestroy(&(Q->EnQLock));
        NuMutexDestroy(&(Q->DeQLock));
        NuCondVarFree(Q->QFullBlocking);
        NuCondVarFree(Q->QEmptyBlocking);

        free(Q->Arrays);
        free(Q);
    }

    return;
}

int NuMQIsFull(NuMQ_t *Q) {
    return (Q->EnQ == Q->DeQ);
}

int NuMQIsEmpty(NuMQ_t *Q) {
    return (Q->DeQ->Next == Q->EnQ);
}

int NuMQCount(NuMQ_t *Q) {
    int Cnt = 0;
    NuMutexLock(&(Q->EnQLock));
    NuMutexLock(&(Q->DeQLock));

    if (Q->Hdr->StartIndex > Q->Hdr->StopIndex)
    { /* Cnt = Start - (Stop + 1) */
        Cnt = Q->Hdr->StartIndex - Q->Hdr->StopIndex - 1;
    }
    else
    { /* Cnt = Capacity - ((Stop + 1) - Start) */
        Cnt= Q->Hdr->Capacity - Q->Hdr->StopIndex - 1 + Q->Hdr->StartIndex;
    }

    NuMutexUnLock(&(Q->DeQLock));
    NuMutexUnLock(&(Q->EnQLock));
    return Cnt;
}

void NuMQClear(NuMQ_t *Q) {
    NuMutexLock(&(Q->EnQLock));
    NuMutexLock(&(Q->DeQLock));

    Q->EnQ = Q->DeQ->Next;
    Q->Hdr->StartIndex = Q->EnQ->Idx;
    Q->Hdr->StopIndex = Q->DeQ->Idx;

    NuMutexUnLock(&(Q->DeQLock));
    NuMutexUnLock(&(Q->EnQLock));

    return;
}

int NuMQBlockingEnqueue(NuMQ_t *Q, void *Item, size_t Len) {
    if (Len > Q->Hdr->MaxLen) {
        return NU_FAIL;
    }

    NuMutexLock(&(Q->EnQLock));

    while(NuMQIsFull(Q))
    {
        NuCondVarBlock(Q->QFullBlocking);
    }

    _EnQ(Q, Item, Len);

    NuMutexUnLock(&(Q->EnQLock));

    return NU_OK;
}


int NuMQBlockingDequeue(NuMQ_t *Q, void *Item, size_t ItemLen, size_t *DataLen) {
    int iRC = NU_OK;

    NuMutexLock(&(Q->DeQLock));

    while(NuMQIsEmpty(Q))
    {
        NuCondVarBlock(Q->QEmptyBlocking);
    }

    iRC = _DeQ(Q, Item, ItemLen, DataLen);

    NuMutexUnLock(&(Q->DeQLock));

    return iRC;
}

int NuMQEnqueue(NuMQ_t *Q, void *Item, size_t Len) {
    int iRC = NU_OK;

    if (Len > Q->Hdr->MaxLen) {
        return NU_FAIL;
    }

    NuMutexLock(&(Q->EnQLock));
    if (NuMQIsFull(Q)) {
        NUGOTO(iRC, NU_FULL, EXIT);
    }

    _EnQ(Q, Item, Len);
    NuCondVarWake(Q->QEmptyBlocking);
EXIT:
    NuMutexUnLock(&(Q->EnQLock));
    return iRC;
}

int NuMQDequeue(NuMQ_t *Q, void *Item, size_t ItemLen, size_t *DataLen) {
    int iRC = NU_OK;

    NuMutexLock(&(Q->DeQLock));

    if (NuMQIsEmpty(Q)) {
        NUGOTO(iRC, NU_EMPTY, EXIT);
    }
    iRC = _DeQ(Q, Item, ItemLen, DataLen);
EXIT:
    NuMutexUnLock(&(Q->DeQLock));

    return iRC;
}

int NuMQDequeueWaitCB(NuMQ_t *Q, void *Item, size_t ItemLen, size_t *DataLen, DeMQCBFn CBFn, void *Argu) {
    int         iRC = 0;

    NuMutexLock(&(Q->DeQLock));

    while (NuMQIsEmpty(Q)) {
        if ( CBFn(Argu) < 0 ) {
            *DataLen = 0;
            NUGOTO(iRC, NU_FAIL, EXIT);
        }
        NuCondVarBlock(Q->QEmptyBlocking);
    }

    iRC = _DeQ(Q, Item, ItemLen, DataLen);
EXIT:
    NuMutexUnLock(&(Q->DeQLock));
    return iRC;
}

void NuMQWakeDeQ(NuMQ_t *Q) {
    NuCondVarWake(Q->QEmptyBlocking);
    return;
}

void NuMQDumpFull(NuMQ_t *Q, MQDumpCBFn CBFn, void *Argu) {
    int            i = 0;
    NuMQ_Msg_t *pMsg = NULL;
    for (i = 0; i < Q->Hdr->Capacity; i++) {
        pMsg = Q->Arrays[i];
        CBFn(pMsg, Argu);
    }
}

void NuMQDump(NuMQ_t *Q, MQDumpCBFn CBFn, void *Argu) {
    NuMQ_Msg_t *pMsg = NULL;
    pMsg = Q->DeQ;
    while(pMsg->Next != Q->EnQ) {
        pMsg = pMsg->Next;
        CBFn(pMsg, Argu);
    }
}

