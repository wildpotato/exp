#include "NuStream.h"
#include "NuMMapStream.h"
#include "NuFileStream.h"

/* internal functions */
/* ====================================================================== */

/* functions */
/* ====================================================================== */
/* ---------------------------------------------
 * for NuMMapStream
 * --------------------------------------------- */
static int MPStrmAllocFn(void *Argv, const char *File, size_t len)
{
    NuStrm_t   *pStrm = (NuStrm_t *)Argv;
    NuMPStrm_t *pMPStrm = NULL;
    char       *pe = NULL;
    size_t     allocSz = 0;

    allocSz = NU_RALIGN_PAGE(len);

    NuMPStrmNew(&pMPStrm, File, "a+", allocSz, PAGE_READWRITE , FILE_MAP_ALL_ACCESS);

    if(!pMPStrm)
    {
        return NU_FAIL;
    }

    pStrm->LeftSz = allocSz;
    pe = (char *)pMPStrm->addr;

    if(*pe =='\0') 
    {
        memset(pMPStrm->addr, MMAP_END, MMAP_END_SZ);
    }

    /* for end of file */
    pStrm->LeftSz -= MMAP_END_SZ;

    pStrm->Stream = pMPStrm;
    return NU_OK;
}

static int MPStrmFreeFn(void *Argv)
{
    NuStrm_t   *pStrm = (NuStrm_t *)Argv;
    void       *ptr = NULL;
    NuMPStrm_t *pMPStrm = NULL;

    while ( base_vector_pop(&(pStrm->vStrm), &ptr) != NU_EMPTY )
    {
        pMPStrm = (NuMPStrm_t *)ptr;
        NuMPStrmFree(pMPStrm);
    }

    return NU_OK;
}

static int MPStrmFlushFn(void *Argv)
{
    NuStrm_t   *pStrm = (NuStrm_t *)Argv;
    NuMPStrm_t *pMPStrm = (NuMPStrm_t *)pStrm->Stream;
    NuMPStrmSync(pMPStrm, FILE_MAP_ALL_ACCESS); 
    return NU_OK;
}

static int MPStrmSeekToEndFn(void *Argv)
{
    char  *ptr = NULL;
    int   *pe = NULL;
    size_t seek_len = 0;

    NuStrm_t   *pStrm = (NuStrm_t *)Argv;
    NuMPStrm_t *pMPStrm = (NuMPStrm_t *)pStrm->Stream;

    ptr = NuMPStrmGetAddr(pMPStrm);
    pe = (int *)ptr;
    while (*pe != MMAP_END)
    {
        --pStrm->LeftSz;
        pe = (int *)(++ptr);
    }

    seek_len = (NU_RALIGN_PAGE(pStrm->AllocSize) - pStrm->LeftSz - MMAP_END_SZ);
    if(seek_len > 0) {
        NuMPStrmGet(pMPStrm,seek_len, (void **)&ptr);
	}

    return pStrm->LeftSz;
}

static size_t MPStrmWriteNFn(void *Argv, const void *data, size_t len)
{
    NuStrm_t   *pStrm = (NuStrm_t *)Argv;
    NuMPStrm_t *pMPStrm = (NuMPStrm_t *)(pStrm->Stream);

    NuMPStrmWriteN(pMPStrm, data, len);

    memset(pMPStrm->addr, MMAP_END, MMAP_END_SZ);

    return len;
}

static size_t MPStrmVPrintf(void *Argv, const char *fmt, va_list ap)
{
    NuStrm_t   *pStrm = (NuStrm_t *)Argv;
    NuMPStrm_t *pMPStrm = (NuMPStrm_t *)(pStrm->Stream);
    size_t len = 0;

    len = NuMPStrmVPrintf(pMPStrm, fmt, ap);
    memset(pMPStrm->addr, MMAP_END, MMAP_END_SZ);

    return len;
}

NuStrmFnType_t NuStrmMmap = {&MPStrmFlushFn, &MPStrmSeekToEndFn, &MPStrmFreeFn, &MPStrmWriteNFn, &MPStrmAllocFn, &MPStrmVPrintf};

/* ---------------------------------------------
 * for NuFileStream
 * --------------------------------------------- */
static int FStrmAllocFn(void *Argv, const char *File, size_t len)
{
    NuStrm_t       *pStrm = (NuStrm_t *)Argv;
    NuFileStream_t *pFStrm = NULL;

    NuFStreamOpen(&pFStrm, File, "a+");
    if(!pFStrm)
    {
        return NU_FAIL;
    }

    pStrm->LeftSz = len;

    pStrm->Stream = pFStrm;
    return NU_OK;
}

static int FStrmFreeFn(void *Argv)
{
    NuStrm_t       *pStrm = (NuStrm_t *)Argv;
    NuFileStream_t *pFStrm = NULL;
    void           *ptr = NULL;

    while ( base_vector_pop(&(pStrm->vStrm), &ptr) != NU_EMPTY )
    {
        pFStrm = (NuFileStream_t *)ptr;
        NuFStreamClose(pFStrm);
    }

    return NU_OK;
}

static int FStrmFlushFn(void *Argv)
{
    NuStrm_t       *pStrm = (NuStrm_t *)Argv;
    NuFileStream_t *pFStrm = (NuFileStream_t *)(pStrm->Stream);

    NuFStreamFlush(pFStrm);
    return NU_OK;
}

static int FStrmSeekToEndFn(void *Argv)
{
    NuStrm_t       *pStrm   = (NuStrm_t *)Argv;
    NuFileStream_t *pFStrm = NULL;

    pFStrm = (NuFileStream_t *)base_vector_get_by_index(&(pStrm->vStrm), base_vector_get_cnt(&(pStrm->vStrm)) - 1);

    NuFStreamSeek(pFStrm, 0L, SEEK_END);

    pStrm->LeftSz = pStrm->AllocSize - NuFStreamGetSize(pFStrm);

    return pStrm->LeftSz;
}

static size_t FStrmWriteNFn(void *Argv, const void *data, size_t len)
{
    NuStrm_t       *pStrm = (NuStrm_t *)Argv;
    NuFileStream_t *pFStrm = (NuFileStream_t *)(pStrm->Stream);

    return NuFStreamWriteN(pFStrm, data, len);
}

static size_t FStrmVPrintf(void *Argv, const char *fmt, va_list ap)
{
    NuStrm_t       *pStrm = (NuStrm_t *)Argv;
    NuFileStream_t *pFStrm = (NuFileStream_t *)(pStrm->Stream);

    return NuFStreamVPrintf(pFStrm, fmt, ap);
}

NuStrmFnType_t NuStrmFile = {&FStrmFlushFn, &FStrmSeekToEndFn, &FStrmFreeFn, &FStrmWriteNFn, &FStrmAllocFn, &FStrmVPrintf};

