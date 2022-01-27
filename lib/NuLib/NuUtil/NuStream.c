#include "NuStream.h"
#include "NuStreamAdapter.c"

/* internal functions */
/* ====================================================================== */

static void _GenFileName(NuStr_t *File, const char *Dir, const char *Name, int Ver)
{
    if(Dir[strlen(Dir) - 1] == NUFILE_SEPARATOR)
    {
        NuStrPrintf(File, 0, "%s%s.%d", Dir, Name, Ver);
    }
    else
    {
        NuStrPrintf(File, 0, "%s%c%s.%d", Dir, NUFILE_SEPARATOR, Name, Ver);
    }

    return;
}

static void _GetLastVer(const char *Name, void *Argu)
{
    NuStrm_t    *Strm = (NuStrm_t *)Argu;    
	char        *ptr = NULL;

    if(!memcmp(Name, NuStrGet(Strm->Name), NuStrSize(Strm->Name)))
    {
		ptr = strrchr(Name, '.');
		if (ptr != NULL)
		{
			int Ver = strtol(ptr + 1, NULL, 10);
			if (Ver > 0)
			{
				if(Strm->MaxVer < Ver)
				{
					Strm->MaxVer = Ver;
				}
			}
		}
    }

    return;
}

int _WriteExtenFn(void *Argu, const void *Data, size_t DataLen)
{
    NuStrm_t    *pStrm = (NuStrm_t *)Argu;

    if(DataLen > pStrm->LeftSz)
    {
        ++ pStrm->MaxVer;
        _GenFileName(pStrm->File, NuStrGet(pStrm->Dir), NuStrGet(pStrm->Name), pStrm->MaxVer);

        if(pStrm->CB_Alloc(pStrm, NuStrGet(pStrm->File), pStrm->AllocSize) < 0)
        {
            return NU_FAIL;
        }

        base_vector_push(&(pStrm->vStrm), pStrm->Stream);
    }

    if(pStrm->CB_WriteN(pStrm, Data, DataLen))
    {
        pStrm->LeftSz -= DataLen;
        return NU_OK;
    }
    else
    {
        return NU_FAIL;
    }
}

int _WriteFn(void *Argu, const void *Data, size_t DataLen)
{
    NuStrm_t    *pStrm = (NuStrm_t *)Argu;

    if(pStrm->CB_WriteN(pStrm, Data, DataLen))
    {
        pStrm->LeftSz -= DataLen;
        return NU_OK;
    }
    else
    {
        return NU_FAIL;
    }
}

/* global   functions */
/* ====================================================================== */
int NuStrmNew(NuStrm_t **pStrm, int StreamType, size_t AllocSize, const char *Dir, const char *Name)
{
    int RC = NU_OK;
    int i   = 0;

    (*pStrm) = NULL;
    if(!((*pStrm) = (NuStrm_t *)malloc(sizeof(NuStrm_t))))
    {
        return NU_MALLOC_FAIL;
    }

    memset((*pStrm), 0, sizeof(NuStrm_t));

    if(base_vector_init(&((*pStrm)->vStrm), 8) != NU_OK)
    {
        NUGOTO(RC, NU_FAIL, EXIT);
    }

    (*pStrm)->MaxVer = 0;
    (*pStrm)->StreamType = StreamType;
    (*pStrm)->AllocSize = AllocSize;

    if(StreamType == enFileStream)
    {
        (*pStrm)->CB_Alloc     = NuStrmFile.Alloc;
        (*pStrm)->CB_Free      = NuStrmFile.Free;
        (*pStrm)->CB_SeekToEnd = NuStrmFile.SeekToEnd;
        (*pStrm)->CB_Flush     = NuStrmFile.Flush;
        (*pStrm)->CB_WriteN    = NuStrmFile.WriteN;
        (*pStrm)->CB_IntWriteN = (AllocSize == 0) ? &_WriteFn : &_WriteExtenFn;

        (*pStrm)->CB_VPrintf    = NuStrmFile.VPrintf;

    }
    else if(StreamType == enMMapStream)
    {
        (*pStrm)->CB_Alloc     = NuStrmMmap.Alloc;
        (*pStrm)->CB_Free      = NuStrmMmap.Free;
        (*pStrm)->CB_SeekToEnd = NuStrmMmap.SeekToEnd;
        (*pStrm)->CB_Flush     = NuStrmMmap.Flush;
        (*pStrm)->CB_WriteN    = NuStrmMmap.WriteN;

        (*pStrm)->CB_VPrintf   = NuStrmMmap.VPrintf;

        if(AllocSize == 0)
        {
            NUGOTO(RC, NU_FAIL, EXIT);
        }
        else
        {
            (*pStrm)->CB_IntWriteN = &_WriteExtenFn;
        }
    }
    else
    {
        NUGOTO(RC, NU_FAIL, EXIT);
    }

    if(NuStrNew(&((*pStrm)->File), NULL) != NU_OK)
    {
        NUGOTO(RC, NU_MALLOC_FAIL, EXIT);
    }

    if(NuStrNew(&((*pStrm)->Dir), Dir) != NU_OK)
    {
        NUGOTO(RC, NU_MALLOC_FAIL, EXIT);
    }

    if(NuStrNew(&((*pStrm)->Name), Name) != NU_OK)
    {
        NUGOTO(RC, NU_MALLOC_FAIL, EXIT);
    }

    NuFileScanDir(Dir, &_GetLastVer, (*pStrm));

    for(i = 0; i <= (*pStrm)->MaxVer; ++ i)
    {
        _GenFileName((*pStrm)->File, Dir, Name, i);
        if((*pStrm)->CB_Alloc(*pStrm, NuStrGet((*pStrm)->File), (*pStrm)->AllocSize) < 0)
        {
            NUGOTO(RC, NU_FAIL, EXIT);
        }

        base_vector_push(&((*pStrm)->vStrm), (*pStrm)->Stream);
    }

    (*pStrm)->CB_SeekToEnd((void *)(*pStrm));

EXIT:
    if(RC < 0)
    {
        NuStrmFree((*pStrm));
    }

    return RC;
}

void NuStrmFree(NuStrm_t *Strm)
{
    if(Strm != NULL)
    {
        Strm->CB_Free(Strm);

        if(Strm->Name != NULL)
        {
            NuStrFree(Strm->Name);
        }

        if(Strm->Dir != NULL)
        {
            NuStrFree(Strm->Dir);
        }

        if(Strm->File != NULL)
        {
            NuStrFree(Strm->File);
        }

        base_vector_destroy(&(Strm->vStrm));
        free(Strm);
    }

    return;
}

size_t NuStrmGetAllocSize(NuStrm_t *Strm)
{
    return Strm->AllocSize;
}

int NuStrmWriteN(NuStrm_t *Strm, const void *Data, size_t DataLen)
{
    return Strm->CB_IntWriteN(Strm, Data, DataLen);
}

int NuStrmFlush(NuStrm_t *Strm)
{
    return Strm->CB_Flush(Strm);
}

int NuStrmVPrintf(NuStrm_t *Strm, const void *fmt, va_list ap)
{
    return Strm->CB_VPrintf(Strm, fmt, ap);
}
