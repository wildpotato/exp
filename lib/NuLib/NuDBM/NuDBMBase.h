
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <math.h>
#include <ctype.h>

#include "NuHangerQ.h"
#include "NuCommon.h"
#include "NuUtil.h"
#include "NuFileStream.h" 
#include "NuHash.h"
#include "NuBuffer.h"
#include "NuLock.h"
#include "NuBlocking.h"
#include "NuThread.h"
#include "NuCStr.h"
#include "NuTimer.h"
#include "NuCBMgr.h"
#include "NuDBMError.h"
#include "NuDBM.h"

#ifndef _NUDBMBASE_H
#define _NUDBMBASE_H

#ifdef __cplusplus
extern "C" {
#endif

/* DBM                                        */
/* ------------------------------------------ */
typedef struct _NuDBM_t
{
    base_vector_t           *TableVec;            /* Pointer to NuTable_t */
    NuDBMConnection_t       CurrConnID;           /* DBM current connection ID */
    NuBuffer_t              *pRSBuffer;           /* the pre-allocate buffer for RecordSet */
    NuTimerEventHdlr_t      *TimerEvent;
} NuDBM_t;

typedef struct _NuIndexTrash_t
{
    base_vector_t   *ItemVec[2];
    base_vector_t   *GroupVec[2];
    int             Using;
} NuIndexTrash_t;

/* Row                                        */
/* ------------------------------------------ */
typedef struct _NuColDef_t
{
    unsigned int  EffectIndex;  /* Binary representation of effected index of this column. */
    size_t        Length;       /* column data size */
    size_t        Start;        /* the start position of row data */
} NuColDef_t;

#define NuRowTranID_INS_FAIL    -2
#define NuRowTranID_DELETED     -1
#define NuRowTranID_NORMAL      0

typedef struct _NuRow_t
{
    int                 TranID;     /* del = -1 , none = 0 , lock = n */
    char                *Data;     /* every last byte is '\0' */
    char                *Update_Data;
    int                 Update_Action;
    unsigned int        Update_EffectIndex;
} NuRow_t;

/* ------------------------------------------ */

struct _NuTable_t
{
    base_vector_t       *Schema;            /* table schema, pointer to NuColDef_t */
    base_vector_t       *Index;
    NuIndex_t           *PKey;
    NuLock_t            Lock;               /* Table Lock */
    NuBlocking_t        *TableEvent;        /* Table condition variable */
    int                 DefaultRowNum;
    size_t              RowSize;
    NuBuffer_t          *pRowBuffer;
    NuBuffer_t          *pRowDataBuffer;    /* Row */
    base_vector_t       *pRSVec;
    NuCBMgr_t           *Event[NuDBMTableEventTypeNo];
    char                Name[1];
};

/* Index */
/* ------------------------------------------ */
#define MaxIndexNo  32

struct _NuIndex_t
{
    unsigned int    IndexFlag;      /* binary representaion of this index. */
    void            *pSearchKey;    /* Key for select only. */
    int             IsUnique;       /* 0 : multi key, 1 : unique key */
    NuHash_t        *IndexMap;      /* value = Row */
    NuTable_t       *pTable;
    NuBuffer_t      *KeyBuffer;     /* Buffer for key container. */
    NuLock_t        Lock;           /* Index lock. */
    NuIndexTrash_t  Trash;
    int             ColumnNo;       /* number of column included in this index. */
    size_t          Column[1];      /* Column involved of this index. */
};

struct _NuRecordSet_t
{
    base_vector_t           *UpdRowVec;
    NuTable_t               *pTable;
    int                     Status;
    NuHashItem_t            *HashIt;          /* Iterator for index Hash. */
    char                    *pSearchData;   /* For select-only result. */
    NuDBMConnection_t       TranID;
    base_vector_it          VecIt;
};

/* ------------------------------------------ */

#ifdef __cplusplus
}
#endif

#endif /* _NUDBMBASE_H */

