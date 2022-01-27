

#ifndef _NUASYNCLOG_H
#define _NUASYNCLOG_H

#include "NuCommon.h"
#include "NuTime.h"
#include "NuFile.h"
#include "NuStr.h"
#include "NuThread.h"
#include "NuCircularQ.h"

#ifdef __cplusplus
extern "C" {
#endif

enum enNuAsyncLogType {
    enNuAsyncLogType_NONE  = 0,
    enNuAsyncLogType_MSG   = 1 << 0,
    enNuAsyncLogType_ERR   = 1 << 1,
    enNuAsyncLogType_DEBUG = 1 << 2
};

typedef struct _NuAsyncLog_t {
    bool       isWork;
    FILE       *fhdl;
    NuStr_t    *file;
    NuCQ_t     *que;

    NuThread_t workThd;
    size_t     msgPrefixSz;
    size_t     msgBufSz;
    size_t     msgCapacity;
    int        logLvEnable; /* default MSG/ERR */
} NuAsyncLog_t;

/* default msgBufSz = 2048, msgBufCnt = 64 */
int NuAsyncLogOpen(NuAsyncLog_t **pLog, const char *path, const char *fileName);
int NuAsyncLogOpen2(NuAsyncLog_t **pLog, const char *path, const char *fileName, size_t msgBufSz, size_t msgBufCnt);
void NuAsyncLogClose(NuAsyncLog_t *pLog);

void NuAsyncLogV(NuAsyncLog_t *pLog, const char *fmt, va_list arguList);
void NuAsyncErrV(NuAsyncLog_t *pLog, const char *fmt, va_list arguList);
void NuAsyncDBGV(NuAsyncLog_t *pLog, const char *fmt, va_list arguList);

static inline void NuAsyncLog(NuAsyncLog_t *pLog, const char *fmt, ...) {
    if (NuBITCHK(pLog->logLvEnable, enNuAsyncLogType_MSG)) {
        va_list arguList;
        va_start(arguList, fmt);
        NuAsyncLogV(pLog, fmt, arguList);
        va_end(arguList);
    }
}

static inline void NuAsyncErr(NuAsyncLog_t *pLog, const char *fmt, ...) {
    if (NuBITCHK(pLog->logLvEnable, enNuAsyncLogType_ERR)) {
        va_list arguList;
        va_start(arguList, fmt);
        NuAsyncErrV(pLog, fmt, arguList);
        va_end(arguList);
    }
}

static inline void NuAsyncDBG(NuAsyncLog_t *pLog, const char *fmt, ...) {
    if (NuBITCHK(pLog->logLvEnable, enNuAsyncLogType_DEBUG)) {
        va_list arguList;
        va_start(arguList, fmt);
        NuAsyncDBGV(pLog, fmt, arguList);
        va_end(arguList);
    }
}

static inline void NuAsynLogSetLogLv(NuAsyncLog_t *pLog, int lv) {
    if (pLog != NULL) {
        pLog->logLvEnable = lv;
    }
}

#ifdef __cplusplus
}
#endif

#endif /* _NUASYNCLOG_H */

