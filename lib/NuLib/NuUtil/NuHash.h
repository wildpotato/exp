
#include "NuCommon.h"
#include "NuUtil.h"
#include "NuBuffer.h"

#ifndef _NUHASH_H
#define _NUHASH_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _NuHashItem_t NuHashItem_t;

typedef unsigned int (*NuHashFn)(const void *Key, size_t KeyLen);
typedef int (*NuHashCompareFn)(const void *Key1, size_t KeyLen1, const void *Key2, size_t KeyLen2);

typedef struct _NuHash_t NuHash_t;

/* default buffer size is 1024. 
 * the BucketSz use for hash bucket. it can't 
 * the BufferSz use for NuHashItem, when not enit will be auto generate by itself.
 * */
int NuHashNew(NuHash_t **pHash, int BucketSz);
int NuHashNew2(NuHash_t **pHash, int BucketSz, NuHashFn HashFn, NuHashCompareFn CompareFn);
int NuHashNewPreAlloc(NuHash_t **pHash, int BucketSz, int BufferSz);
int NuHashNewPreAlloc2(NuHash_t **pHash, int BucketSz, int BufferSz, NuHashFn HashFn, NuHashCompareFn CompareFn);

void NuHashFree(NuHash_t *Hash);
void NuHashClear(NuHash_t *Hash);

NuHashItem_t *NuHashSearch(NuHash_t *Hash, const void *Key, size_t KeyLen, unsigned int *Idx);
NuHashItem_t *NuHashAdd(NuHash_t *Hash, const void *Key, size_t KeyLen, void *Value, unsigned int Idx);
NuHashItem_t *NuMultiHashAdd(NuHash_t *Hash, NuHashItem_t *Item, void *Value);

void NuHashRemove(NuHash_t *Hash, NuHashItem_t *Item, unsigned int Idx);
/* if call NuHashRemove2 , please call NuHashItemRemoveComplete for destroy NuHashItem_t */
NuHashItem_t *NuHashRemove2(NuHash_t *Hash, NuHashItem_t *Item, unsigned int Idx);
void NuHashItemRemoveComplete(NuHash_t *Hash, NuHashItem_t *Item);

int NuHashGetNum(NuHash_t *Hash);

NuHashItem_t *NuHashRight(NuHashItem_t *Item);
NuHashItem_t *NuHashLeft(NuHashItem_t *Item);
NuHashItem_t *NuHashRightMost(NuHashItem_t *Item);
NuHashItem_t *NuHashLeftMost(NuHashItem_t *Item);

const void *NuHashItemGetKey(NuHashItem_t *Item);
size_t NuHashItemGetKeyLen(NuHashItem_t *Item);
void *NuHashItemGetValue(NuHashItem_t *Item);
int NuHashItemGetKeyCollision(NuHashItem_t *Item);
void NuHashItemSetValue(NuHashItem_t *Item, void *Value);

typedef void (*NuHashForEachFn)(NuHashItem_t *Item, void *Argu);
void NuHashForEach(NuHash_t *Hash, const void *Key, size_t KeyLen, NuHashForEachFn Fn, void *Argu);

#ifdef __cplusplus
}
#endif

#endif /* _NUHASH_H */

