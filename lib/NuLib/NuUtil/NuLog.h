
#include "NuCommon.h"

#ifndef _NULOG_H
#define _NULOG_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _NuLog_t NuLog_t;

int NuLogOpen(NuLog_t **pLog, const char *Path, const char *FileName);
int NuLogOpen2(NuLog_t **pLog, const char *Path, const char *FileName, bool AutoFlush);
int NuLogOpen3(NuLog_t **pLog, const char *Path, const char *FileName, bool AutoFlush, size_t FileSz, int StreamType);

void NuLogClose(NuLog_t *Log);
void NuLogSetAutoFlush(NuLog_t *Log);
void NuLogSetThreadSafe(NuLog_t *Log);

void NuLog(NuLog_t *Log, const char *Format, ...);
void NuLogV(NuLog_t *Log, const char *Format, va_list ArguList);

void NuErr(NuLog_t *Log, const char *Format, ...);
void NuErrV(NuLog_t *Log, const char *Format, va_list ArguList);

void NuLogFlush(NuLog_t *Log);

void NuLogBin(NuLog_t *Log, const void *Data, size_t DataLen);

#ifdef __cplusplus
}
#endif

#endif /* _NULOG_H */

