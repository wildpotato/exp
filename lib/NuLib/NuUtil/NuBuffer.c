#include "NuBuffer.h"
#include "NuBufferAdapter.c"

#define NuBufferNodeSize            (sizeof(NuBufferNode_t))
#define NuBufferDefaultAllocRate    100

struct _NuBuffer_t
{
    base_vector_t       AllocVec;
    int                 AllocRate;
    size_t              NodeSize;
    NuBufferNode_t      *Buffer;
    void                *Allocator;
    NuBufferAllocType_t *Type;
};

struct _NuSubBuffer_t
{
    int             Cnt;
    NuBuffer_t      *pParent;
    NuBufferNode_t  *pHead;
    NuBufferNode_t  *pTail;
};

/* Internal functions ====================================================== */

static void _NuBufferRebuild(NuBuffer_t *pBuf, void *pAllocHdr)
{
    int                 Num = pBuf->AllocRate;
    size_t              Size = pBuf->NodeSize;
    NuBufferNode_t      *pBufferNode = NULL, *pFirstNode = NULL;

    pBufferNode = pFirstNode = (pBuf->Type->Get)(pAllocHdr);

    while(-- Num)
    {
        *pBufferNode = (NuBufferNode_t *)((char *)pBufferNode + Size);
        pBufferNode = *pBufferNode;
    }

    *pBufferNode = pBuf->Buffer;
    pBuf->Buffer = pFirstNode;

    return;
}

static int _NuBufferExpand(NuBuffer_t *pBuf)
{
    int         Num = pBuf->AllocRate;
    size_t      Size = (pBuf->NodeSize) * Num;
    void        *pAllocHdr = (pBuf->Type->Alloc)(pBuf->Allocator, Size);

    if(!pAllocHdr)
    {
        return NU_MALLOC_FAIL;
    }

    base_vector_push(&(pBuf->AllocVec), pAllocHdr);

    _NuBufferRebuild(pBuf, pAllocHdr);

    return NU_OK;
}

/* Constructor/Destructor ================================================== */

int NuBufferNew(NuBuffer_t **pBuf, size_t NodeSize, int Num)
{
    return NuBufferNew2(pBuf, &NuBufferMalloc, NodeSize, Num, NuBufferDefaultAllocRate, NULL);
}

int NuBufferMmapNew(NuBuffer_t **pBuf, size_t NodeSize, int Num, char *MmapPath)
{
    return NuBufferNew2(pBuf, &NuBufferMmap, NodeSize, Num, Num, MmapPath);
}

int NuBufferNew2(NuBuffer_t **pBuf, NuBufferAllocType_t *Type, size_t NodeSize, int Num, int AllocRate, void *AllocatorArgu)
{
    int                     PageNum = Num / AllocRate + 1;

    if(NodeSize == 0 || Num == 0)
    {
        return NU_PARAMERROR;
    }

    if(!((*pBuf) = (NuBuffer_t *)malloc(sizeof(NuBuffer_t))))
    {
        return NU_MALLOC_FAIL;
    }

    (*pBuf)->Type = Type;

    if(!AllocatorArgu)
    {
        (*pBuf)->Allocator = NULL;
    }
    else
    {
        (*pBuf)->Allocator = malloc(strlen((char *)AllocatorArgu) + 10);
    }

    Type->Set((*pBuf)->Allocator, AllocatorArgu);

    base_vector_init(&((*pBuf)->AllocVec), PageNum);
    (*pBuf)->Buffer = NULL;
    (*pBuf)->AllocRate = AllocRate;
    (*pBuf)->NodeSize = NuMax(NodeSize, NuBufferNodeSize);

    while(PageNum --)
    {
        _NuBufferExpand(*pBuf);
    }

    return NU_OK;
}

void NuBufferFree(NuBuffer_t *pBuf)
{
    void            *pAllocHdr = NULL;
    base_vector_t   *pVec = &(pBuf->AllocVec);

    while(base_vector_pop(pVec, &pAllocHdr) != NU_EMPTY)
    {
        (pBuf->Type->Free)(pAllocHdr);
    }

    base_vector_destroy(pVec);

    if(pBuf->Allocator)
    {
        free(pBuf->Allocator);
    }

    free(pBuf);

    return;
}

