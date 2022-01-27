
#include "NuFileStream.h"

struct _NuFileStream_t
{
    NuStr_t     *pFile;
    NuStr_t     *Mode;
    FILE        *FHdl;
    size_t      lPos;
    struct stat FileStat;
    NuEventFn   FlushFn;
};
#define NuFileStreamSz   sizeof(NuFileStream_t)

int NuFStreamOpen(NuFileStream_t **pFobj, const char *pFile, const char *mode)
{
    int RC = NU_OK;

    if(!((*pFobj) = (NuFileStream_t *)malloc(sizeof(NuFileStream_t))))
    {
        NUGOTO(RC, NU_MALLOC_FAIL, EXIT);
    }

    memset((*pFobj), 0, sizeof(NuFileStream_t));
    NuStrNew(&((*pFobj)->pFile), pFile);

    NuStrNew(&((*pFobj)->Mode), mode);

    (*pFobj)->lPos = 0;
    (*pFobj)->FlushFn = &NuEventFn_Default;

    if(!((*pFobj)->FHdl = fopen(pFile, mode)))
    {
        NUGOTO(RC, NU_OPENFILEFAIL, EXIT);
    }

    if(stat(NuStrGet((*pFobj)->pFile), &((*pFobj)->FileStat)) < 0)
    {
        NUGOTO(RC, NU_FILEUNSTABLE, EXIT);
    }

EXIT:
    if(RC < 0)
    {
        NuFStreamClose(*pFobj);
        (*pFobj) = NULL;
    }

    return RC;
}

void NuFStreamClose(NuFileStream_t *pFobj)
{
    if(pFobj != NULL)
    {
        if(pFobj->FHdl != NULL)
        {
            fclose(pFobj->FHdl);
        }

        if(pFobj->pFile != NULL)
        {
            NuStrFree(pFobj->pFile);
        }

        if(pFobj->Mode != NULL)
        {
            NuStrFree(pFobj->Mode);
        }

        free(pFobj);
    }

    return;
}

int NuFStreamReOpen(NuFileStream_t *pFobj)
{
    FILE        *pFHdlr = NULL;
    const char  *pFilePath = NuStrGet(pFobj->pFile);

//    if(pFHdlr != NULL)
//    {
//        fclose(pFHdlr);
//        pFobj->FHdl = NULL;
//    }

    pFobj->lPos = 0;
    if(!(pFHdlr = pFobj->FHdl = fopen(pFilePath, NuStrGet(pFobj->Mode))))
    {
        return NU_OPENFILEFAIL;
    }

    if(stat(pFilePath, &(pFobj->FileStat)) < 0)
    {
        fclose(pFHdlr);
        return NU_FILEUNSTABLE;
    }

    return NU_OK;
}

static void _FlushFn(void *Argu)
{
    fflush(((NuFileStream_t *)Argu)->FHdl);

    return;
}

void NuFStreamSetAutoFlush(NuFileStream_t *pFobj, bool AutoFlush)
{
    if(AutoFlush)
    {
        pFobj->FlushFn = &_FlushFn;
    }
    else
    {
        pFobj->FlushFn = &NuEventFn_Default;
    }

    return;
}

size_t NuFStreamReadN(NuFileStream_t *FileStream, void *Buffer, size_t Bytes)
{
    return fread(Buffer, sizeof(char), Bytes, FileStream->FHdl);
}

size_t NuFStreamReadN_Block(NuFileStream_t *pFobj, char *pStr, size_t nBytes)
{
    size_t readn = 0;
    char   *p    = pStr;

    do
    {
		int chr = 0;
        chr = fgetc(pFobj->FHdl);
        if(chr == EOF)
        {
            pStr[readn] = '\0';
            fseek(pFobj->FHdl, (pFobj->lPos + readn), SEEK_SET);
            sleep(1);
        }
        else
        {
            *p = chr;
            readn++;
            p++;
        }
    }
    while(nBytes > readn);

    pFobj->lPos += readn;
    pStr[readn] = '\0';
    return readn;
}

int NuFStreamReadLine(NuFileStream_t *pFobj, char *pStr, size_t StrLen)
{
    int     chr = 0;
    long    pos_cur = NuFStreamTell(pFobj);
    size_t  Len = StrLen;

    do
    {
        if((chr = fgetc(pFobj->FHdl)) == EOF)
        {
            /* think ?
             * must seek to position that before read?
             * */
            NuFStreamSeek(pFobj, pos_cur, SEEK_SET);
            return NU_READFAIL;
        }
        
        *pStr = chr;

        ++ pStr;
        -- Len;
    }
    while((chr != NULINE_END) && Len);

    *pStr = '\0';
    return StrLen - Len;
}

int NuFStreamReadLineByStr(NuFileStream_t *pFobj, NuStr_t *pStr)
{
    int    chr = 0;
    long   pos_cur = NuFStreamTell(pFobj);

    NuStrClear(pStr);
    do
    {
        if ((chr = fgetc(pFobj->FHdl)) == EOF)
        {
            /* think ?
             * must seek to position that before read?
             * */
            NuFStreamSeek(pFobj, pos_cur, SEEK_SET);
            return NU_READFAIL;
        }

        NuStrCatChr(pStr, chr);

    }
    while((chr != NULINE_END));

    return NuStrSize(pStr);
}

