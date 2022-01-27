#include "NuDBMBase.h"

#include "NuIndex.c"

extern NuDBM_t *_DBM;

/* Permission Control                         */
/* ------------------------------------------ */
enum _RSStatus
{
    _RSStatus_Initiate = 0,
    _RSStatus_Process = 1,
    _RSStatus_Final = 2
};

#define _CanInsert(pRS)             ((pRS)->Status < _RSStatus_Final && (pRS)->TranID > 0)
#define _CanSelect(pRS)             ((pRS)->Status < _RSStatus_Final)
#define _CanSetData(pRS)            ((pRS)->Status == _RSStatus_Process && (pRS)->TranID > 0)
#define _CanDelete(pRS)             ((pRS)->Status == _RSStatus_Process && (pRS)->TranID > 0)
#define _CanNext(pRS)               ((pRS)->Status == _RSStatus_Process)
#define _CanCommitOrRollBack(pRS)   ((pRS)->Status < _RSStatus_Final && (pRS)->TranID > 0)


/* Internal Function                          */
/* ------------------------------------------ */
#define GetRowDataFromBuffer(pTable)	(char *)NuBufferGet((pTable)->pRowDataBuffer)

static NuRow_t *GetRowFromBuffer(NuTable_t *pTable)
{
    NuRow_t *pRow = (NuRow_t *)NuBufferGet(pTable->pRowBuffer);

    pRow->TranID = NuRowTranID_NORMAL;
    pRow->Data = pRow->Update_Data = NULL;

    return pRow;
}

static int TrySetTran(NuRecordSet_t *pRec, NuIndex_t *pIndex)
{
    NuRow_t             *pRow = NULL;
    NuTable_t           *pTable = pRec->pTable;
    NuHashItem_t        *pIt = pRec->HashIt;
    base_vector_t       *pVec = pRec->UpdRowVec;
    void                *VecData = NULL;
    int                 TranID = pRec->TranID;

 //   NuHashItTheFirst(pIt);
 	pIt = NuHashLeftMost(pIt);

    NuLockLock(&(pTable->Lock));
    NuLockLock(&(pIndex->Lock));

    do
    {
        pRow = (NuRow_t *)NuHashItemGetValue(pIt);
        switch(pRow->TranID)
		{
			case NuRowTranID_DELETED:
				break;
			case NuRowTranID_NORMAL:
				pRow->Update_Action = NuDBMTableEventType_Exception;
				pRow->Update_EffectIndex = 0;
				pRow->TranID = TranID;
				base_vector_push(pVec, pRow);

				break;
			default:
				while(base_vector_pop(pVec, &VecData) >= 0)
				{
					pRow = (NuRow_t *)VecData;
					pRow->TranID = NuRowTranID_NORMAL;
				}

				NuLockUnLock(&(pIndex->Lock));
				NuLockUnLock(&(pTable->Lock));
				return NUFALSE;
		}
    }
    while((pIt = NuHashRight(pIt)) != NULL);

    NuLockUnLock(&(pIndex->Lock));
    NuLockUnLock(&(pTable->Lock));

    base_vector_it_set(pRec->VecIt, pRec->UpdRowVec);
//    NuHashItTheFirst(pIt);

    return NUTRUE;
}

static void NuRowDataChange(NuRow_t *pRow)
{
    char *p = pRow->Data;
    pRow->Data = pRow->Update_Data;
    pRow->Update_Data = p;

    return;
}

