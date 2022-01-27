
extern NuDBM_t *_DBM;

#define NuRecovery_InsertSign           'I'
#define NuRecovery_DeleteSign           'D'
#define NuRecovery_TerminateSign        '\001'

#define NuRecovery_Delimiter            '|'
#define NuRecovery_Peroid               '$'
#define NuRecovery_TailStr              "$\n"

#define NuRecovery_LogInsertHdrStr      "I|"
#define NuRecovery_LogUpdateDelHdrStr   "d|"
#define NuRecovery_LogUpdateInsHdrStr   "i|"
#define NuRecovery_LogDeleteHdrStr      "D|"
#define NuRecovery_LogTerminateStr      "\001"

#define NuRecovery_ReplaceDelimiterTo   '\002'

#define NuRecoverySign(Sign)            toupper(Sign)


typedef struct _NuRecoveryEvSet_t
{
    NuCBMgrHdlr_t           *Ev[NuDBMTableEventTypeNo];
    NuFileStream_t          *Log;
    NuTable_t               *Table;
} NuRecoveryEvSet_t;

typedef struct _NuTranLog_t
{
    NuRecoveryEvSet_t   *EvSet;
    char                *TypeHdr;
    char                Data[1];
} NuTranLog_t;

typedef struct _NuRecovery_t
{
    NuThread_t              ThreadID;
    int                     ThreadAlive;
    NuLock_t                Lock;
    NuBuffer_t              *LogBuffer;
    NuHQ_t                  *Q;
    int                     EvSetSize;
    NuRecoveryEvSet_t       EvSet[1];
} NuRecovery_t;

static NuRecovery_t *pRecovery = NULL;

static NuTranLog_t *GetTranLog(NuRecoveryEvSet_t *pEvSet, char *TypeHdr)
{
    NuTranLog_t *pTranLog = NULL;

    NuLockLock(&(pRecovery->Lock));
    pTranLog = (NuTranLog_t *)NuBufferGet(pRecovery->LogBuffer);
    pTranLog->EvSet = pEvSet;
    pTranLog->TypeHdr = TypeHdr;
    NuLockUnLock(&(pRecovery->Lock));

    return pTranLog;
}

static void NuRecoveryInsert(const void *Rec, void *Argu)
{
    NuRecordSet_t   *pRS = (NuRecordSet_t *)Rec;
    NuTable_t       *pTable = pRS->pTable;
    NuTranLog_t     *pTranLog = GetTranLog(Argu, NuRecovery_LogInsertHdrStr);

    memcpy(pTranLog->Data, ((NuRow_t *)(*(pRS->VecIt)))->Data, pTable->RowSize);

    NuHQEnqueue(pRecovery->Q, pTranLog, pTable->RowSize);

    return;
}

static void NuRecoveryUpdateInsert(const void *Rec, void *Argu)
{
    NuRecordSet_t   *pRS = (NuRecordSet_t *)Rec;
    NuTable_t       *pTable = pRS->pTable;
    NuTranLog_t     *pTranLog = GetTranLog(Argu, NuRecovery_LogUpdateInsHdrStr);

    memcpy(pTranLog->Data, ((NuRow_t *)(*(pRS->VecIt)))->Data, pTable->RowSize);

    NuHQEnqueue(pRecovery->Q, pTranLog, pTable->RowSize);

    return;
}

static void NuRecoveryUpdateDelete(const void *Rec, void *Argu)
{
    NuRecordSet_t   *pRS = (NuRecordSet_t *)Rec;
    NuTable_t       *pTable = pRS->pTable;
    NuTranLog_t     *pTranLog = GetTranLog(Argu, NuRecovery_LogUpdateDelHdrStr);
    int             Len = NuRSGenerateKey(pRS, pTable->PKey, pTranLog->Data);

    NuHQEnqueue(pRecovery->Q, pTranLog, Len);
    return;
}

static void NuRecoveryDelete(const void *Rec, void *Argu)
{
    NuRecordSet_t   *pRS = (NuRecordSet_t *)Rec;
    NuTable_t       *pTable = pRS->pTable;
    NuTranLog_t     *pTranLog = GetTranLog(Argu, NuRecovery_LogDeleteHdrStr);
    int             Len = NuRSGenerateKey(pRS, pTable->PKey, pTranLog->Data);

    NuHQEnqueue(pRecovery->Q, pTranLog, Len);
    return;
}

