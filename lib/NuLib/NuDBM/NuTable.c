#include "NuDBMBase.h"
#include "NuRecovery.c"

#define	DEFAULT_TABLE_NUM		5
#define	DEFAULT_UPDATEROW_NUM	30
#define	SET_TRAN_MAX_RETRY		3
#define DEFAULT_CONN_NO         10

NuDBM_t *_DBM = NULL;

/* Internal Function                          */
/* ------------------------------------------ */
static void DBMTimerEvent(void *Argu)
{
    NuTable_t       *pTable = NULL;
    base_vector_t   *TableVec = _DBM->TableVec, *IndexVec = NULL;
    base_vector_it  TableVecIt, IndexVecIt;
    NuIndex_t       *pIndex = NULL;
    NuIndexTrash_t  *pIndexTrash = NULL;
    int             Tmp = 0;
    void            *ptr = NULL;

    base_vector_it_set(TableVecIt, TableVec);
    while(TableVecIt != base_vector_it_end(TableVec))
    {
        pTable = (NuTable_t *)(*TableVecIt);

        base_vector_it_set(IndexVecIt, IndexVec = pTable->Index);
        while(IndexVecIt != base_vector_it_end(IndexVec))
        {
            pIndex = (NuIndex_t *)(*IndexVecIt);
            pIndexTrash = &(pIndex->Trash);

            NuLockLock(&(pIndex->Lock));

            Tmp = (pIndexTrash->Using) ^ 1;

            while(base_vector_pop(pIndexTrash->ItemVec[Tmp], &ptr) >= 0)
            {
                NuHashItemRemoveComplete(pIndex->IndexMap, ptr);
            }

            pIndexTrash->Using = Tmp;

            NuLockUnLock(&(pIndex->Lock));

            ++ IndexVecIt;
        }

        ++ TableVecIt;
    }

    return;
}

static void NuTableFree(NuTable_t *pTable)
{
    int         Cnt = 0;
    void        *ptr = NULL;
    NuIndex_t   *pIndex = NULL;

    if(pTable != NULL)
    {
        if(pTable->Index != NULL)
        {
            while(base_vector_pop(pTable->Index, &ptr) >= 0)
            {
                pIndex = (NuIndex_t *)ptr;

				NuLockDestroy(&(pIndex->Lock));
				base_vector_free(pIndex->Trash.ItemVec[0]);
				base_vector_free(pIndex->Trash.ItemVec[1]);
				base_vector_free(pIndex->Trash.GroupVec[0]);
				base_vector_free(pIndex->Trash.GroupVec[1]);

                NuHashFree(pIndex->IndexMap);
                NuBufferFree(pIndex->KeyBuffer);

                free(pIndex);
            }
        
            base_vector_free(pTable->Index);
        }

        if(pTable->TableEvent != NULL)
        {
            NuBlockingFree(pTable->TableEvent, NULL);
        }

        if(pTable->pRowBuffer != NULL)
        {
            NuBufferFree(pTable->pRowBuffer);
        }

        if(pTable->pRowDataBuffer != NULL)
        {
            NuBufferFree(pTable->pRowDataBuffer);
        }

        while(base_vector_pop(pTable->pRSVec, &ptr) >= 0)
        {
			base_vector_free(((NuRecordSet_t *)ptr)->UpdRowVec);
            NuBufferPut(_DBM->pRSBuffer, ptr);
        }
		base_vector_free(pTable->pRSVec);

        if(pTable->Schema != NULL)
        {
            ptr = (void *)base_vector_get_by_index(pTable->Schema, 0);
            free(ptr);
            base_vector_free(pTable->Schema);
        }

        for(Cnt = 0; Cnt < NuDBMTableEventTypeNo; ++ Cnt)
        {
			NuCBMgrDel(pTable->Event[Cnt]);
        }

		NuLockDestroy(&(pTable->Lock));

        free(pTable);
    }

    return;
}

int NuDBMNew(void)
{
    int RC = NU_OK;

    _DBM = NULL;

    if(!(_DBM = (NuDBM_t *)malloc(sizeof(NuDBM_t))))
    {
        NUGOTO(RC, NU_MALLOC_FAIL, EXIT);
    }

    _DBM->TableVec = NULL;
    RC = base_vector_new(&(_DBM->TableVec), DEFAULT_TABLE_NUM);
    NUCHKRC(RC, EXIT);

    _DBM->CurrConnID = 0;

    _DBM->TimerEvent = NULL;

EXIT:
    if(RC < 0)
    {
        NuDBMFree();
    }

    return RC;
}