/* Get/Put from/to buffer system =========================================== */
void *NuBufferGet(NuBuffer_t *pBuf)
{
    void    *ptr = NULL;

    if(!(pBuf->Buffer))
    {
        while(_NuBufferExpand(pBuf) < 0)
        {
            pBuf->AllocRate /= 2;
            if(pBuf->AllocRate < 1)
            {
                return NULL;
            }
        }
    }

    ptr = pBuf->Buffer;
    pBuf->Buffer = *(pBuf->Buffer);

    return ptr;
}

void NuBufferPut(NuBuffer_t *pBuf, void *InPut)
{
    NuBufferNode_t  *pBufferNode = InPut;

    *pBufferNode = pBuf->Buffer;
    pBuf->Buffer = pBufferNode;

    return;
}

/* Configure the buffer system ============================================= */

int NuBufferSetAllocRate(NuBuffer_t *pBuf, int AllocRate)
{
    if(AllocRate > 0)
    {
        pBuf->AllocRate = AllocRate;
    }

    return NU_OK;
}

int NuBufferForceExpand(NuBuffer_t *pBuf, int ForceAllocNum)
{
    if(ForceAllocNum < 0)
    {
        return NU_PARAMERROR;
    }

    ForceAllocNum /= (pBuf->AllocRate);
    ++ ForceAllocNum;

    while(ForceAllocNum --)
    {
        _NuBufferExpand(pBuf);
    }

    return NU_OK;
}

/* Additional function ===================================================== */

void NuBufferClear(NuBuffer_t *pBuf)
{
    base_vector_it  VecIt;

    base_vector_it_set(VecIt, &(pBuf->AllocVec));
    pBuf->Buffer = NULL;

    while(VecIt != base_vector_it_end(&(pBuf->AllocVec)))
    {
        _NuBufferRebuild(pBuf, *VecIt);
        ++ VecIt;
    }

    return;
}

int NuBufferGetAllocNum(NuBuffer_t *pBuf)
{
    return pBuf->AllocRate * base_vector_get_cnt(&(pBuf->AllocVec));
}

/* SubBuffer system function =============================================== */

int NuSubBufferNew(NuSubBuffer_t **pSBuf, NuBuffer_t *pBuf)
{
    if(!((*pSBuf) = (NuSubBuffer_t *)malloc(sizeof(NuSubBuffer_t))))
    {
        return NU_MALLOC_FAIL;
    }

    (*pSBuf)->Cnt = 0;
    (*pSBuf)->pParent = pBuf;
    (*pSBuf)->pHead = (*pSBuf)->pTail = NULL;

    return NU_OK;
}

void *NuSubBufferGet(NuSubBuffer_t *pSBuf)
{
    NuBufferNode_t  *pBufferNode = pSBuf->pHead;

    if(!(pSBuf->Cnt))
    {
        return NULL;
    }

    pSBuf->pHead = *pBufferNode;

    if(!(-- (pSBuf->Cnt)))
    {
        pSBuf->pTail = NULL;
    }

    return pBufferNode;
}

void NuSubBufferPut(NuSubBuffer_t *pSBuf, void *InPut)
{
    NuBufferNode_t  *pBufferNode = (NuBufferNode_t *)InPut;

    if(!(pSBuf->Cnt))
    {
        pSBuf->pHead = pSBuf->pTail = pBufferNode;
    }
    else
    {
        *(pSBuf->pTail) = pBufferNode;
        pSBuf->pTail = pBufferNode;
    }

    ++ (pSBuf->Cnt);

    return;
}

void NuSubBufferTribute(NuSubBuffer_t *pSBuf)
{
    NuBuffer_t  *pBuf = pSBuf->pParent;

    if(!(pSBuf->pTail))
    {
        return;
    }

    *(pSBuf->pTail) = pBuf->Buffer;
    pBuf->Buffer = pSBuf->pHead;

    pSBuf->pTail = pSBuf->pHead = NULL;

    (pSBuf->Cnt) = 0;

    return;
}

void NuSubBufferFree(NuSubBuffer_t *pSBuf)
{
    free(pSBuf);

    return;
}

int NuSubBufferGetCnt(NuSubBuffer_t *pSBuf)
{
    return pSBuf->Cnt;
}

