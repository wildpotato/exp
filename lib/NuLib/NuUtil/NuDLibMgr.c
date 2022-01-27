
#include <dlfcn.h>
#include <pthread.h>

#include "NuBuffer.h"
#include "NuHash.h"
#include "NuStr.h"
#include "NuLock.h"
#include "NuFile.h"
#include "NuDLibMgr.h"

typedef struct _NuDLibMgrItem_t
{
    NuStr_t     *Name;
    void        *Hdlr;
} NuDLibMgrItem_t;

#define NuDLibMgr_HashSize   32

typedef struct _NuDLibMgr_t
{
    NuHash_t        *Hash;
    NuBuffer_t      *MemBuf;
    NuStr_t         *KeyCache;
    NuLock_t        Lock;
    pthread_once_t  Once;
} NuDLibMgr_t;

static NuDLibMgr_t _DLibMgr = {
                                .Hash = NULL,
                                .MemBuf = NULL,
                                .KeyCache = NULL,
                                .Once = PTHREAD_ONCE_INIT
                                };

static void _NuDLibMgrInit(void)
{
    NuHashNew(&(_DLibMgr.Hash), NuDLibMgr_HashSize);
    NuBufferNew(&(_DLibMgr.MemBuf), sizeof(NuDLibMgrItem_t), 128);
    NuStrNew(&(_DLibMgr.KeyCache), "\0");
    NuLockInit(&(_DLibMgr.Lock), &NuLockType_Mutex);

    return;
}

static NuDLibMgrItem_t *GetItem(const char *Name, void *Hdlr)
{
    NuDLibMgrItem_t *DLibMgrItem = (NuDLibMgrItem_t *)NuBufferGet(_DLibMgr.MemBuf);

    NuStrNew(&(DLibMgrItem->Name), Name);
    DLibMgrItem->Hdlr = Hdlr; 

    return DLibMgrItem;
}

int NuDLibMgrLoad(const char *AliasName, const char *DLibPath, int Flag, char **Err)
{
    int             RC = NU_OK;
    NuDLibMgrItem_t *DLibMgrItem = NULL;

    unsigned int    Idx = 0;
    NuHashItem_t    *HashItem = NULL;
    void            *pHdlr = NULL;

    *Err = NULL;

    pthread_once(&(_DLibMgr.Once), &_NuDLibMgrInit);

    if(!NuIsFile(DLibPath))
    {
        NUGOTO(RC, NU_FAIL, EXIT);
    }

    if(!(pHdlr = dlopen(DLibPath, RTLD_NOW | Flag)))
    {
        *Err = dlerror();
        NUGOTO(RC, NU_FAIL, EXIT);
    }

    NuLockLock(&(_DLibMgr.Lock));

    if(!(HashItem = NuHashSearch(_DLibMgr.Hash, AliasName, strlen(AliasName), &Idx)))
    {
        DLibMgrItem = GetItem(AliasName, pHdlr);

        NuHashAdd(_DLibMgr.Hash, NuStrGet(DLibMgrItem->Name), NuStrSize(DLibMgrItem->Name), DLibMgrItem, Idx);
    }
    else
    {
        DLibMgrItem = (NuDLibMgrItem_t *)NuHashItemGetValue(HashItem);
        if(DLibMgrItem->Hdlr != pHdlr)
        {
            NUGOTO(RC, NU_FAIL, EXIT);
        }

        DLibMgrItem->Hdlr = pHdlr;
    }

EXIT:
    NuLockUnLock(&(_DLibMgr.Lock));
    return RC;
}

int NuDLibMgrLoadToGlobal(const char *AliasName, const char *DLibPath, char **Err)
{
    return NuDLibMgrLoad(AliasName, DLibPath, RTLD_GLOBAL, Err);
}

int NuDLibMgrLoadToLocal(const char *AliasName, const char *DLibPath, char **Err)
{
    return NuDLibMgrLoad(AliasName, DLibPath, RTLD_LOCAL, Err);
}