static void NuRSClean(NuRecordSet_t *pRec, int Success)
{
    base_vector_it      VecIt;
    NuRow_t             *pRow = NULL;
    NuTable_t           *pTable = pRec->pTable;
    NuIndex_t           *pIndex = NULL;

    pRec->Status = _RSStatus_Final;

    base_vector_it_set(pRec->VecIt, pRec->UpdRowVec);
    while(pRec->VecIt != base_vector_it_end(pRec->UpdRowVec))
    {
        pRow = (NuRow_t *)(*(pRec->VecIt));

        if(Success)
        {
			switch(pRow->Update_Action)
			{
				case NuDBMTableEventType_BeforeUpdate:
				case NuDBMTableEventType_AfterUpdate:
					NuCBMgrRaiseEvent(pTable->Event[NuDBMTableEventType_BeforeUpdate], pRec);
					NuRowDataChange(pRow);
					NuCBMgrRaiseEvent(pTable->Event[NuDBMTableEventType_AfterUpdate], pRec);
					break;
				default:
					NuCBMgrRaiseEvent(pTable->Event[pRow->Update_Action], pRec);
					break;
			}
        }

        if(pRow->Update_Data != NULL)
        {
            NuBufferPut(pTable->pRowDataBuffer, pRow->Update_Data);
            pRow->Update_Data = NULL;
        }

		if (pRow->TranID == NuRowTranID_DELETED)
		{
            base_vector_it_set(VecIt, pTable->Index);
            while(VecIt != base_vector_it_end(pTable->Index))
            {
                pIndex = (NuIndex_t *)(*VecIt);

                NuLockLock(&(pIndex->Lock));
                NuIndexRmv(pIndex, pRow->Data);
                NuLockUnLock(&(pIndex->Lock));
				++VecIt;
            }

            NuBufferPut(pTable->pRowDataBuffer, pRow->Data);
            NuBufferPut(pTable->pRowBuffer, pRow);
		}
        if(pRow->TranID == NuRowTranID_INS_FAIL)
        { 
            NuBufferPut(pTable->pRowDataBuffer, pRow->Data);
            NuBufferPut(pTable->pRowBuffer, pRow);
            pRow->TranID = NuRowTranID_DELETED;
        }
        else
        { 
            pRow->TranID = NuRowTranID_NORMAL;
        }

        ++ (pRec->VecIt);
    }

    /* Reset */
    pRec->TranID *= -1;
    pRec->Status = _RSStatus_Initiate;

    pRec->HashIt = NULL;
    base_vector_clear(pRec->UpdRowVec);

    return;
}

/* RecordSet Function                         */
/* ------------------------------------------ */

int NuRSSetTran(NuRecordSet_t *pRec)
{
    if(pRec->TranID < 0)
    {
        pRec->TranID *= -1;
    }
    pRec->Status = _RSStatus_Process;    

    return NU_OK;
}

int NuRSDataSetByRow(NuRecordSet_t *pRec, ...)
{
    int             Cnt = 0;
    char            *pCol = NULL;
    NuRow_t         *pRow = NULL;
    NuTable_t       *pTable = pRec->pTable;
    va_list         Column;
    base_vector_t   *pSchema = pTable->Schema;
    base_vector_it  VecIt;
    NuColDef_t      *pColDef = NULL;
    char            *pData = NULL;

    if(!_CanSetData(pRec))
    {
        return NuDBMErr_InvalidRecordSet;
    }

    pRow = (NuRow_t *)(*(pRec->VecIt));

    if(pRow->Update_Action == NuDBMTableEventType_Exception)
    {
        if(!(pRow->Update_Data))
        {
            NuLockLock(&(pTable->Lock));
            pRow->Update_Data = GetRowDataFromBuffer(pTable);
            NuLockUnLock(&(pTable->Lock));
            memcpy(pRow->Update_Data, pRow->Data, pTable->RowSize);
        }

        pRow->Update_Action = NuDBMTableEventType_BeforeUpdate;
    }

    pData = (pRow->TranID > 0) ? pRow->Update_Data : pRow->Data;

    va_start(Column, pRec);

    base_vector_it_set(VecIt, pSchema);
    while(VecIt != base_vector_it_end(pSchema))
    {
        if((pCol = va_arg(Column, char *)) != NULL)
        {
            pColDef = (NuColDef_t *)base_vector_get_by_index(pSchema, Cnt);
            strncpy(pData + pColDef->Start, pCol, pColDef->Length);
            pRow->Update_EffectIndex |= pColDef->EffectIndex;
        }

        ++ Cnt;
        ++ VecIt;
    }

    va_end(Column);

    return NU_OK;
}

