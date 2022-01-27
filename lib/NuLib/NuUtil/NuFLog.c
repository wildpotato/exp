
//#include "NuLock.h"
#include <pthread.h>
#include "NuTime.h"
#include "NuFile.h"
#include "NuStr.h"
#include "NuFLog.h"

/* macro */
/* -------------------------------------------------------------------- */
#define NuFLogTime           "hh:mm:ss.SSSSSS"
#define NuFLogLogTag         NuFLogTime" [MSG] Msg"
#define NuFLogErrorTag       NuFLogTime" [ERR] Msg"
#define NuFLogBinTag         NuFLogTime" [WRT] Msg"

#define NuFLogTimeLen        sizeof(NuFLogTime) - 1
#define NuFLogTagLen         sizeof(" [TAG] ") - 1

#define NuFLogPrefixLen      NuFLogTimeLen + NuFLogTagLen

#define NuMBSz              (1024 * 1024)

struct _NuFLog_t
{
    FILE              *FHdl;
    pthread_mutex_t   mtx;

    bool              isAutoFlush;
    bool              isThreadSafe;

    NuStr_t           *File;
    NuStr_t           *Log;
    NuStr_t           *Err;
    char              Bin[NuFLogPrefixLen];

};

/* static function     */
/* ====================================================================== */

static void NuFLogBufInit(NuStr_t **Str, const char *Prefix)
{
    NuStrNew(Str, Prefix);

    return;
}

static void NuFLogInitFile(NuStr_t **Str, const char *Path, const char *FileName) 
{
    NuCreateRecursiveDir(Path);
    NuStrNewPreAlloc(Str, 128);
    NuStrPrintf(*Str, 0, "%s/%s.log", Path, FileName);
    return;
}

static inline void _NuFLogLock(NuFLog_t *Log) 
{
    if (Log->isThreadSafe)
    {
        pthread_mutex_lock(&(Log->mtx));
    }
}

static inline void _NuFLogUnLock(NuFLog_t *Log)
{
    if (Log->isThreadSafe)
    {
        pthread_mutex_unlock(&(Log->mtx));
    }
}

static inline void _NuFLogFlush(NuFLog_t *Log)
{
    if (Log->isAutoFlush)
    {
        fflush(Log->FHdl);
    }
}

static inline int _NuFLogOpen(NuFLog_t **pLog, const char *Path, const char *FileName, bool AutoFlush)
{
    int RC = 0;
    (*pLog) = NULL;

    if(!((*pLog) = (NuFLog_t *)malloc(sizeof(NuFLog_t))))
    {
        NUGOTO(RC, NU_MALLOC_FAIL, EXIT);  
    }

    (*pLog)->FHdl = NULL;

    (*pLog)->isAutoFlush = AutoFlush;
    (*pLog)->isThreadSafe = false;

    NuFLogInitFile(&((*pLog)->File), Path, FileName);

    if(!((*pLog)->FHdl = fopen(NuStrGet((*pLog)->File), "a+")))
    {
        NUGOTO(RC, NU_OPENFILEFAIL, EXIT);
    }

    NuFLogBufInit(&((*pLog)->Log), NuFLogLogTag);
    NuFLogBufInit(&((*pLog)->Err), NuFLogErrorTag);
    memcpy((*pLog)->Bin, NuFLogBinTag, NuFLogPrefixLen);

    pthread_mutex_init(&(*pLog)->mtx, NULL);

    RC = NU_OK;

EXIT:
    if(RC < 0)
    {
        NuFLogClose((*pLog));
    }

    return RC;
}

static inline void NuFLogVPrintf(NuFLog_t *Log, NuStr_t *Str, const char *Format, va_list ArguList)
{
    _NuFLogLock(Log);

    NuGetTime((char *)NuStrGet(Str));
    NuStrSetChr(Str, NuFLogTimeLen, ' ');

    fwrite(NuStrGet(Str), sizeof(char), NuFLogPrefixLen, Log->FHdl);
    vfprintf(Log->FHdl, Format, ArguList);

    _NuFLogFlush(Log);
    _NuFLogUnLock(Log);

    return;
}

/* Log function        */
/* ====================================================================== */
int NuFLogOpen(NuFLog_t **pLog, const char *Path, const char *FileName)
{
    return _NuFLogOpen(pLog, Path, FileName, false);
}

void NuFLogClose(NuFLog_t *Log)
{
    if(Log != NULL)
    {
        if(Log->FHdl != NULL)
        {
            fclose(Log->FHdl);
        }

        NuStrFree(Log->File);
	    NuStrFree(Log->Log);
	    NuStrFree(Log->Err);

        pthread_mutex_destroy(&(Log->mtx));

        free(Log);
    }

    return;
}

void NuFLogSetAutoFlush(NuFLog_t *Log)
{
    Log->isAutoFlush = true;
    return;
}

void NuFLogSetThreadSafe(NuFLog_t *Log)
{
    Log->isThreadSafe = true;
    return;
}

void NuFLog(NuFLog_t *Log, const char *Format, ...)
{
    va_list ArguList;

    va_start(ArguList, Format);
    NuFLogVPrintf(Log, Log->Log, Format, ArguList);
    va_end(ArguList);

    return;
}

void NuFLogV(NuFLog_t *Log, const char *Format, va_list ArguList)
{
    NuFLogVPrintf(Log, Log->Log, Format, ArguList);

    return;
}

void NuFErr(NuFLog_t *Log, const char *Format, ...)
{
    va_list ArguList;

    va_start(ArguList, Format);
    NuFLogVPrintf(Log, Log->Err, Format, ArguList);
    va_end(ArguList);

    return;
}

void NuFErrV(NuFLog_t *Log, const char *Format, va_list ArguList)
{
    NuFLogVPrintf(Log, Log->Err, Format, ArguList);

    return;
}

void NuFLogBin(NuFLog_t *Log, const void *Data, size_t DataLen)
{
    char    *Buf = Log->Bin;

    _NuFLogLock(Log);

    NuGetTime(Buf);
    *(Buf + NuFLogTimeLen) = ' ';
    
    fwrite(Buf, sizeof(char), NuFLogPrefixLen, Log->FHdl);
    fwrite(Data, sizeof(char), DataLen, Log->FHdl);
    fwrite("\n", sizeof(char), 1, Log->FHdl);

    _NuFLogFlush(Log);

    _NuFLogUnLock(Log);

    return;
}

void NuFLogFlush(NuFLog_t *Log)
{
    _NuFLogLock(Log);
    fflush(Log->FHdl);
    _NuFLogUnLock(Log);
    return;
}


