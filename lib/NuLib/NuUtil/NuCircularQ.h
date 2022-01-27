
#ifndef _NUCIRCULARQ_H
#define _NUCIRCULARQ_H

#include "NuCommon.h"
#include "NuUtil.h"
#include "NuMutex.h"
#include "NuCondVar.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _NuCQHdr_t 
{
    size_t capacity;
    int    begin;
    int    end;
} NuCQHdr_t;

typedef struct _NuCQData_t
{
    size_t Len;
    char   Data[1];
} NuCQData_t;

typedef struct _NuCQIdx_t
{
    int    No;
    struct _NuCQData_t *node;
    struct _NuCQIdx_t  *nextIdx;
} NuCQIdx_t;

typedef struct _NuCQ_t 
{
    NuCQHdr_t    hdr;

    NuMutex_t    wLock;
    NuMutex_t    rLock;
    NuCondVar_t  fullBlocking;
    NuCondVar_t  emptyBlocking;
    bool         isOpen;

    NuCQIdx_t    *Idx;
    char         *items;
    char         memory[1];
    
} NuCQ_t;

int NuCQNew(NuCQ_t **cq, size_t itemSz, size_t itemCnt);
void NuCQFree(NuCQ_t *cq);
void NuCQClose(NuCQ_t *cq);

int NuCQEnqueue(NuCQ_t *cq, const char *item, size_t len);
int NuCQDequeue(NuCQ_t *cq, char *out, size_t *len);

NU_ATTR_INLINE 
static inline bool NuCQIsFull(NuCQ_t *cq) {
    return (cq->Idx[cq->hdr.end].nextIdx == &(cq->Idx[cq->hdr.begin])) ? true : false;
}

NU_ATTR_INLINE 
static inline bool NuCQIsEmpty(NuCQ_t *cq) {
    return (cq->Idx[cq->hdr.begin].nextIdx == &(cq->Idx[cq->hdr.end])) ? true : false;
}

#ifdef __cplusplus
}
#endif

#endif /* _NUCIRCULARQ_H */