typedef void (*NuRSDataSetFn)(char *Data, size_t DataLen, char *Value, size_t Len, va_list Argu);

static int _NuRSDataSet_Template(NuRecordSet_t *pRec, int Pos, char *Value, size_t ValLen, NuRSDataSetFn Fn, va_list Argu)
{
    NuRow_t         *pRow = NULL;
    NuTable_t       *pTable = pRec->pTable;
    NuColDef_t      *pColDef = NULL;
    base_vector_t   *pSchema = pTable->Schema;
    char            *pData = NULL;

    if(!_CanSetData(pRec))
    {
        return NuDBMErr_InvalidRecordSet;
    }

    if(base_vector_get_cnt(pSchema) < Pos)
    {
        return NuDBMErr_InvalidRowNum;
    }

    pRow = (NuRow_t *)(*(pRec->VecIt));

    if(pRow->Update_Action == NuDBMTableEventType_Exception)
    {
        if(!(pRow->Update_Data))
        {
            NuLockLock(&(pTable->Lock));
            pRow->Update_Data = GetRowDataFromBuffer(pTable);
            NuLockUnLock(&(pTable->Lock));
            memcpy(pRow->Update_Data, pRow->Data, pTable->RowSize);
        }

        pRow->Update_Action = NuDBMTableEventType_BeforeUpdate;
    }

    pData = ((pRow->TranID > 0) ? pRow->Update_Data : pRow->Data) + (pColDef = (NuColDef_t *)base_vector_get_by_index(pSchema, Pos))->Start;

    Fn(pData, pColDef->Length, Value, ValLen, Argu);

    pRow->Update_EffectIndex |= pColDef->EffectIndex;

    return NU_OK;
}

static void _NuRSDataSet_helper(char *Data, size_t DataLen, char *Value, size_t ValLen, va_list Argu)
{
    size_t  Len = NuMin(ValLen, DataLen);

    memcpy(Data, Value, Len);
    *(Data + Len) = '\0';

    return;
}

int NuRSDataSet(NuRecordSet_t *pRec, int Pos, char *Value, size_t ValLen)
{
    return _NuRSDataSet_Template(pRec, Pos, Value, ValLen, &_NuRSDataSet_helper, NULL);
}

static void _NuRSDataSetStr_helper(char *Data, size_t DataLen, char *StrValue, size_t ValLen, va_list Argu)
{
    strncpy(Data, StrValue, DataLen);
    return;
}

int NuRSDataSetStr(NuRecordSet_t *pRec, int Pos, char *StrValue)
{
    return _NuRSDataSet_Template(pRec, Pos, StrValue, 0, &_NuRSDataSetStr_helper, NULL);
}

static void _NuRSDataSetChar_helper(char *Data, size_t DataLen, char *CharValue, size_t ValLen, va_list Argu)
{
    *Data = *CharValue;
    *(Data + 1) = '\0';

    return;
}

int NuRSDataSetChar(NuRecordSet_t *pRec, int Pos, char *CharValue)
{
    return _NuRSDataSet_Template(pRec, Pos, CharValue, 0, &_NuRSDataSetChar_helper, NULL);
}

static void _NuRSDataPrintf_helper(char *Data, size_t DataLen, char *Fmt, size_t ValLen, va_list Argu)
{
    vsnprintf(Data, DataLen, Fmt, Argu);

    return;
}

int NuRSDataVPrintf(NuRecordSet_t *pRec, int Pos, char *Fmt, va_list Argu)
{
    return _NuRSDataSet_Template(pRec, Pos, Fmt, 0, &_NuRSDataPrintf_helper, Argu);
}

int NuRSDataPrintf(NuRecordSet_t *pRec, int Pos, char *Fmt, ...)
{
    va_list Argu;
    int     RC = NU_OK;

    va_start(Argu, Fmt);
    RC = _NuRSDataSet_Template(pRec, Pos, Fmt, 0, &_NuRSDataPrintf_helper, Argu);
    va_end(Argu);

    return RC;
}

int NuRSDataSetNULL(NuRecordSet_t *pRec, int Pos)
{
    return NuRSDataSetChar(pRec, Pos, "\0");
}

