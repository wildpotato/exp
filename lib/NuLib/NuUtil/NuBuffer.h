#include "NuCommon.h"
#include "NuUtil.h"

#ifndef _NUBUFFER_H
#define _NUBUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void* NuBufferNode_t;

typedef void *(*BufferAllocFn)(void *Allocator, size_t AllocSize);
typedef void (*BufferFreeFn)(void *AllocHdr);
typedef void *(*BufferGetFn)(void *AllocHdr);
typedef void (*BufferSetFn)(void *Allocator, void *AllocatorArgu);

typedef struct _NuBufferAllocType_t
{
    BufferAllocFn       Alloc;
    BufferFreeFn        Free;
    BufferGetFn         Get;
    BufferSetFn         Set;
} NuBufferAllocType_t;

typedef struct _NuBuffer_t NuBuffer_t;
typedef struct _NuSubBuffer_t NuSubBuffer_t;

/* Constructor/Destructor. */
int NuBufferNew(NuBuffer_t **pBuf, size_t NodeSize, int Num);
int NuBufferMmapNew(NuBuffer_t **pBuf, size_t NodeSize, int Num, char *MmapPath);
int NuBufferNew2(NuBuffer_t **pBuf, NuBufferAllocType_t *Type, size_t NodeSize, int Num, int AllocRate, void *AllocatorArgu);
void NuBufferFree(NuBuffer_t *pBuf);

/* Get/Put from/to buffer system. */
void *NuBufferGet(NuBuffer_t *pBuf);
void NuBufferPut(NuBuffer_t *pBuf, void *InPut);

/* Configure the buffer system. */
int NuBufferSetPreAllocRate(NuBuffer_t *pBuf, int AllocRate);
int NuBufferForceExpand(NuBuffer_t *pBuf, int ForceAllocNum);

/* Additional function. */
void NuBufferClear(NuBuffer_t *pBuf);
int NuBufferGetAllocNum(NuBuffer_t *pBuf);

/* SubBuffer system function. */
int NuSubBufferNew(NuSubBuffer_t **pSBuf, NuBuffer_t *pBuf);
void *NuSubBufferGet(NuSubBuffer_t *pSBuf);
void NuSubBufferPut(NuSubBuffer_t *pSBuf, void *InPut);
void NuSubBufferTribute(NuSubBuffer_t *pSBuf);
void NuSubBufferFree(NuSubBuffer_t *pSBuf);
int NuSubBufferGetCnt(NuSubBuffer_t *pSBuf);

#ifdef __cplusplus
}
#endif

#endif /* _NUBUFFER_H */

