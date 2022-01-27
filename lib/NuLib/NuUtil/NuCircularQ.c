
#include "NuCircularQ.h"

/* Queue function */
/* ====================================================================== */
int NuCQNew(NuCQ_t **cq, size_t itemSz, size_t itemCnt)
{
    int           i = 0;
    size_t    memSz = 0;
    size_t    idxSz = 0;
    size_t   dataSz = 0;
    NuCQ_t       *q = NULL;
    NuCQIdx_t *qIdx = NULL;
    char     *pData = NULL;

    idxSz  = sizeof(NuCQIdx_t) * itemCnt;
    dataSz = sizeof(NuCQData_t) + itemSz;
    memSz  = idxSz + (dataSz * itemCnt);

    //printf("idxSz = %ld, dataSz = %ld, memSz = %ld\n", idxSz, dataSz, memSz);
    q = (NuCQ_t *)malloc(sizeof(char) * (sizeof(NuCQ_t) + memSz));
    if (q == NULL) {
        return NU_MALLOC_FAIL;
    }

    qIdx  = (NuCQIdx_t *)(q->memory);
    pData = (char *)(q->memory);
    pData = pData + idxSz;

    for(i = 0; i < itemCnt; i++) {
        qIdx[i].No = i;
        qIdx[i].node = (NuCQData_t *)(pData + (i * dataSz));
        if (i + 1 >= itemCnt) {
            qIdx[i].nextIdx = &qIdx[0];
        } else {
            qIdx[i].nextIdx = &qIdx[i + 1];
        }
    }

    /* init */
    q->isOpen       = true;
    q->Idx          = qIdx;
    q->items        = pData;
    q->hdr.capacity = itemCnt;
    q->hdr.begin    = 0;
    q->hdr.end      = 1;

    NuMutexInit(&(q->wLock));
    NuMutexInit(&(q->rLock));
    NuCondVarInit(&(q->fullBlocking));
    NuCondVarInit(&(q->emptyBlocking));

    (*cq) = q;

    return NU_OK;
}

void NuCQFree(NuCQ_t *cq)
{
    free(cq);
}

void NuCQClose(NuCQ_t *cq) 
{
    cq->isOpen = false;
    NuCondVarWake(&(cq->emptyBlocking));
    NuCondVarWake(&(cq->fullBlocking));
}

int NuCQEnqueue(NuCQ_t *cq, const char *item, size_t len) 
{
    int iRC = NU_OK;
    NuCQData_t *pItem = NULL;

    NuMutexLock(&(cq->wLock));
    //-----------------------------------
    if (cq->isOpen) {
        while(NuCQIsFull(cq)) {
            NuCondVarBlock(&(cq->fullBlocking));
        }
    } else {
        NUGOTO(iRC, NU_FAIL, EXIT);
    }

    //pItem = cq->Idx[cq->hdr.end].nextIdx->node;
    pItem = cq->Idx[cq->hdr.end].node;
    pItem->Len = len;
    memcpy(pItem->Data, item, len);
    //memcpy(pItem->Data, item, 7);

    cq->hdr.end = cq->Idx[cq->hdr.end].nextIdx->No;
    //printf("end = %d\n", cq->hdr.end);

    //-----------------------------------
    NuCondVarWake(&(cq->emptyBlocking));

EXIT:
    NuMutexUnLock(&(cq->wLock));

    return iRC;
}

int NuCQDequeue(NuCQ_t *cq, char *out, size_t *len) 
{
    int iRC = NU_OK;
    NuCQData_t *pItem = NULL;

    NuMutexLock(&(cq->rLock));
    //-----------------------------------
    if (cq->isOpen) {
        while(NuCQIsEmpty(cq)) {
            NuCondVarBlock(&(cq->emptyBlocking));
        }
    } else {
        if (NuCQIsEmpty(cq)) {
            NUGOTO(iRC, NU_FAIL, EXIT);
        }
        printf("deq next\n");
    }
    
    pItem = cq->Idx[cq->hdr.begin].nextIdx->node;

    *len = pItem->Len;
    memcpy(out, pItem->Data, pItem->Len);

    cq->hdr.begin = cq->Idx[cq->hdr.begin].nextIdx->No;
    //-----------------------------------
    NuCondVarWake(&(cq->fullBlocking));

EXIT:
    NuMutexUnLock(&(cq->rLock));

    return iRC;
}
