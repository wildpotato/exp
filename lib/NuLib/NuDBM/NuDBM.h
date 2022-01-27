
#include "NuCBMgr.h"
#include "NuCommon.h"

#ifndef _NUDBM_H
#define _NUDBM_H

#ifdef __cplusplus
extern "C" {
#endif

typedef int NuDBMConnection_t;
typedef struct _NuTable_t NuTable_t;
typedef struct _NuIndex_t NuIndex_t;
typedef struct _NuRecordSet_t NuRecordSet_t;

typedef enum _NuDBM_TableEventType
{
    NuDBMTableEventType_Insert = 0,
    NuDBMTableEventType_BeforeUpdate,
    NuDBMTableEventType_AfterUpdate,
    NuDBMTableEventType_Delete,
    NuDBMTableEventTypeNo,
    NuDBMTableEventType_Exception
} NuDBM_TableEventType;

typedef enum _NuIndexKind
{
    NuIndexKind_Index = 0,
    NuIndexKind_PKey,
    NuIndexKind_Unique,
    NuIndexKindNo
} NuIndexKind;

/* DBM Function                               */
/* ------------------------------------------ */
int NuDBMNew(void);
int NuDBMOpen(char *TranLogPath);
void NuDBMFree(void);
/* ColNum : column defain pair count, [Name, Length] */
NuTable_t *NuDBMAddTable(char *TableName, int DefaultRowNum, int ColNum, ...); 
NuTable_t *NuDBMGetTableByName(char *Name);
NuDBMConnection_t NuDBMConnect(void);

/* Table Function                             */
/* ------------------------------------------ */
NuIndex_t *NuTableAddIndex(NuTable_t *Table, NuIndexKind Kind, int ColumnNo, ...);
NuCBMgrHdlr_t *NuTableRegisterEvent(NuTable_t *Table, NuDBM_TableEventType Type, NuCBMgrFn Fn, void *Argu);

void NuTableUnRegisterEvent(NuCBMgrHdlr_t *EventHdlr);

/* RecordSet Function                         */
/* ------------------------------------------ */
int NuRSSetTran(NuRecordSet_t *pRec);

int NuRSDataSetByRow(NuRecordSet_t *pRec, ...);
int NuRSDataSet(NuRecordSet_t *pRec, int Pos, char *Value, size_t ValLen);
int NuRSDataSetStr(NuRecordSet_t *pRec, int Pos, char *StrValue);
int NuRSDataSetChar(NuRecordSet_t *pRec, int Pos, char *CharValue);
int NuRSDataVPrintf(NuRecordSet_t *pRec, int Pos, char *Fmt, va_list Argu);
int NuRSDataPrintf(NuRecordSet_t *pRec, int Pos, char *Fmt, ...);
int NuRSDataSetNULL(NuRecordSet_t *pRec, int Pos);

char *NuRSDataGet(NuRecordSet_t *pRec, int Pos);

int NuRSResultCount(NuRecordSet_t *pRec);
int NuRSSelectByIndex(NuRecordSet_t *pRec, NuIndex_t *Index, ...);
int NuRSSelectByKey(NuRecordSet_t *pRec, NuIndex_t *Index, char *Key, size_t Klen);

//int NuRSSelectAll(NuRecordSet_t *pRec);
int NuRSInsert(NuRecordSet_t *pRec);
int	NuRSDelete(NuRecordSet_t *pRec);
int NuRSGenerateKey(NuRecordSet_t *pRec, NuIndex_t *Index, char *Key);
NuRecordSet_t *NuRSGet(NuDBMConnection_t ConnID, NuTable_t *Table);

int NuRSNextData(NuRecordSet_t *pRec);

int NuRSCommit(NuRecordSet_t *pRec);
int NuRSRollback(NuRecordSet_t *pRec);

#ifdef __cplusplus
}
#endif

#endif /* _NUDBM_H */