void NuDBMRecover(char *RecoverLogPath, char *TableName)
{
	int iRC = 0;
    char                *ptr = NULL, *pPipe = NULL, *pPipeEnd = NULL;
    int                 Cnt = 0;
    NuTable_t           *Table = NULL;
    NuFileStream_t      *pFS = NULL;
    int                 LogLen = 0;
    NuStr_t             *pStr = NULL;
    NuDBMConnection_t   ConnID = NuDBMConnect();
    NuRecordSet_t       *pRS = NULL;

    if((Table = NuDBMGetTableByName(TableName)) < 0)
    {
        return;
    }

    NuStrNew(&pStr, "\0");

    if(NuFStreamOpen(&pFS, RecoverLogPath, "r") == NU_OK)
    {
        while((LogLen = NuFStreamReadLineByStr(pFS, pStr)) > 0)
        {
            ptr = (char *)NuStrGet(pStr);
            if(*(ptr + LogLen - 2) != NuRecovery_Peroid)
            {
                continue;
            }

            *(ptr + LogLen - 2) = '\0';

            if(!(pPipe = strchr(ptr, '|')))
            {
                continue;
            }
            *pPipe = '\0';
            ++ pPipe;

            pRS = NuRSGet(ConnID, Table);
        
			switch(NuRecoverySign(*ptr))
			{
				case NuRecovery_InsertSign:
					Cnt = 0;
					NuRSSetTran(pRS);
					NuRSInsert(pRS);

					while((pPipeEnd = strchr(pPipe, '|')) != NULL)
					{
						*pPipeEnd = '\0';
						NuRSDataSetStr(pRS, Cnt, pPipe);

						++ Cnt;
						pPipe = pPipeEnd + 1;
					}

					NuRSCommit(pRS);

					break;
				case NuRecovery_DeleteSign:
					Cnt = strlen(pPipe);
					NuCStrReplaceChr(pPipe, '|', '\0');

					NuRSSetTran(pRS);
					iRC = NuRSSelectByKey(pRS, Table->PKey, pPipe, Cnt);
					if (iRC < 0)
					{
						NuRSRollback(pRS);
					}
					else
					{
						NuRSDelete(pRS);
						NuRSCommit(pRS);
					}

					break;
				default:
					break;
			}
        }
    }

    NuStrFree(pStr);
    NuFStreamClose(pFS);

    return;
}

static void _NuRecoveryFormatInsert(NuTranLog_t *pTranLog, NuStr_t *pStr)
{
    char            *pData = pTranLog->Data;
    base_vector_t   *pSchema = pTranLog->EvSet->Table->Schema;
    base_vector_it  VecIt;

    base_vector_it_set(VecIt, pSchema);
	/* ivan add table name to tlog */
//    NuStrCat(pStr, pTranLog->EvSet->Table->Name);
//    NuStrCat(pStr, "|");

    while(VecIt != base_vector_it_end(pSchema))
    {
        NuStrCat(pStr, pData + ((NuColDef_t *)(*VecIt))->Start);
        NuStrCat(pStr, "|");
        ++ VecIt;
    }

    return;
}

static void _NuRecoveryFormatDelete(NuTranLog_t *pTranLog, NuStr_t *pStr, size_t Len)
{
    char            *pData = pTranLog->Data;

    NuCStrReplaceRangeChr(pData, '\0', '|', Len);

//    NuStrCat(pStr, pTranLog->EvSet->Table->Name);
//    NuStrCat(pStr, "|");

    NuStrNCat(pStr, pData, Len);

    return;
}

static NUTHD_FUNC RecoveryLogThread(void *Argu)
{
    NuTranLog_t         *pTranLog = NULL;
    void                *ptr = NULL;
    NuRecovery_t        *pRecovery = (NuRecovery_t *)Argu;
    size_t              LogLen = 0;
    NuSubBuffer_t       *pSubBuf = NULL;
    NuStr_t             *pStr = NULL;
    char                Type = '\0';

    NuSubBufferNew(&pSubBuf, pRecovery->LogBuffer);
    NuStrNew(&pStr, NULL);

    while(pRecovery->ThreadAlive || !NuHQIsEmpty(pRecovery->Q))
    {
        if(NuHQDequeue(pRecovery->Q, &ptr, &LogLen) == NU_OK)
        {
            pTranLog = (NuTranLog_t *)ptr;

			switch(Type = NuRecoverySign(*(pTranLog->TypeHdr)))
			{
				case NuRecovery_TerminateSign:
					break;
				default:
					NuStrCpy(pStr, pTranLog->TypeHdr);
					NuCStrReplaceRangeChr(pTranLog->Data, NuRecovery_Delimiter, NuRecovery_ReplaceDelimiterTo, LogLen);

					if(Type == NuRecovery_InsertSign)
					{
						_NuRecoveryFormatInsert(pTranLog, pStr);
					}
					else
					{
						_NuRecoveryFormatDelete(pTranLog, pStr, LogLen);
					}

					NuStrCat(pStr, NuRecovery_TailStr);
					NuFStreamWriteN(pTranLog->EvSet->Log, NuStrGet(pStr), NuStrSize(pStr));

					break;
			}

            NuSubBufferPut(pSubBuf, ptr);
        }

        if(NuSubBufferGetCnt(pSubBuf) > 100)
        {
            NuLockLock(&(pRecovery->Lock));
            NuSubBufferTribute(pSubBuf);
            NuLockUnLock(&(pRecovery->Lock));
        }
    }

    NuSubBufferFree(pSubBuf);
    NuStrFree(pStr);

    NuThdReturn();
    return NULL;
}