int NuFStreamWriteLineN(NuFileStream_t *pFobj, const char *pStr, size_t nBytes)
{
    size_t writen = fwrite( pStr, sizeof(char), nBytes, pFobj->FHdl);

    if(!writen)
    {
        return NU_OK;
    }
    else if(writen == EOF)
    {
        return NU_WRITEFAIL;
    }

    writen += (int)fputc( NULINE_END, pFobj->FHdl);

    pFobj->FlushFn(pFobj);

    pFobj->lPos += writen;

    return writen;
}

int NuFStreamWriteN(NuFileStream_t *pFobj, const char *pStr, size_t nBytes)
{
    size_t writen = fwrite( pStr, sizeof(char), nBytes, pFobj->FHdl);

    if(!writen)
    {
        return NU_OK;
    }
    else if(writen == EOF)
    {
        return NU_WRITEFAIL;
    }

    pFobj->FlushFn(pFobj);

    pFobj->lPos += writen;

    return writen;
}

int NuFStreamPutC(NuFileStream_t *pFobj, char pChr)
{
    size_t writen = fputc( pChr, pFobj->FHdl);

    if(!writen)
    {
        return NU_OK;
    }
    else if(writen == EOF)
    {
        return NU_WRITEFAIL;
    }

    pFobj->FlushFn(pFobj);

    pFobj->lPos += writen;

    return writen;
}

int NuFStreamVPrintf(NuFileStream_t *pFobj, const char *fmt, va_list ap)
{
    size_t writen = vfprintf(pFobj->FHdl, fmt, ap);

    pFobj->FlushFn(pFobj);

    pFobj->lPos += writen;

    return writen;
}

int NuFStreamPrintf(NuFileStream_t *pFobj, const char *fmt, ...)
{
    va_list arg;
    size_t  cnt = 0;

    va_start(arg, fmt);
    cnt = NuFStreamVPrintf(pFobj, fmt, arg);
    va_end(arg);

    return cnt;
}

void NuFStreamFlush(NuFileStream_t *pFobj)
{
    fflush(pFobj->FHdl);
}

int  NuFStreamSeek(NuFileStream_t *pFobj, long offset, int whence)
{
    return fseek(pFobj->FHdl, offset, whence);
}

long NuFStreamTell(NuFileStream_t *pFobj)
{
    return (pFobj == NULL) ? 0 : ftell(pFobj->FHdl);
}

int NuFStreamCheck(NuFileStream_t *pFobj)
{
    struct stat Stat;
    struct stat *pStat = &(pFobj->FileStat);

    if(stat(NuStrGet(pFobj->pFile), &Stat) < 0)
    {
        return NU_FILENOTEXIST;
    }

    if((Stat.st_ino != pStat->st_ino) || (Stat.st_dev != pStat->st_dev))
    {
        return NU_FILEUNSTABLE;
    }

    return NU_OK;
}

int NuFStreamLastModifyTime(NuFileStream_t *pFobj, char *pTime)
{
    struct stat Stat;

    if(stat(NuStrGet(pFobj->pFile), &Stat) < 0)
    {
        return NU_FAIL;
    }

    strftime(pTime, 18, "%Y%m%d %H:%M:%S", localtime(&(Stat.st_mtime)));
    return NU_OK;
}

void NuFStreamDump(NuFileStream_t *pFobj, char *pData, size_t DataLen)
{
    char  szHex[100] = {0};
    char  szTmp[100] = {0};
    char  szWrite[250] = {0};
    char *ptr        = NULL;
    char *pHex       = szHex;
    char *pTmp       = szTmp;
    int   i          = 0;
    int   j          = 1;

    ptr = pData;

    for (i = 0; i < DataLen; i++)
    {
        if ( (j != 20) && (i != DataLen - 1) )
        {
            pHex += sprintf(pHex, "%02x ", *ptr);

            if ((*ptr == 0x0a) || (*ptr == 0x0d))
                pTmp += sprintf(pTmp, "%s ", "\\n");
            else if ((*ptr = 0x00))
                pTmp += sprintf(pTmp, "%s ", ".");
            else
                pTmp += sprintf(pTmp, "%s ", ".");

            j++;
        }
        else
        {
            snprintf(szWrite, sizeof(szWrite), "%s     %s", szHex, szTmp);
            if (pFobj == NULL)
                printf("%s\n", szWrite);
            else
                fputs(szWrite, pFobj->FHdl);

            j = 1;
            szHex[0] = '\0';
            szTmp[0] = '\0';

            pHex = szHex;
            pTmp = szTmp;
        }

        ptr++;
    }

    return;
}

bool NuFStreamIsEOF(NuFileStream_t *pFobj)
{
    return feof(pFobj->FHdl);
}

const char *NuFStreamGetPath(NuFileStream_t *pFobj)
{
    return NuStrGet(pFobj->pFile);
}

int NuFStreamGetFD(NuFileStream_t *pFobj)
{
    return fileno(pFobj->FHdl);
}

size_t NuFStreamGetSize(NuFileStream_t *pFobj)
{
    return NuFileGetSize(NuFStreamGetFD(pFobj));
}