char *NuRSDataGet(NuRecordSet_t *pRec, int Pos)
{
    base_vector_t   *pSchema = pRec->pTable->Schema;
    char            *pData = (pRec->TranID < 0) ? (pRec->pSearchData) : (((NuRow_t *)(*(pRec->VecIt)))->Data);

    if(base_vector_get_cnt(pSchema) < Pos)
    {
        return NULL;
    }

    return pData + ((NuColDef_t *)base_vector_get_by_index(pSchema, Pos))->Start;
}

int NuRSResultCount(NuRecordSet_t *pRec)
{
	return (pRec->HashIt == NULL) ? 0 : NuHashItemGetKeyCollision(pRec->HashIt);
}

int NuRSSelectByIndex(NuRecordSet_t *pRec, NuIndex_t *Index, ...)
{
    int         Klen = 0;
    va_list     Column;

    va_start(Column, Index);

    Klen = NuIndexGenerateKeyV(pRec->pSearchData, Index, Column);
    va_end(Column);

    return NuRSSelectByKey(pRec, Index, pRec->pSearchData, Klen);
}

int NuRSSelectByKey(NuRecordSet_t *pRec, NuIndex_t *Index, char *Key, size_t Klen)
{
    NuTable_t       *pTable = pRec->pTable;
    NuRow_t         *pRow = NULL;

    if(!_CanSelect(pRec))
    {
        return NuDBMErr_InvalidRecordSet;
    }
    if(!(pRow = NuIndexSearch(Index, Key, Klen, &(pRec->HashIt))))
    {
        return NU_FAIL;
    }

    pRec->Status = _RSStatus_Process;

    if(pRec->TranID < 0)
    {	/* for select */
        memcpy(pRec->pSearchData, pRow->Data, pTable->RowSize);
    }
    else
    {
        while(!TrySetTran(pRec, Index))
        {
            NuBlockingBlock(pTable->TableEvent, NULL);
        }
    }

    return NU_OK;
}
/*
int NuRSSelectAll(NuRecordSet_t *pRec)
{
    NuTable_t       *pTable = pRec->pTable;
    NuRow_t         *pRow = NULL;
    NuIndex_t       *pPKey = pTable->PKey;

    if(!_CanSelect(pRec))
    {
        return NuDBMErr_InvalidRecordSet;
    }

    if(!(pRow = NuIndexGetAll(pPKey, &(pRec->HashIt))))
    {
        return NU_FAIL;
    }

    if(pRec->TranID > 0)
    {
        return NU_FAIL;
    }

    pRec->Status = _RSStatus_Process;
    memcpy(pRec->pSearchData, pRow->Data, pTable->RowSize);

    return NU_OK;
}
*/
int NuRSInsert(NuRecordSet_t *pRec)
{
    NuRow_t         *pRow = NULL;
    NuTable_t       *pTable = pRec->pTable;

    if(!_CanInsert(pRec))
    {
        return NuDBMErr_InvalidRecordSet;
    }

    NuLockLock(&(pTable->Lock));
    pRow = GetRowFromBuffer(pTable);
    pRow->Data = GetRowDataFromBuffer(pTable);
    NuLockUnLock(&(pTable->Lock));

    pRow->TranID = NuRowTranID_DELETED;
    pRow->Update_Action = NuDBMTableEventType_Insert;

    base_vector_push(pRec->UpdRowVec, pRow);
    base_vector_it_setend(pRec->VecIt, pRec->UpdRowVec);

    return NU_OK;
}

int NuRSDelete(NuRecordSet_t *pRec)
{
    if(!_CanDelete(pRec))
    {
        return NuDBMErr_InvalidRecordSet;
    }

    ((NuRow_t *)(*(pRec->VecIt)))->Update_Action = NuDBMTableEventType_Delete;

    return NU_OK;
}

int NuRSGenerateKey(NuRecordSet_t *pRec, NuIndex_t *Index, char *Key)
{
    NuRow_t *pRow = (NuRow_t *)((pRec->TranID < 0) ? NuHashItemGetValue(pRec->HashIt) : (*(pRec->VecIt)));
 
    return NuIndexGenerateKeyByRowData(Key, Index, pRow->Data);
}

