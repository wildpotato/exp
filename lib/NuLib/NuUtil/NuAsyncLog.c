#include "NuAsyncLog.h"

/* macro/struct */
/* -------------------------------------------------------------------- */
typedef struct _NuAsyncLogBuf_t {
    struct timeval tv;
    int    logLv;          /* MSG/ERR/DEBUG  */
    size_t Length;         /* Message Length */
    char   Message[1];
} NuAsyncLogBuf_t;

/* static function     */
/* ====================================================================== */
static inline void _asyncLogV(NuAsyncLog_t *pLog, const char *fmt, va_list arguList, int logLv)
{
    size_t len = 0;
	char buffer[pLog->msgBufSz];
    NuAsyncLogBuf_t *msg = (NuAsyncLogBuf_t *)buffer;

    gettimeofday(&(msg->tv), NULL);
    msg->logLv = logLv;
    msg->Length = vsnprintf(msg->Message, pLog->msgCapacity, fmt, arguList);
    if (msg->Length >= pLog->msgCapacity) {
        msg->Length = pLog->msgCapacity - 1;
        msg->Message[pLog->msgCapacity] = '\0';
    }

    len = sizeof(NuAsyncLogBuf_t) + msg->Length;

    NuCQEnqueue(pLog->que, (const char *)msg, len);

    return;
}

static inline void __write_log(NuAsyncLog_t *pLog, const char *msg)
{
    char prefix[15+1] = {0};

    NuGetTime(prefix);
    fprintf(pLog->fhdl, "%s [MSG] %s\n", prefix, msg);
    fflush(pLog->fhdl);
}

static void *_asyncLogWorkFn(void *argu) 
{
    NuAsyncLog_t *log = (NuAsyncLog_t *)argu;

	char buffer[log->msgBufSz];
    char      *msgBuf = (char *)buffer;
    size_t     msgLen = 0;
    char prefix[22+1] = {0};

    NuAsyncLogBuf_t *msg = (NuAsyncLogBuf_t *)msgBuf;
    char        *msgType = NULL;

    strcpy(prefix, "hh:mm:ss.SSSSSS [MSG] ");
    msgType = prefix + 17;

    while (log->isWork || !NuCQIsEmpty(log->que)) {
        int iRC = NuCQDequeue(log->que, msgBuf, &msgLen);
        if (iRC < 0) {
            __write_log(log, "write log thread end.");
            break;
        }

        if (NuBITCHK(msg->logLv, enNuAsyncLogType_MSG)) {
            memcpy(msgType, "MSG", 3);
        } else if (NuBITCHK(msg->logLv, enNuAsyncLogType_ERR)) {
            memcpy(msgType, "ERR", 3);
        } else if (NuBITCHK(msg->logLv, enNuAsyncLogType_DEBUG)) {
            memcpy(msgType, "DBG", 3);
        }

        NuTimeToStr(&(msg->tv), prefix);
        prefix[15] = ' ';

        fwrite(prefix, sizeof(char), 22, log->fhdl);
        fwrite(msg->Message, sizeof(char), msg->Length, log->fhdl);
//        fprintf(log->fhdl, "%s%s", prefix, msg->Message);
        fflush(log->fhdl);
    }

    NuThdReturn();
    return NULL;
}

/* message prefix
 * char [22]
 * "hh:mm:ss.SSSSSS [MSG] "
 * */
static inline int _asyncLogOpen(NuAsyncLog_t **pLog, const char *path, const char *fileName, size_t msgBufSz, size_t msgBufCnt) 
{
    int iRC = 0;

    NuAsyncLog_t *log = NULL; 
    
    log = (NuAsyncLog_t *)malloc(sizeof(NuAsyncLog_t));
    if (log == NULL) {
        return NU_MALLOC_FAIL;
    } 
    memset(log, ' ', sizeof(NuAsyncLog_t));

    /* open log file handler */
    NuCreateRecursiveDir(path);
    NuStrNewPreAlloc(&log->file, 128);
    NuStrPrintf(log->file, 0, "%s/%s.log", path, fileName);
    log->fhdl = fopen(NuStrGet(log->file), "a+");
    if (log->fhdl == NULL) {
        NUGOTO(iRC, NU_OPENFILEFAIL, EXIT);
    }

    /* set log buffer, 22 = prefix info size */
    log->msgPrefixSz = 22;
    log->msgCapacity = msgBufSz;
    log->msgBufSz    = sizeof(NuAsyncLogBuf_t) + log->msgPrefixSz + msgBufSz;

    /* create backend thread */
    iRC = NuCQNew(&(log->que), log->msgBufSz, msgBufCnt);
    if (iRC != NU_OK) {
        log->que = NULL;
        NUGOTO(iRC, NU_RMQFAIL, EXIT);
    }

    log->isWork = true;
    iRC = NuThdCreate(&_asyncLogWorkFn, (void *)log, &(log->workThd));
    if (iRC != NU_OK) {
        NUGOTO(iRC, NU_FAIL, EXIT);
    }

    log->logLvEnable = enNuAsyncLogType_NONE;;
    NuAsynLogSetLogLv(log, enNuAsyncLogType_MSG | enNuAsyncLogType_ERR);
    
    (*pLog) = log;
    iRC = NU_OK;

EXIT:
    if(iRC < 0) {
        NuAsyncLogClose(log);
    }

    return iRC;
}

/* Log function        */
/* ====================================================================== */

int NuAsyncLogOpen(NuAsyncLog_t **pLog, const char *path, const char *fileName)
{
    return _asyncLogOpen(pLog, path, fileName, 2048, 64);
}

int NuAsyncLogOpen2(NuAsyncLog_t **pLog, const char *path, const char *fileName, size_t msgBufSz, size_t msgBufCnt)
{
    return _asyncLogOpen(pLog, path, fileName, msgBufSz, msgBufCnt);
}

void NuAsyncLogClose(NuAsyncLog_t *pLog) 
{
    if (pLog == NULL) {
        return;
    }

    pLog->isWork = false;

    while(!NuCQIsEmpty(pLog->que)) {
        usleep(1000);
    }

    NuCQClose(pLog->que);

    NuThdJoin(pLog->workThd);


    if (pLog->fhdl != NULL) {
        fclose(pLog->fhdl);
    }

    NuStrFree(pLog->file);
    NuCQFree(pLog->que);

    free(pLog);
}

void NuAsyncLogV(NuAsyncLog_t *pLog, const char *fmt, va_list arguList)
{
    _asyncLogV(pLog, fmt, arguList, enNuAsyncLogType_MSG);
}

void NuAsyncErrV(NuAsyncLog_t *pLog, const char *fmt, va_list arguList) 
{
    _asyncLogV(pLog, fmt, arguList, enNuAsyncLogType_ERR);
}

void NuAsyncDBGV(NuAsyncLog_t *pLog, const char *fmt, va_list arguList)
{
    _asyncLogV(pLog, fmt, arguList, enNuAsyncLogType_DEBUG);
}