void NuDLibMgrUnLoad(const char *AliasName, char **Err)
{
    unsigned int    Idx = 0;
    NuHashItem_t    *HashItem = NULL;
    NuDLibMgrItem_t *DLibMgrItem = NULL;

    *Err = NULL;

    pthread_once(&(_DLibMgr.Once), &_NuDLibMgrInit);

    if(strstr(AliasName, "::"))
    {
        return;
    }

    NuLockLock(&(_DLibMgr.Lock));

    /* remove from hash */
    if((HashItem = NuHashSearch(_DLibMgr.Hash, AliasName, strlen(AliasName), &Idx)) != NULL)
    {
        NuHashRemove(_DLibMgr.Hash, HashItem, Idx);

        DLibMgrItem = (NuDLibMgrItem_t *)NuHashItemGetValue(HashItem);

        dlclose(DLibMgrItem->Hdlr);
        *Err = dlerror();

        DLibMgrItem->Hdlr = NULL;
    }

    NuLockUnLock(&(_DLibMgr.Lock));
    return;
}

static void UnLoadAllHelper(NuHashItem_t *Item, void *Argu)
{
    char        *Err = NULL;

    NuDLibMgrUnLoad(NuHashItemGetKey(Item), &Err);

    return;
}

void NuDLibMgrUnLoadAll(void)
{
    pthread_once(&(_DLibMgr.Once), &_NuDLibMgrInit);

    NuHashForEach(_DLibMgr.Hash, NULL, 0, &UnLoadAllHelper, NULL);

    return;
}

void *NuDLibMgrGetFn(const char *AliasName, const char *FnName, char **Err)
{
    void            *Fn = NULL;
    unsigned int    SoIdx = 0, FnIdx = 0;
    NuHashItem_t    *HashItem = NULL;
    NuDLibMgrItem_t *DLibMgrItem = NULL;
    NuStr_t         *Key = NULL;

    *Err = NULL;

    pthread_once(&(_DLibMgr.Once), &_NuDLibMgrInit);

    NuLockLock(&(_DLibMgr.Lock));

    Key = _DLibMgr.KeyCache;

    NuStrClear(Key);
    NuStrCpy(Key, AliasName);
    NuStrCat(Key, "::");
    NuStrCat(Key, FnName);

    if(!(HashItem = NuHashSearch(_DLibMgr.Hash, NuStrGet(Key), NuStrSize(Key), &FnIdx)))
    { /* Not exists, try load it. */
        if((HashItem = NuHashSearch(_DLibMgr.Hash, AliasName, strlen(AliasName), &SoIdx)) != NULL)
        {
            DLibMgrItem = GetItem(NuStrGet(Key), NULL);
            DLibMgrItem->Hdlr = dlsym(((NuDLibMgrItem_t *)NuHashItemGetValue(HashItem))->Hdlr, FnName);
            if(!((*Err) = dlerror()))
            {
                if(!(HashItem = NuHashSearch(_DLibMgr.Hash, NuStrGet(Key), NuStrSize(Key), &SoIdx)))
                {
                    NuHashAdd(_DLibMgr.Hash, NuStrGet(DLibMgrItem->Name), NuStrSize(Key), DLibMgrItem, SoIdx);
                }
                else
                {
                    NuMultiHashAdd(_DLibMgr.Hash, HashItem, DLibMgrItem);
                }

                //HashItem = NuHashAdd(_DLibMgr.Hash, NuStrGet(DLibMgrItem->Name), NuStrSize(DLibMgrItem->Name), DLibMgrItem, FnIdx);
                NuHashAdd(_DLibMgr.Hash, NuStrGet(DLibMgrItem->Name), NuStrSize(DLibMgrItem->Name), DLibMgrItem, FnIdx);
 
                Fn = DLibMgrItem->Hdlr;
            }
            else
            { /* Load fn failed. */
                NuStrFree(DLibMgrItem->Name);
                NuBufferPut(_DLibMgr.MemBuf, DLibMgrItem);
            }
        }
    }
    else
    { /* Found. */
        DLibMgrItem = (NuDLibMgrItem_t *)NuHashItemGetValue(HashItem);
        if(!(Fn = DLibMgrItem->Hdlr))
        { /* Might be unloaded or reloaded */
            if((HashItem = NuHashSearch(_DLibMgr.Hash, AliasName, strlen(AliasName), &SoIdx)) != NULL)
            {
                DLibMgrItem->Hdlr = dlsym(((NuDLibMgrItem_t *)NuHashItemGetValue(HashItem))->Hdlr, FnName);
                if(!((*Err) = dlerror()))
                {
                    Fn = DLibMgrItem->Hdlr;
                }
            }
        }
    }

    NuLockUnLock(&(_DLibMgr.Lock));

    return Fn;
}

