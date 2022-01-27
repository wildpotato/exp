#ifndef _NUSTREAM_H
#define _NUSTREAM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "NuUtil.h"
#include "NuStr.h"
#include "NuCStr.h"
#include "NuTime.h"
#include "NuFile.h"

/* -----------------------------------
 * enumerator
 * ----------------------------------- */
typedef enum _StreamType
{
	enMMapStream = 0,
	enFileStream     
} StremType;
/* -----------------------------------
 * define 
 * ----------------------------------- */
typedef int    (*StrmCB_Fn)(void *);
typedef int    (*StrmAllocCB_Fn)(void *, const char *, size_t);
typedef size_t (*StrmWrtCB_Fn)(void *, const void *, size_t);
typedef size_t (*StrmReadCB_Fn)(void *, size_t, void **);
typedef size_t (*StrmVPrintfCB_Fn)(void *, const char *, va_list);

/* internal */
typedef int    (*_WrtNCB_Fn)(void *, const void *, size_t);

typedef struct _NuStrmFnType_t
{
	StrmCB_Fn      Flush;
	StrmCB_Fn      SeekToEnd;
	StrmCB_Fn      Free;
	StrmWrtCB_Fn   WriteN;
	StrmAllocCB_Fn Alloc;
    StrmVPrintfCB_Fn VPrintf;
} NuStrmFnType_t;
/* ----------------------------------- */

/** 
 * A structure to rotate
 */ 
typedef struct _NuStrm_t
{
	base_vector_t       vStrm;              /* stream list */
	void                *Stream;             /* Current Stream */
	int                 StreamType;
	size_t              AllocSize;
	size_t              LeftSz;
	int                 MaxVer; 

	NuStr_t             *Name;
	NuStr_t             *Dir;
	NuStr_t             *File;

	StrmWrtCB_Fn        CB_WriteN;          /* callback : Write n byte data to stream */
    StrmVPrintfCB_Fn    CB_VPrintf;

	StrmCB_Fn           CB_Flush;           /* callback : Strm flow, please implement stream flush  */
	StrmCB_Fn           CB_SeekToEnd;       /* callback : Strm flow, please implement stream flush  */

	StrmAllocCB_Fn      CB_Alloc;
	StrmCB_Fn           CB_Free;

	_WrtNCB_Fn           CB_IntWriteN;
} NuStrm_t;

int NuStrmNew(NuStrm_t **pStrm, int StreamType, size_t AllocSize, const char *Dir, const char *Name);
void NuStrmFree(NuStrm_t *Strm);
size_t NuStrmGetAllocSize(NuStrm_t *Strm);
int NuStrmWriteN(NuStrm_t *Strm, const void *Data, size_t DataLen);
int NuStrmFlush(NuStrm_t *Strm);

int NuStrmVPrintf(NuStrm_t *Strm, const void *fmt, va_list ap);

#ifdef __cplusplus
}
#endif

#endif /* _NUSTREAM_H */

