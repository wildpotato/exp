
#include "NuCommon.h"

#ifndef _NUSTR_H
#define _NUSTR_H

#ifdef __cplusplus
extern "C" {
#endif

/* extensible string */
typedef struct _NuStr_t NuStr_t;

int NuStrNewPreAlloc(NuStr_t **pStr, size_t Len);
int NuStrNew(NuStr_t **pStr, const char *Value);
void NuStrFree(NuStr_t *Str);
void NuStrClear(NuStr_t *Str);

void NuStrCat(NuStr_t *Str, const char *Src);
void NuStrNCat(NuStr_t *Str, const void *Data, size_t DataLen);
void NuStrCatChr(NuStr_t *Str, char Chr);

void NuStrNCpy(NuStr_t *Str, const void *Data, size_t DataLen);
void NuStrCpy(NuStr_t *Str, const char *Src);

int NuStrPrintf(NuStr_t *Str, int Offset, const char *Format, ...);
int NuStrVPrintf(NuStr_t *Str, int Offset, const char *Format, va_list ArguList);
void NuStrSetChr(NuStr_t *Str, int Idx, char Chr);

void NuStrAppendPrintf(NuStr_t *Str, const char *Format, ...);

/* NuStr tools */
int NuStrCmp(NuStr_t *Str1, NuStr_t *Str2);
void NuStrRTrimChr(NuStr_t *Str, char Chr);
void NuStrRTrim(NuStr_t *Str);
void NuStrLTrim(NuStr_t *Str);
void NuStrReplaceRangeChr(NuStr_t *Str, char Chr, char NewChr, size_t Start, size_t Len);
void NuStrReplaceChr(NuStr_t *Str, char Chr, char NewChr);
const char *NuStrGet(NuStr_t *Str);
char NuStrGetChr(NuStr_t *Str, int Idx);
size_t NuStrSize(NuStr_t *Str);
void NuStrAppendDump(NuStr_t *Str, void *Data, size_t Len);

#define NuStrGetInt(NUSTR)      strtol(NuStrGet(NUSTR), NULL, 10)
#define NuStrGetDouble(NUSTR)   strtod(NuStrGet(NUSTR), NULL)

#ifdef __cplusplus
}
#endif

#endif /* _NUSTR_H */

