
#include "NuCommon.h"
#include "NuLock.h"
#include "NuMMap.h"

#ifndef _NUSEQNO_H
#define _NUSEQNO_H

#ifdef __cplusplus
extern "C" {
#endif

/* struct define                        */
/* ------------------------------------ */
typedef struct _NuSeqNo_t NuSeqNo_t;

typedef int (*NuSeqNoFn)(NuSeqNo_t *SeqNo, void *Argu);
typedef int (*NuSeqNoNextFn)(const void *CurSeqNo, int SeqNoLen, void *NextSeqNo, void *Argu);
typedef int (*NuSeqNoCmpFn)(const void *SeqNo1, int SeqNoLen, const void *SeqNo2);

typedef struct _NuSeqNoType_t
{
    NuSeqNoFn     SeqNoInit;
    NuSeqNoFn     SetMaxNo;
    NuSeqNoFn     SetMinNo;
    NuSeqNoFn     SetCurNo;
    NuSeqNoFn     GetCurNo;
    NuSeqNoNextFn Next;
    NuSeqNoCmpFn  Compare;
    NuSeqNoFn     SeqNoPop;
    NuSeqNoFn     SeqNoPush;
} NuSeqNoType_t;

struct _NuSeqNo_t
{
    char          *pSeqFilePos1;
    char          *pSeqFilePos2;
    NuMMap_t      *pSeqFile;
    NuLock_t      Lock;
    NuStr_t       *pName;
    NuStr_t       *pPath;
    void          *pCurSeqNo;
    void          *pMaxSeqNo;
    void          *pMinSeqNo;
    size_t        Length;
    NuSeqNoType_t Type;
    void          *Argu;
};

/* ------------------------------------ */
/* global variable                      */
/* ------------------------------------ */
extern NuSeqNoType_t NuSeqNoType_Int;
extern NuSeqNoType_t NuSeqNoType_String;
/* ------------------------------------ */
/* global function                      */
/* ------------------------------------ */
int  NuSeqNoNew(NuSeqNo_t **pSeqNo, NuSeqNoType_t *Type, int SeqNoLen, const char *FilePath, const char *Name);
int  NuSeqNoNew2(NuSeqNo_t **pSeqNo, NuSeqNoType_t *Type, int SeqNoLen, const char *FilePath, const char *Name, NuSeqNoNextFn Next_fn, NuSeqNoCmpFn Compare_fn, void *Argu);
void NuSeqNoFree(NuSeqNo_t *SeqNo);

void NuSeqNoSetThreadSafe(NuSeqNo_t *SeqNo);

void NuSeqNoSetMaxNo(NuSeqNo_t *SeqNo, void *MaxSeqNo);
void NuSeqNoSetMinNo(NuSeqNo_t *SeqNo, void *MinSeqNo);
void NuSeqNoSetCurNo(NuSeqNo_t *SeqNo, void *CurrSeqNo);

int NuSeqNoGetCurNo(NuSeqNo_t *SeqNo, void *CurrSeqNo);

int  NuSeqNoPop(NuSeqNo_t *SeqNo, void *PopSeqNo);
int  NuSeqNoPush(NuSeqNo_t *SeqNo, void *PushSeqNo);

int NuSeqNoGetLen(NuSeqNo_t *SeqNo);

#ifdef __cplusplus
}
#endif

#endif /* _NUSEQNO_H */

