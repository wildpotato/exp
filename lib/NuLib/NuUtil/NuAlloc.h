
#ifndef _NUALLOC_H
#define _NUALLOC_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void* NuAllocPtr_t;
typedef struct _NuAlloc_t NuAlloc_t;

NuAlloc_t *NuAllocOpen(int item_cnt, size_t init_sz);
void NuAllocClose(NuAlloc_t *alloc);

NuAllocPtr_t *NuAllocGet(NuAlloc_t *alloc, size_t sz);
void NuAllocPut(NuAlloc_t *alloc, NuAllocPtr_t *ptr);

size_t NuAllocPtrGetSize(NuAllocPtr_t *alloc_ptr);
NuAllocPtr_t *NuAllocPtrExtend(NuAllocPtr_t *alloc_ptr, size_t len);



#ifdef __cplusplus
}
#endif

#endif /* _NUALLOCATOR_H */

