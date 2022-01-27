
#include "NuCommon.h"
#include "NuStr.h"

#ifndef _NUFIXREADER_H
#define _NUFIXREADER_H

#ifdef __cplusplus
extern "C" {
#endif

/* NuFixReader */
typedef struct _NuFixReader_t NuFixReader_t;
typedef struct _NuFixReaderNode_t NuFixReaderNode_t;

int NuFixReaderNew(NuFixReader_t **Reader);
void NuFixReaderFree(NuFixReader_t *Reader);

typedef bool (*NuFixReaderFn)(const char *Tag, size_t TagLen, const char *Value, size_t ValueLen, void *Argu, void *Closure);
NuFixReaderNode_t *NuFixReaderGetNode(NuFixReader_t *pReader, char *key, unsigned int key_len);
void NuFixReaderSetCallback(NuFixReaderNode_t *Node, NuFixReaderFn Fn, void *Argu);
void NuFixReaderRemoveCallback(NuFixReaderNode_t *Node);
bool NuFixReaderForEachWithCB(NuFixReader_t *pReader, const char *Msg, int Len, NuFixReaderFn DefaultFn, void *Closure);
bool NuFixReaderForEach(NuFixReader_t *pReader, const char *Msg, int Len, void *Closure);



#ifdef __cplusplus
}
#endif

#endif /* _NUFIXREADER_H */


