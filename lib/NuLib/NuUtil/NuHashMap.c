
#include "NuHashMap.h"

#define HASHMAP_DEFAULT_VALUE_SIZE 64
#define HASHMAP_DEFAULT_KEY_SIZE   16

struct _NuHashMap_t
{
    NuHash_t    *pHash;
    NuBuffer_t  *KeyBuffer;
    NuBuffer_t  *ValBuffer;

    int         default_klen;
    int         default_vlen;

    NuLock_t    Lock;

    NuDestructor FreeKey_fn;
    NuDestructor FreeValue_fn;
};

/* Internal function */
/* ============================================================================== */

static int _NuHashMapFn(void *val)
{
    return NU_OK;
}

/* ----- Map functions ----- */
/* ============================================================================== */
int NuHashMapNew(NuHashMap_t **pMap, int Num)
{
    return NuHashMapNew2(pMap, Num, NULL, NULL, NULL, NULL);
}

int NuHashMapNew2(NuHashMap_t **pMap, int Num,  NuHashFn Hash_fn, 
                                                NuHashCompareFn Compare_fn,
                                                NuDestructor FreeKey_fn,
                                                NuDestructor FreeValue_fn)
{
    return NuHashMapNew3(pMap, Num, HASHMAP_DEFAULT_KEY_SIZE, HASHMAP_DEFAULT_VALUE_SIZE, Hash_fn, Compare_fn, FreeKey_fn, FreeValue_fn);
}

int NuHashMapNew3(NuHashMap_t **pMap, int Num, size_t klen, size_t vlen, NuHashFn Hash_fn, 
                                                                        NuHashCompareFn Compare_fn,
                                                                        NuDestructor FreeKey_fn,
                                                                        NuDestructor FreeValue_fn)
{
    int             iRC = NU_OK;
    unsigned int    default_size = 0;

    (*pMap) = (NuHashMap_t *)malloc(sizeof(NuHashMap_t));
    if ((*pMap) == NULL)
        NUGOTO(iRC, NU_MALLOC_FAIL, EXIT);

    default_size = iRC = NuHashNew2(&((*pMap)->pHash), Num, Hash_fn, Compare_fn);
    NUCHKRC(iRC, EXIT);

    iRC = NuBufferNew(&((*pMap)->KeyBuffer), klen, default_size);
    NUCHKRC(iRC, EXIT);

    iRC = NuBufferNew(&((*pMap)->ValBuffer), vlen, default_size);
    NUCHKRC(iRC, EXIT);

    (*pMap)->default_klen = klen;
    (*pMap)->default_vlen = vlen;
    (*pMap)->FreeKey_fn = (FreeKey_fn == NULL) ? &_NuHashMapFn : FreeKey_fn;
    (*pMap)->FreeValue_fn = (FreeValue_fn == NULL) ? &_NuHashMapFn : FreeValue_fn;

    NuLockInit(&((*pMap)->Lock), &NuLockType_NULL);
EXIT:
    if(iRC < 0)
    {
        if((*pMap)->pHash != NULL)
            NuHashFree((*pMap)->pHash);

        if((*pMap)->KeyBuffer != NULL)
            NuBufferFree((*pMap)->KeyBuffer);

        if((*pMap)->ValBuffer != NULL)
            NuBufferFree((*pMap)->ValBuffer);

        if((*pMap) != NULL)
            free((*pMap));
    }
    else
    {
        iRC = default_size;
    }

    return iRC;
}

void NuHashMapSetThreadSafe(NuHashMap_t *pMap)
{
    NuLockDestroy(&(pMap->Lock));
    NuLockInit(&(pMap->Lock), &NuLockType_Mutex);

    return;
}

void NuHashMapFree(NuHashMap_t *pMap)
{
    NuBufferFree(pMap->KeyBuffer);
    NuBufferFree(pMap->ValBuffer);
    NuHashFree(pMap->pHash);

    NuLockDestroy(&(pMap->Lock));

    free(pMap);

    return;
}

int NuHashMapAdd(NuHashMap_t *pMap, const void *key, size_t klen, void *val, size_t vlen)
{
    int             iRC = NU_OK;
    unsigned int    idx = 0;
    void            *KeyBuf = NULL, *ValBuf = NULL;
    NuHash_t        *pHash = pMap->pHash;

    NuLockLock(&(pMap->Lock));

    if(NuHashSearch(pHash, key, klen, &idx) == NULL)
    {
        KeyBuf = NuBufferGet(pMap->KeyBuffer);
        memcpy(KeyBuf, key, pMap->default_klen);

        ValBuf = NuBufferGet(pMap->ValBuffer);
        memcpy(ValBuf, val, pMap->default_vlen);

        NuHashAdd(pHash, KeyBuf, klen, ValBuf, idx);
    }
    else
    {
        iRC = NU_DUPLICATE;
    }

    NuLockUnLock(&(pMap->Lock));

    return iRC;
}