int NuDBMOpen(char *TranLogPath)
{
    int             RecordSetNo = 0;
    NuTable_t       *pTable = NULL;
    base_vector_t   *pTableVec = NULL;
    base_vector_it  VecIt;
    size_t          RowLen = 0;

    if(!_DBM)
    {
        NuDBMNew();
    }

    if(_DBM->TimerEvent != NULL)
    {
        return NU_OK;
    }

    pTableVec = _DBM->TableVec;

    base_vector_it_set(VecIt, pTableVec);
    while(VecIt != base_vector_it_end(pTableVec))
    {
        pTable = (NuTable_t *)(*VecIt);

        if(!(pTable->PKey))
        {
            return NuDBMErr_TableNoPKey;
        }

        if(pTable->RowSize > RowLen)
        {
            RowLen = pTable->RowSize;
        }

        ++ VecIt;
    }
/*    
    if(TranLogPath != NULL)
    {
        NuRecoveryStart(RowLen * 2, TranLogPath);
    }
*/
    RecordSetNo = base_vector_get_cnt(pTableVec) * DEFAULT_CONN_NO;

    NuBufferNew(&(_DBM->pRSBuffer), sizeof(NuRecordSet_t), RecordSetNo);
    _DBM->TimerEvent = NuTimerRegister(30, 0, &DBMTimerEvent, _DBM);

    NuDBMConnect();

    if(TranLogPath != NULL)
    {
        NuRecoveryStart(RowLen * 2, TranLogPath);
    }

    return NU_OK;
}

void NuDBMFree(void)
{
    void    *pTable = NULL;

    if(_DBM != NULL)
    {
        NuRecoveryStop();

        if(_DBM->TimerEvent != NULL)
        {
            NuTimerUnregister(_DBM->TimerEvent);
        }

        NuTimerFree();
        sleep(3);

        if(_DBM->TableVec != NULL)
        {
            while(base_vector_get_cnt(_DBM->TableVec) > 0)
            {
                base_vector_pop(_DBM->TableVec, &pTable);
                NuTableFree(pTable);
            }
            
            base_vector_free(_DBM->TableVec);
        }

		NuBufferFree(_DBM->pRSBuffer);

        free(_DBM);
    }

    _DBM = NULL;
    return;
}

NuTable_t *NuDBMGetTableByName(char *Name)
{
    base_vector_t   *pVec = NULL;
    base_vector_it  VecIt;
    NuTable_t       *pTable = NULL;

    if(!_DBM)
    {
        NuDBMNew();
    }

    base_vector_it_set(VecIt, pVec = _DBM->TableVec);

    while(VecIt != base_vector_it_end(pVec))
    {
        pTable = (NuTable_t *)(*VecIt);

        if(!strcmp(Name, pTable->Name))
        {
            return pTable;
        }

        ++ VecIt;
    }

    return NULL;
}

NuDBMConnection_t NuDBMConnect(void)
{
    NuRecordSet_t   *pRS = NULL;
    NuTable_t       *pTable = NULL;
    base_vector_t   *pTableVec = _DBM->TableVec;
    base_vector_it  VecIt;

    if(!_DBM->TimerEvent)
    {
        return NuDBMErr_DBMNotOpen;
    }

    base_vector_it_set(VecIt, pTableVec);

    while(VecIt != base_vector_it_end(pTableVec))
    {
        pTable = (NuTable_t *)(*VecIt);

        NuLockLock(&(pTable->Lock));
        pRS = (NuRecordSet_t *)NuBufferGet(_DBM->pRSBuffer);
        pRS->TranID = -1 * (_DBM->CurrConnID);
        pRS->Status = 0;

        pRS->HashIt = NULL;

        base_vector_new(&(pRS->UpdRowVec), 10);

        pRS->pSearchData = (char *)NuBufferGet(pTable->pRowDataBuffer);

        pRS->pTable = pTable;

        base_vector_push(pTable->pRSVec, pRS);
        NuLockUnLock(&(pTable->Lock));
    
        ++ VecIt;
    }

    ++ (_DBM->CurrConnID);

    return (_DBM->CurrConnID) - 1;
}

NuTable_t *NuDBMAddTable(char *TableName, int DefaultRowNum, int ColNum, ...)
{
    int         RC = NU_OK, ColLen = 0, RowLen = 0;
    char        *ColName = NULL;
    NuColDef_t  *pColDef = NULL;
    NuTable_t   *pTable = NULL;
    va_list     Column;

    if(_DBM->TimerEvent)
    {
        return NULL;
    }

    if(!(pTable = (NuTable_t *)malloc(sizeof(NuTable_t) + strlen(TableName) + 1)))
    {
        NUGOTO(RC, NU_MALLOC_FAIL, EXIT);
    }

    base_vector_new(&(pTable->Schema), 10);
    if(!(pTable->Schema))
    {
        NUGOTO(RC, NU_MALLOC_FAIL, EXIT);
    }

    RC = base_vector_new(&(pTable->Index), 10);
    NUCHKRC(RC, EXIT);

    pTable->PKey = NULL;

    strcpy(pTable->Name, TableName);

    RC = NuLockInit(&(pTable->Lock), &NuLockType_Mutex);
    NUCHKRC(RC, EXIT);

    pTable->TableEvent = NULL;
    RC = NuBlockingNew(&(pTable->TableEvent), &NuBlocking_CondVar, NULL);
    NUCHKRC(RC, EXIT);

    pTable->pRowBuffer = NULL;
    RC = NuBufferNew(&(pTable->pRowBuffer), sizeof(NuRow_t), DefaultRowNum);
    NUCHKRC(RC, EXIT);

    pColDef = (NuColDef_t *)malloc(sizeof(NuColDef_t) * ColNum);

    va_start(Column, ColNum);
    for(RC = 0; RC < ColNum; ++ RC)
    {
        ColName = va_arg(Column, char *);
        ColLen = va_arg(Column, size_t);

		if (ColName == NULL)
		{
			ColName = "";
		}

        pColDef->EffectIndex = 0;
        pColDef->Start = RowLen;
        pColDef->Length = ColLen;

        base_vector_push(pTable->Schema, pColDef);
        RowLen += (ColLen + 1);

        ++ pColDef;
    }
    va_end(Column);

    pTable->DefaultRowNum = DefaultRowNum;
    pTable->RowSize = RowLen;

    pTable->pRowDataBuffer = NULL;
    RC = NuBufferNew(&(pTable->pRowDataBuffer), RowLen, DefaultRowNum + 10);
    NUCHKRC(RC, EXIT);

    for(RC = 0; RC < NuDBMTableEventTypeNo; ++ RC)
    {
        pTable->Event[RC] = NuCBMgrAdd();
    }

    base_vector_new(&(pTable->pRSVec), DEFAULT_CONN_NO);
    
    base_vector_push(_DBM->TableVec, pTable);

EXIT:
    if(RC < 0)
    {
        NuTableFree(pTable);
    }

    return pTable;
}

