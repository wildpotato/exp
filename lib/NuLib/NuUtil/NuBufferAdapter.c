
#include "NuCommon.h"
#include "NuStr.h"
#include "NuMMapStream.h"

static void *NuBufferMallocAlloc(void *Allocator, size_t AllocSize)
{
    return malloc(AllocSize);
}

static void NuBufferMallocFree(void *Alloc)
{
    free(Alloc);
	return;
}

static void NuBufferMallocSet(void *Allocator, void *Argu)
{
    return;
}

static void *NuBufferMallocGet(void *AllocHdr)
{
    return AllocHdr;
}

NuBufferAllocType_t NuBufferMalloc = {
                                        .Alloc = &NuBufferMallocAlloc,
                                        .Free = &NuBufferMallocFree,
                                        .Get = &NuBufferMallocGet,
                                        .Set = &NuBufferMallocSet
                                    };

static void *NuBufferMmapAlloc(void *Allocator, size_t AllocSize)
{
    NuMPStrm_t  *pMmap = NULL;
    char        *pStr = (char *)Allocator;
    char        *pVer = strrchr(pStr, '.');

    if(!pVer)
    {
        return NULL;
    }

    if(NuMPStrmNew(&pMmap, pStr, "a+", AllocSize, PAGE_READWRITE, FILE_MAP_ALL_ACCESS) < 0)
    {
        return NULL;
    }

    *pVer = '\0';
    sprintf(pVer, ".%d", atoi(pVer + 1) + 1);

    return pMmap;
}

static void NuBufferMmapFree(void *pAlloc)
{
    NuMPStrmFree(pAlloc);
    return;
}

static void NuBufferMmapSet(void *Allocator, void *Argu)
{
    char    *pStr = (char *)Allocator;

    strcpy(pStr, (char *)Argu);
    strcat(pStr, ".0");

    return;
}

static void *NuBufferMmapGet(void *AllocHdr)
{
    return NuMPStrmGetAddr((NuMPStrm_t *)AllocHdr);
}

NuBufferAllocType_t NuBufferMmap = {
                                        .Alloc = &NuBufferMmapAlloc,
                                        .Free = &NuBufferMmapFree,
                                        .Get = &NuBufferMmapGet,
                                        .Set = &NuBufferMmapSet
                                    };

