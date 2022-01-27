
#include "NuLock.h"
#include "NuTime.h"
#include "NuFile.h"
#include "NuStream.h"
#include "NuStr.h"
#include "NuLog.h"

/* macro */
/* -------------------------------------------------------------------- */
#define NuLogTime           "hh:mm:ss.SSSSSS"
#define NuLogLogTag         NuLogTime" [MSG] Msg"
#define NuLogErrorTag       NuLogTime" [ERR] Msg"
#define NuLogBinTag         NuLogTime" [WRT] Msg"

#define NuLogTimeLen        sizeof(NuLogTime) - 1
#define NuLogTagLen         sizeof(" [TAG] ") - 1

#define NuLogPrefixLen      NuLogTimeLen + NuLogTagLen

#define NuMBSz              (1024 * 1024)

struct _NuLog_t
{
    NuStrm_t    *FStream;
    NuLock_t    Lock;
    NuEventFn   FlushFn;
    NuStr_t     *Log;
    NuStr_t     *Err;
    char        Bin[NuLogPrefixLen];
};

/* static function     */
/* ====================================================================== */

static void NuLogFlushFn(void *Argu)
{
    NuStrmFlush(((NuLog_t *)Argu)->FStream);

    return;
}

static void NuLogBufInit(NuStr_t **Str, const char *Prefix)
{
    NuStrNew(Str, Prefix);

    return;
}

static int _NuLogOpen(NuLog_t **pLog, const char *Path, const char *FileName, bool AutoFlush, size_t FileSz, int StreamType)
{
    int RC = 0;
    (*pLog) = NULL;

    if(!((*pLog) = (NuLog_t *)malloc(sizeof(NuLog_t))))
    {
        NUGOTO(RC, NU_MALLOC_FAIL, EXIT);  
    }

    (*pLog)->FStream = NULL;

    if(AutoFlush)
    {
        (*pLog)->FlushFn = &NuLogFlushFn;
    }
    else
    {
        (*pLog)->FlushFn = &NuEventFn_Default;
    }

    NuCreateRecursiveDir(Path);

    RC = NuStrmNew(&((*pLog)->FStream), StreamType, FileSz, Path, FileName);
    NUCHKRC(RC, EXIT);

    NuLogBufInit(&((*pLog)->Log), NuLogLogTag);
    NuLogBufInit(&((*pLog)->Err), NuLogErrorTag);
    memcpy((*pLog)->Bin, NuLogBinTag, NuLogPrefixLen);
    NuLockInit(&((*pLog)->Lock), &NuLockType_NULL);

    RC = NU_OK;

EXIT:
    if(RC < 0)
    {
        NuLogClose((*pLog));
    }

    return RC;
}

static void NuLogVPrintf(NuLog_t *Log, NuStr_t *Str, const char *Format, va_list ArguList)
{
    NuLockLock(&(Log->Lock));

    NuGetTime((char *)NuStrGet(Str));
    NuStrSetChr(Str, NuLogTimeLen, ' ');

    NuStrmWriteN(Log->FStream, NuStrGet(Str), NuLogPrefixLen);
    NuStrmVPrintf(Log->FStream, Format, ArguList);

    Log->FlushFn(Log);

    NuLockUnLock(&(Log->Lock));

    return;
}

/* Log function        */
/* ====================================================================== */
int NuLogOpen(NuLog_t **pLog, const char *Path, const char *FileName)
{
    return _NuLogOpen(pLog, Path, FileName, false, (1000 * NuMBSz), enFileStream);
}

int NuLogOpen2(NuLog_t **pLog, const char *Path, const char *FileName, bool AutoFlush)
{
    return _NuLogOpen(pLog, Path, FileName, AutoFlush, (1000 * NuMBSz), enFileStream);
}

int NuLogOpen3(NuLog_t **pLog, const char *Path, const char *FileName, bool AutoFlush, size_t FileMBSz, int StreamType)
{
    return _NuLogOpen(pLog, Path, FileName, AutoFlush, FileMBSz * (NuMBSz), StreamType);
}

void NuLogClose(NuLog_t *Log)
{
    if(Log != NULL)
    {
        if(Log->FStream != NULL)
        {
            NuStrmFree(Log->FStream);
            Log->FStream = NULL;
        }

	    NuStrFree(Log->Log);
	    NuStrFree(Log->Err);
        NuLockDestroy(&(Log->Lock));

        free(Log);
    }

    return;
}

void NuLogSetAutoFlush(NuLog_t *Log)
{
    Log->FlushFn = &NuLogFlushFn;

    return;
}

void NuLogSetThreadSafe(NuLog_t *Log)
{
    NuLockDestroy(&(Log->Lock));
    NuLockInit(&(Log->Lock), &NuLockType_Mutex);

    return;
}

void NuLog(NuLog_t *Log, const char *Format, ...)
{
    va_list ArguList;

    va_start(ArguList, Format);
    NuLogV(Log, Format, ArguList); 
    va_end(ArguList);

    return;
}

void NuLogV(NuLog_t *Log, const char *Format, va_list ArguList)
{
    NuLogVPrintf(Log, Log->Log, Format, ArguList);

    return;
}

void NuErr(NuLog_t *Log, const char *Format, ...)
{
    va_list ArguList;

    va_start(ArguList, Format);
    NuErrV(Log, Format, ArguList); 
    va_end(ArguList);

    return;
}

void NuErrV(NuLog_t *Log, const char *Format, va_list ArguList)
{
    NuLogVPrintf(Log, Log->Err, Format, ArguList);

    return;
}

void NuLogBin(NuLog_t *Log, const void *Data, size_t DataLen)
{
    char    *Buf = Log->Bin;

    NuLockLock(&(Log->Lock));

    NuGetTime(Buf);
    *(Buf + NuLogTimeLen) = ' ';

    NuStrmWriteN(Log->FStream, Buf, NuLogPrefixLen);
    NuStrmWriteN(Log->FStream, Data, DataLen);
    NuStrmWriteN(Log->FStream, "\n", 1);

    Log->FlushFn(Log);

    NuLockUnLock(&(Log->Lock));

    return;
}

void NuLogFlush(NuLog_t *Log)
{
    NuLockLock(&(Log->Lock));
    NuStrmFlush(Log->FStream);
    NuLockUnLock(&(Log->Lock));

    return;
}


