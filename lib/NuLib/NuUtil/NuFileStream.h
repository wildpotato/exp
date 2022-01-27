
#include "NuFile.h"
#include "NuStream.h"

#ifndef _NUFILESTREAM_H
#define _NUFILESTREAM_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _NuFileStream_t NuFileStream_t;

int NuFStreamOpen(NuFileStream_t **pFobj, const char *pFile, const char *mode);
void NuFStreamClose(NuFileStream_t *pFobj);
int NuFStreamReOpen(NuFileStream_t *pFobj);

void NuFStreamSetAutoFlush(NuFileStream_t *pFobj, bool AutoFlush);

size_t NuFStreamReadN(NuFileStream_t *FileStream, void *Buffer, size_t Bytes);
size_t NuFStreamReadN_Block(NuFileStream_t *pFobj, char *pStr, size_t nBytes);

int NuFStreamReadLine(NuFileStream_t *pFobj, char *pStr, size_t StrLen);
int NuFStreamReadLineByStr(NuFileStream_t *pFobj, NuStr_t *pStr);

int NuFStreamWriteLineN(NuFileStream_t *pFobj, const char *pStr, size_t nBytes);
int NuFStreamWriteN(NuFileStream_t *pFobj, const char *pStr, size_t nBytes);
int NuFStreamPutC(NuFileStream_t *pFobj, char pChr);

int NuFStreamVPrintf(NuFileStream_t *pFobj, const char *fmt, va_list ap);
int NuFStreamPrintf(NuFileStream_t *pFobj, const char *fmt, ...);

void NuFStreamFlush(NuFileStream_t *pFobj);

int  NuFStreamSeek(NuFileStream_t *pFobj, long offset, int whence);
long NuFStreamTell(NuFileStream_t *pFobj);

int NuFStreamCheck(NuFileStream_t *pFobj);

int NuFStreamLastModifyTime(NuFileStream_t *pFobj, char *pTime);

/* dump pData to file */
void NuFStreamDump(NuFileStream_t *pFobj, char *pData, size_t DataLen);

bool NuFStreamIsEOF(NuFileStream_t *pFobj);
const char *NuFStreamGetPath(NuFileStream_t *pFobj);
int NuFStreamGetFD(NuFileStream_t *pFobj);
size_t NuFStreamGetSize(NuFileStream_t *pFobj);


#ifdef __cplusplus
}
#endif

#endif /* _NUFILESTREAM_H */

