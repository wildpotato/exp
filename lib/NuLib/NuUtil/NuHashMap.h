
#include "NuHash.h"
#include "NuLock.h"

#ifndef _NUHASHMAP_H
#define _NUHASHMAP_H

#ifdef __cplusplus
extern "C" {
#endif


typedef struct _NuHashMap_t NuHashMap_t;

int NuHashMapNew(NuHashMap_t **pMap, int Num);
int NuHashMapNew2(NuHashMap_t **pMap, int Num,  NuHashFn Hash_fn, 
                                                NuHashCompareFn Compare_fn,
                                                NuDestructor FreeKey_fn,
                                                NuDestructor FreeValue_fn);
int NuHashMapNew3(NuHashMap_t **pMap, int Num, size_t klen, size_t vlen, NuHashFn Hash_fn, 
                                                                       NuHashCompareFn Compare_fn,
                                                                       NuDestructor FreeKey_fn,
                                                                       NuDestructor FreeValue_fn);

void NuHashMapFree(NuHashMap_t *Map);
void NuHashMapSetThreadSafe(NuHashMap_t *Map);
int NuHashMapAdd(NuHashMap_t *Map, const void *key, size_t klen, void *val, size_t vlen);
void NuHashMapRmv(NuHashMap_t *Map , const void *key, size_t klen);
int NuHashMapUpd(NuHashMap_t *Map , const void *key, size_t klen, void *val, size_t vlen);
void NuMultiHashMapAdd(NuHashMap_t *Map, const void *key, size_t klen, void *val, size_t vlen);
void NuMultiHashMapRmv(NuHashMap_t *Map , const void *key, size_t klen);
int NuMultiHashMapUpd(NuHashMap_t *Map , const void *key, size_t klen, void *val, size_t vlen);

int NuHashMapGetNum(NuHashMap_t *Map);

#ifdef __cplusplus
}
#endif

#endif /* _NUHASHMAP_H */