void NuMultiHashMapAdd(NuHashMap_t *pMap, const void *key, size_t klen, void *val, size_t vlen)
{
    unsigned int    idx = 0;
    void            *KeyBuf = NULL, *ValBuf = NULL;
    NuHashItem_t    *pItem = NULL;
    NuHash_t        *pHash = pMap->pHash;

    NuLockLock(&(pMap->Lock));

    ValBuf = NuBufferGet(pMap->ValBuffer);
    memcpy(ValBuf, val, pMap->default_vlen);

    if((pItem = NuHashSearch(pHash, key, klen, &idx)) == NULL)
    {
        KeyBuf = NuBufferGet(pMap->KeyBuffer);
        memcpy(KeyBuf, key, pMap->default_klen);

        NuHashAdd(pHash, KeyBuf, klen, ValBuf, idx);
    }
    else
    {
        NuMultiHashAdd(pHash, pItem, ValBuf);
    }

    NuLockUnLock(&(pMap->Lock));

    return;
}

void NuHashMapRmv(NuHashMap_t *Map, const void *Key, size_t KeyLen)
{
    unsigned int    Idx = 0;
    NuHashItem_t    *Item = NULL;  
    NuHash_t        *Hash = Map->pHash;

    NuLockLock(&(Map->Lock));

    if((Item = NuHashSearch(Hash, Key, KeyLen, &Idx)) != NULL)
    {
        NuBufferPut(Map->ValBuffer, NuHashItemGetValue(Item));
        NuBufferPut(Map->KeyBuffer, (void *)NuHashItemGetKey(Item));

        NuHashRemove(Hash, Item, Idx);
    }

    NuLockUnLock(&(Map->Lock));

    return;
}

void NuMultiHashMapRmv(NuHashMap_t *Map , const void *Key, size_t KeyLen)
{
    unsigned int    Idx = 0;
    NuHashItem_t    *Item = NULL, *TmpItem = NULL;
    NuHash_t        *Hash = Map->pHash;

    NuLockLock(&(Map->Lock));

    Item = NuHashSearch(Hash, Key, KeyLen, &Idx);
    while(Item != NULL)
    {
        TmpItem = Item;

        NuBufferPut(Map->ValBuffer, NuHashItemGetValue(TmpItem));
        NuBufferPut(Map->KeyBuffer, (void *)NuHashItemGetKey(TmpItem));
        NuHashRemove(Hash, TmpItem, Idx);

        Item = NuHashRight(Item);
    }

    NuLockUnLock(&(Map->Lock));

    return;
}

int NuHashMapUpd(NuHashMap_t *pMap, const void *key, size_t klen, void *val, size_t vlen)
{
    int             RC = NU_OK;
    unsigned int    idx = 0;
    NuHashItem_t    *pItem = NULL;

    NuLockLock(&(pMap->Lock));

    if((pItem = NuHashSearch(pMap->pHash, key, klen, &idx)) != NULL)
    {
        memcpy(NuHashItemGetValue(pItem), val, pMap->default_vlen);
        NuHashItemSetValue(pItem, NuHashItemGetValue(pItem));
    }
    else
    {
        RC = NU_NOTFOUND;
    }

    NuLockUnLock(&(pMap->Lock));

    return RC;
}

int NuMultiHashMapUpd(NuHashMap_t *pMap , const void *key, size_t klen, void *val, size_t vlen)
{
    unsigned int    idx = 0;
    NuHashItem_t    *pItem = NULL;

    NuLockLock(&(pMap->Lock));

    pItem = NuHashSearch(pMap->pHash, key, klen, &idx);
    while(pItem != NULL)
    {
        memcpy(NuHashItemGetValue(pItem), val, pMap->default_vlen);
        NuHashItemSetValue(pItem, NuHashItemGetValue(pItem));

        pItem = NuHashRight(pItem);
    }

    NuLockUnLock(&(pMap->Lock));

    return NU_OK;
}

int NuHashMapGetNum(NuHashMap_t *Map)
{
    return NuHashGetNum(Map->pHash);
}