static void NuRecoveryStop(void)
{
    int         Cnt = 0;
    NuTranLog_t TranLog = {NULL, NuRecovery_LogTerminateStr, "\0"};

    if(pRecovery != NULL)
    {
        if(pRecovery->ThreadAlive == 1)
        {
            pRecovery->ThreadAlive = 0;

            NuHQEnqueue(pRecovery->Q, &TranLog, 1);
            NuThdJoin(pRecovery->ThreadID);
        }

        while(pRecovery->EvSetSize --)
        {
            for(Cnt = 0; Cnt < NuDBMTableEventTypeNo; ++ Cnt)
            {
                NuTableUnRegisterEvent((pRecovery->EvSet[pRecovery->EvSetSize]).Ev[Cnt]);
            }

            NuFStreamFlush(pRecovery->EvSet[pRecovery->EvSetSize].Log);
            NuFStreamClose(pRecovery->EvSet[pRecovery->EvSetSize].Log);
        }

        if(pRecovery->Q != NULL)
        {
            NuHQFree(pRecovery->Q);
        }

        NuBufferFree(pRecovery->LogBuffer);
        NuLockDestroy(&(pRecovery->Lock));

        free(pRecovery);
    }

    return;
}

static int NuRecoveryStart(size_t LogSize, char *Path)
{
    int                 RC = NU_OK;
    base_vector_t       *pVec = _DBM->TableVec;
    base_vector_it      VecIt;
    NuStr_t             *pStr = NULL;
    int                 TableNo = base_vector_get_cnt(pVec);
    NuTable_t           *pTable = NULL;
    NuRecoveryEvSet_t   *pEvSet = NULL;
    
    if(!(pRecovery = (NuRecovery_t *)malloc(sizeof(NuRecovery_t) + sizeof(NuRecoveryEvSet_t) * TableNo)))
    {
        return NU_MALLOC_FAIL;
    }

    RC = NuHQNew(&(pRecovery->Q));
    NUCHKRC(RC, EXIT);

    RC = NuBufferNew(&(pRecovery->LogBuffer), sizeof(NuTranLog_t) + LogSize, TableNo * 100);
    NUCHKRC(RC, EXIT);

    RC = NuLockInit(&(pRecovery->Lock), &NuLockType_Mutex);
    NUCHKRC(RC, EXIT);

    pRecovery->EvSetSize = TableNo;

    pEvSet = pRecovery->EvSet;
    base_vector_it_set(VecIt, pVec);

    NuStrNew(&pStr, NULL);

	/* TODO : foreach all Table */
    while(VecIt != base_vector_it_end(pVec))
    {
        pTable = (NuTable_t *)(*VecIt);
/*        
        pEvSet->Ev[NuDBMTableEventType_Insert] = NuTableRegisterEvent(pTable, NuDBMTableEventType_Insert, &NuRecoveryInsert, pEvSet);
        pEvSet->Ev[NuDBMTableEventType_BeforeUpdate] = NuTableRegisterEvent(pTable, NuDBMTableEventType_BeforeUpdate, &NuRecoveryUpdateDelete, pEvSet);
        pEvSet->Ev[NuDBMTableEventType_AfterUpdate] = NuTableRegisterEvent(pTable, NuDBMTableEventType_AfterUpdate, &NuRecoveryUpdateInsert, pEvSet);
        pEvSet->Ev[NuDBMTableEventType_Delete] = NuTableRegisterEvent(pTable, NuDBMTableEventType_Delete, &NuRecoveryDelete, pEvSet);
*/

        pEvSet->Table = pTable;

        NuStrPrintf(pStr, 0, "%s%c%s.tlog", Path, NUFILE_SEPARATOR, pTable->Name);

        if(NuIsFile(NuStrGet(pStr)))
        {
            NuFStreamOpen(&(pEvSet->Log), NuStrGet(pStr), "a+");

			/* TODO Recover Table */
			NuDBMRecover((char *)NuStrGet(pStr), pTable->Name);

        }
        else
        {
            NuCreateRecursiveDir(Path);

            NuFStreamOpen(&(pEvSet->Log), NuStrGet(pStr), "w+");
        }

        pEvSet->Ev[NuDBMTableEventType_Insert] = NuTableRegisterEvent(pTable, NuDBMTableEventType_Insert, &NuRecoveryInsert, pEvSet);
        pEvSet->Ev[NuDBMTableEventType_BeforeUpdate] = NuTableRegisterEvent(pTable, NuDBMTableEventType_BeforeUpdate, &NuRecoveryUpdateDelete, pEvSet);
        pEvSet->Ev[NuDBMTableEventType_AfterUpdate] = NuTableRegisterEvent(pTable, NuDBMTableEventType_AfterUpdate, &NuRecoveryUpdateInsert, pEvSet);
        pEvSet->Ev[NuDBMTableEventType_Delete] = NuTableRegisterEvent(pTable, NuDBMTableEventType_Delete, &NuRecoveryDelete, pEvSet);

        ++ pEvSet;
        ++ VecIt;
    }

    NuStrFree(pStr);

    pRecovery->ThreadAlive = 1;
    RC = NuThdCreate(RecoveryLogThread, pRecovery, &(pRecovery->ThreadID));
    NUCHKRC(RC, EXIT);

EXIT:
    if(RC < 0)
    {
        NuRecoveryStop();
    }

    return RC;
}