NuRecordSet_t *NuRSGet(NuDBMConnection_t ConnID, NuTable_t *Table)
{
    return (NuRecordSet_t *)base_vector_get_by_index(Table->pRSVec, (int)ConnID);
}

int NuRSNextData(NuRecordSet_t *pRec)
{
    NuRow_t             *pRow = NULL;
    NuHashItem_t    *pIt = pRec->HashIt;

    if(!_CanNext(pRec))
    {
        return NuDBMErr_InvalidRecordSet;
    }

    if(pRec->TranID < 0)
    {
        while( (pIt = NuHashRight(pIt)) != NULL)
        {
            pRow = (NuRow_t *)(NuHashItemGetValue(pIt));
            if(pRow->TranID == NuRowTranID_DELETED)
            {
                continue;
            }

            memcpy(pRec->pSearchData, pRow->Data, pRec->pTable->RowSize);
			pRec->HashIt = pIt;
            return NU_OK;
        }
    }
    else
    {
        if((++ (pRec->VecIt)) != base_vector_it_end(pRec->UpdRowVec))
        {
            return NU_OK;
        }
        else
        {
            -- (pRec->VecIt);
        }
    }

    return NuDBMErr_AlreadyLastData;
}

int NuRSCommit(NuRecordSet_t *pRec)
{
    int                 RC = NU_OK;
    base_vector_t       *pVec = pRec->UpdRowVec;
    base_vector_it      VecIt;
    NuRow_t             *pRow = NULL;
    NuTable_t           *pTable = pRec->pTable;

    if(!_CanCommitOrRollBack(pRec))
    {
        return NuDBMErr_InvalidRecordSet;
    }

    base_vector_it_set(VecIt, pVec);

    NuLockLock(&(pTable->Lock));

    while(VecIt != base_vector_it_end(pVec) && RC == NU_OK)
    {
        pRow = (NuRow_t *)(*VecIt);

        switch(pRow->Update_Action)
		{
			case NuDBMTableEventType_Delete:
				pRow->TranID = NuRowTranID_DELETED;
				break;
			case NuDBMTableEventType_BeforeUpdate:
				if((RC = NuIndexUpdate(pTable, pRow, pRow->Update_EffectIndex)) != NU_OK)
				{
					do
					{
						pRow = (NuRow_t *)(*VecIt);

						switch(pRow->Update_Action)
						{
							case NuDBMTableEventType_Delete:
								pRow->TranID = pRec->TranID;
								break;
							case NuDBMTableEventType_BeforeUpdate:
								NuIndexUpdate(pTable, pRow, pRow->Update_EffectIndex);
								break;
							case NuDBMTableEventType_Insert:
								pRow->TranID = NuRowTranID_DELETED;
								break;
						}
						-- VecIt;
					}
					while(VecIt != base_vector_it_begin(pVec));
				}

				break;
			case NuDBMTableEventType_Insert:
				if((RC = NuIndexInsert(pTable, pRow)) >= 0)
				{
					pRow->TranID = NuRowTranID_NORMAL;
				}
				else
				{
					pRow->TranID = NuRowTranID_INS_FAIL;
				}

				break;
		}
		++ VecIt;
	}

	/* Clean up. */
    NuRSClean(pRec, (RC == NU_OK) ? 1 : 0);
    NuLockUnLock(&(pTable->Lock));
    NuBlockingWakeAll(pTable->TableEvent, NULL);

    return RC;
}

int NuRSRollback(NuRecordSet_t *pRec)
{
    NuTable_t   *pTable = pRec->pTable;

    if(!_CanCommitOrRollBack(pRec))
    {
        return NuDBMErr_InvalidRecordSet;
    }

    NuLockLock(&(pTable->Lock));
    NuRSClean(pRec, 0);
    NuLockUnLock(&(pTable->Lock));

    NuBlockingWakeAll(pTable->TableEvent, NULL);

    return NU_OK;
}