/* Table Function                             */
/* ------------------------------------------ */

NuIndex_t *NuTableAddIndex(NuTable_t *Table, NuIndexKind Kind, int ColumnNo, ...)
{
    int         RC = NU_OK;
    va_list     Column;
    int         Cnt = 0;
    size_t      KeyLen = 1;
    int         ColumnIdx = -1;
    NuIndex_t   *pIndex = NULL;
    NuColDef_t  *pSchema = NULL;

    if(_DBM->TimerEvent)
    {
        return NULL;
    }

    if(base_vector_get_cnt(Table->Index) >= MaxIndexNo)
    {
        return NULL;
    }

    if(!(pIndex = (NuIndex_t *)calloc(1, sizeof(NuIndex_t) + sizeof(size_t) * ColumnNo)))
    {
        NUGOTO(RC, NU_MALLOC_FAIL, EXIT);
    }

    pIndex->IndexFlag = 1<<(base_vector_get_cnt(Table->Index));

    pIndex->IndexMap = NULL;
    RC = NuHashNew(&(pIndex->IndexMap), Table->DefaultRowNum);
    NUCHKRC(RC, EXIT);

    va_start(Column, ColumnNo);
    for(Cnt = 0; Cnt < ColumnNo; Cnt ++)
    {
        ColumnIdx = va_arg(Column, int);
        pSchema = (NuColDef_t *)(base_vector_get_by_index(Table->Schema, ColumnIdx));
        pIndex->Column[Cnt] = pSchema->Start;
        KeyLen += (pSchema->Length + 1);
        pSchema->EffectIndex |= pIndex->IndexFlag;
    }
    va_end(Column);

    pIndex->ColumnNo = ColumnNo;
    NuBufferNew(&(pIndex->KeyBuffer), KeyLen, Table->DefaultRowNum);
    pIndex->pSearchKey = NuBufferGet(pIndex->KeyBuffer);

    RC = NuLockInit(&(pIndex->Lock), &NuLockType_Mutex);
    NUCHKRC(RC, EXIT);

    base_vector_new(&(pIndex->Trash.ItemVec[0]), 10);
    base_vector_new(&(pIndex->Trash.ItemVec[1]), 10);
    base_vector_new(&(pIndex->Trash.GroupVec[0]), 10);
    base_vector_new(&(pIndex->Trash.GroupVec[1]), 10);

    pIndex->Trash.Using = 0;

    pIndex->pTable = Table;
    pIndex->IsUnique = 0;
    RC = base_vector_push(Table->Index, pIndex);

    switch(Kind)
    {
    case NuIndexKind_PKey:
        Table->PKey = pIndex;
    case NuIndexKind_Unique:
        pIndex->IsUnique = 1;
    case NuIndexKind_Index:
    default:
        break;
    }


EXIT:
    if(RC < 0)
    {
        if(pIndex != NULL)
        {
            if(pIndex->IndexMap != NULL)
            {
                NuHashFree(pIndex->IndexMap);
            }

            NuLockDestroy(&(pIndex->Lock));

            free(pIndex);
        }
    }

    return pIndex;
}

NuCBMgrHdlr_t *NuTableRegisterEvent(NuTable_t *Table, NuDBM_TableEventType Type, NuCBMgrFn Fn, void *Argu)
{
    if(Type >= NuDBMTableEventTypeNo)
    {
        return NULL;
    }

    return NuCBMgrRegisterEvent(Table->Event[Type], Fn, Argu);
}

void NuTableUnRegisterEvent(NuCBMgrHdlr_t *Hdlr)
{
    NuCBMgrUnRegisterEvent(Hdlr);
    return;
}

