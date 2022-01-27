
#include "NuUtil.h"
#include "NuLock.h"
#include "NuFile.h"
#include "NuCommon.h"
#include "NuStr.h"
#include "NuCStr.h"
#include "NuMMap.h"

#include "NuSeqNo.h"
#include "NuSeqNo_String.c"
#include "NuSeqNo_Int.c"

static int _NuSeqNoSetFile(NuSeqNo_t *pSeqNo)
{
    int     iRC = NU_OK;
    int     Len = pSeqNo->Length;
    int     IsNewFile = 0;
    int     iSize = NU_ALIGN8( ((Len * 2) + 10) );

    if(!NuIsFile(NuStrGet(pSeqNo->pPath)))
    {
        IsNewFile = 1;
    }

    if((iRC = NuMMapNew(&(pSeqNo->pSeqFile), NuStrGet(pSeqNo->pPath), "a+", iSize, PAGE_READWRITE, FILE_MAP_ALL_ACCESS)) < 0)
    {
        return iRC;
    }

    pSeqNo->pSeqFilePos1 = (char *)NuMMapGetAddr(pSeqNo->pSeqFile);
    pSeqNo->pSeqFilePos2 = pSeqNo->pSeqFilePos1 + Len + 1;

    if(IsNewFile)
    {
        memset(pSeqNo->pSeqFilePos1, '\0', iSize);
    }

    if(*(pSeqNo->pSeqFilePos1) == '\0')
    { /* First position is null. */
        if(*(pSeqNo->pSeqFilePos2) != '\0')
        {
            pSeqNo->pCurSeqNo = pSeqNo->pSeqFilePos2;
        }
        else
        {
            memcpy(pSeqNo->pCurSeqNo = pSeqNo->pSeqFilePos1, pSeqNo->pMinSeqNo, pSeqNo->Length);
        }
    }
    else
    { /* First position nonnull. */
        if(*(pSeqNo->pSeqFilePos2) == '\0')
        {
            pSeqNo->pCurSeqNo = pSeqNo->pSeqFilePos1;
        }
        else
        {
            if(pSeqNo->Type.Compare(pSeqNo->pSeqFilePos1, Len, pSeqNo->pSeqFilePos2) < 0)
            {
                pSeqNo->pCurSeqNo = pSeqNo->pSeqFilePos2;
            }
            else
            {
                pSeqNo->pCurSeqNo = pSeqNo->pSeqFilePos1;
            }
        }
    }

    return NU_OK;
}

void NuSeqNoSetThreadSafe(NuSeqNo_t *SeqNo)
{
    NuLockDestroy(&(SeqNo->Lock));
    NuLockInit(&(SeqNo->Lock), &NuLockType_Spin);

    return;
}

int NuSeqNoNew(NuSeqNo_t **pSeqNo, NuSeqNoType_t *Type, int SeqNoLen, const char *FilePath, const char *Name)
{
    return NuSeqNoNew2(pSeqNo, Type, SeqNoLen, FilePath, Name, NULL, NULL, NULL);
}

