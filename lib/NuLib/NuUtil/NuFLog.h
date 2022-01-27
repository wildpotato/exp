
#include "NuCommon.h"

#ifndef _NUFLOG_H
#define _NUFLOG_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _NuFLog_t NuFLog_t;

int NuFLogOpen(NuFLog_t **pLog, const char *Path, const char *FileName);

void NuFLogClose(NuFLog_t *Log);
void NuFLogSetAutoFlush(NuFLog_t *Log);
void NuFLogSetThreadSafe(NuFLog_t *Log);

void NuFLog(NuFLog_t *Log, const char *Format, ...);
void NuFLogV(NuFLog_t *Log, const char *Format, va_list ArguList);

void NuFErr(NuFLog_t *Log, const char *Format, ...);
void NuFErrV(NuFLog_t *Log, const char *Format, va_list ArguList);

void NuFLogFlush(NuFLog_t *Log);

void NuFLogBin(NuFLog_t *Log, const void *Data, size_t DataLen);

#ifdef __cplusplus
}
#endif

#endif /* _NUFLOG_H */

