#include "NuUtil.h"
#include "NuFStr.h"

#define _FSTRCLEAR(STR) do {\
    (STR)->pW = (STR)->strBuf; \
    (STR)->strBuf[0] = '\0'; \
}while(0)

int NuFStrNew(NuFStr_t **str, size_t len)
{
    int RC = NU_OK;

    if(!((*str) = (NuFStr_t *)malloc(sizeof(NuFStr_t))))
    {
        NUGOTO(RC, NU_MALLOC_FAIL, EXIT);
    }

    (*str)->strBufSz = NU_ALIGN8(len);
    (*str)->strBuf = malloc(sizeof(char) * (*str)->strBufSz);

    if ((*str)->strBuf == NULL)
    {
        NUGOTO(RC, NU_MALLOC_FAIL, EXIT);
    }
   
    (*str)->pW = (*str)->strBuf;
EXIT:
    if(RC < 0)
    {
        NuFStrFree(*str);
    }

    return RC;
}

void NuFStrFree(NuFStr_t *str)
{
    if(str != NULL)
    {
        if(str->strBuf != NULL)
        {
            free(str->strBuf);
        }

        free(str);
    }

    return;
}

void NuFStrClear(NuFStr_t *str)
{
    _FSTRCLEAR(str);
    return;
}

void NuFStrChkSz(NuFStr_t *str, size_t len) 
{
    char *ptr = str->strBuf;
    size_t strSz = str->pW - str->strBuf;
    size_t needN = strSz + len + 1;

    if (unlikely(needN < str->strBufSz)) {

        needN = NU_RALIGN_PAGE(needN);
        str->strBuf = (char *)realloc(str->strBuf, sizeof(char) * needN);
        if (str->strBuf == NULL) {
            str->strBuf = ptr;
        } else {
            str->strBufSz = needN;
        }
        str->pW = str->strBuf + strSz;
    }
}

void NuFStrNCat(NuFStr_t *str, const void *data, size_t dataLen)
{
    memcpy(str->pW, data, dataLen);
    str->pW += dataLen;
    str->pW[0] = '\0'; 
    return;
}

void NuFStrCatChr(NuFStr_t *str, char chr)
{
    str->pW[0] = chr;
    str->pW += 1;
    str->pW[0] = '\0'; 
    return;
}

void NuFStrNCpy(NuFStr_t *str, const void *data, size_t dataLen)
{
    _FSTRCLEAR(str);
    NuFStrNCat(str, data, dataLen);
    return;
}

void NuFStrAppendPrintf(NuFStr_t *str, const char *fmt, ...)
{
    va_list arguList;

    va_start(arguList, fmt);
    str->pW += vsprintf(str->pW, fmt, arguList);
    va_end(arguList);

    return;
}

int NuFStrPrintf(NuFStr_t *str, const char *fmt, ...)
{
    int Len = 0;

    va_list arguList;
    va_start(arguList, fmt);
    Len = vsprintf(str->strBuf, fmt, arguList);
    va_end(arguList);

    if (Len < 0)
    {
        _FSTRCLEAR(str);
    }
    else
    {
        str->pW = str->strBuf + Len;
    }

    return Len;
}

