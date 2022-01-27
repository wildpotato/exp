
#include "NuCommon.h"

#ifndef _NUFSTR_H
#define _NUFSTR_H

#ifdef __cplusplus
extern "C" {
#endif

/* fast extensible string 
 * please check string buffer is enough with NuFStrChkSz. 
 * this object not control buffer auto extenstion, please control it by ap with NuFStrChkSz.
 * */
typedef struct _NuFStr_t
{
    char    *strBuf;
    size_t  strBufSz;
    char    *pW;
} NuFStr_t;

int NuFStrNew(NuFStr_t **pstr, size_t Len);
void NuFStrFree(NuFStr_t *str);
void NuFStrClear(NuFStr_t *str);

void NuFStrChkSz(NuFStr_t *str, size_t len) ;

void NuFStrNCat(NuFStr_t *str, const void *data, size_t datalen);
void NuFStrCatChr(NuFStr_t *str, char chr);

void NuFStrNCpy(NuFStr_t *str, const void *data, size_t datalen);

int NuFStrPrintf(NuFStr_t *str, const char *fmt, ...);
void NuFStrAppendPrintf(NuFStr_t *str, const char *fmt, ...);

static inline const char *NuFStrGet(NuFStr_t *str) {
    return str->strBuf;
}

static inline size_t NuFStrSize(NuFStr_t *str) {
    return str->pW - str->strBuf;
}



#ifdef __cplusplus
}
#endif

#endif /* _NUFSTR_H */