int NuSeqNoNew2(NuSeqNo_t **pSeqNo, NuSeqNoType_t *Type, int SeqNoLen, const char *FilePath, const char *Name, NuSeqNoNextFn Next_fn, NuSeqNoCmpFn Compare_fn, void *Argu)
{
    int RC = 0;

    if(!((*pSeqNo) = (NuSeqNo_t *)malloc(sizeof(NuSeqNo_t))))
    {
        NUGOTO(RC, NU_MALLOC_FAIL, EXIT);
    }
    else
    {
        (*pSeqNo)->pName = NULL;
        (*pSeqNo)->pPath = NULL;
        (*pSeqNo)->pCurSeqNo = NULL;
        (*pSeqNo)->pMaxSeqNo = NULL;
        (*pSeqNo)->pMinSeqNo = NULL;
        (*pSeqNo)->pSeqFile = NULL;
        (*pSeqNo)->pSeqFilePos1 = NULL;
        (*pSeqNo)->pSeqFilePos2 = NULL;
        (*pSeqNo)->Argu = Argu;
        (*pSeqNo)->Length = SeqNoLen;

        memcpy(&((*pSeqNo)->Type), Type, sizeof(NuSeqNoType_t));
        if(Next_fn != NULL)
        {
            (*pSeqNo)->Type.Next = Next_fn;
        }

        if(Compare_fn != NULL)
        {
            (*pSeqNo)->Type.Compare = Compare_fn;
        }
    }

    NuLockInit(&((*pSeqNo)->Lock), &NuLockType_NULL);

    if(NuStrNew(&((*pSeqNo)->pName), Name) < 0)
    {
        NUGOTO(RC, NU_MALLOC_FAIL, EXIT);
    }

    if(NuStrNew(&((*pSeqNo)->pPath), NULL) < 0)
    {
        NUGOTO(RC, NU_MALLOC_FAIL, EXIT);
    }

    (*pSeqNo)->pMaxSeqNo = (char *)malloc(sizeof(char) * SeqNoLen);

    (*pSeqNo)->pMinSeqNo = (char *)malloc(sizeof(char) * SeqNoLen);

    if(!((*pSeqNo)->pMaxSeqNo) || !((*pSeqNo)->pMinSeqNo))
    {
        NUGOTO(RC, NU_MALLOC_FAIL, EXIT);
    }

    (*pSeqNo)->Type.SeqNoInit((*pSeqNo), NULL);

    NuCreateRecursiveDir(FilePath);

    NuPathCombine((*pSeqNo)->pPath, FilePath, Name);
    NuStrCat((*pSeqNo)->pPath, ".seq");

    if((RC = _NuSeqNoSetFile((*pSeqNo))) < 0)
    {
        goto EXIT;
    }

    RC = NU_OK;
EXIT:

    return RC;
}

void NuSeqNoFree(NuSeqNo_t *SeqNo)
{
    NuLockLock(&(SeqNo->Lock));

    NuStrFree(SeqNo->pName);
    NuStrFree(SeqNo->pPath);
    
    if(SeqNo->pMaxSeqNo != NULL)
    {
        free(SeqNo->pMaxSeqNo);
    }

    if(SeqNo->pMinSeqNo != NULL)
    {
        free(SeqNo->pMinSeqNo);
    }

    NuMMapFree(SeqNo->pSeqFile);

    NuLockUnLock(&(SeqNo->Lock));

    NuLockDestroy(&(SeqNo->Lock));

    free(SeqNo);

    return;
}

void NuSeqNoSetMaxNo(NuSeqNo_t *SeqNo, void *MaxSeqNo)
{
    NuLockLock(&(SeqNo->Lock));
    SeqNo->Type.SetMaxNo(SeqNo, MaxSeqNo);
    NuLockUnLock(&(SeqNo->Lock));

    return;
}

void NuSeqNoSetMinNo(NuSeqNo_t *SeqNo, void *MinSeqNo)
{
    NuLockLock(&(SeqNo->Lock));
    SeqNo->Type.SetMinNo(SeqNo, MinSeqNo);
    NuLockUnLock(&(SeqNo->Lock));

    return;
}

void NuSeqNoSetCurNo(NuSeqNo_t *SeqNo, void *CurrSeqNo)
{
    NuLockLock(&(SeqNo->Lock));
    SeqNo->Type.SetCurNo(SeqNo, CurrSeqNo);
    NuLockUnLock(&(SeqNo->Lock));

    return;
}

int NuSeqNoGetCurNo(NuSeqNo_t *SeqNo, void *CurrSeqNo)
{
    int RC = NU_OK;

    NuLockLock(&(SeqNo->Lock));
    RC = SeqNo->Type.GetCurNo(SeqNo, CurrSeqNo);
    NuLockUnLock(&(SeqNo->Lock));

    return RC;
}

int NuSeqNoPop(NuSeqNo_t *SeqNo, void *PopSeqNo)
{
    int RC = NU_OK;

    NuLockLock(&(SeqNo->Lock));
    RC = SeqNo->Type.SeqNoPop(SeqNo, PopSeqNo);
    NuLockUnLock(&(SeqNo->Lock));

    return RC;
}

int NuSeqNoPush(NuSeqNo_t *SeqNo, void *PushSeqNo)
{
    int RC = NU_OK;

    NuLockLock(&(SeqNo->Lock));
    RC = SeqNo->Type.SeqNoPush(SeqNo, PushSeqNo);
    NuLockUnLock(&(SeqNo->Lock));

    return RC;
}

int NuSeqNoGetLen(NuSeqNo_t *SeqNo)
{
    return SeqNo->Length;
}

