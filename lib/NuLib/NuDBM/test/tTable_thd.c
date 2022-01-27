
#include "NuDBM.h"
#include "NuThread.h"
#include <time.h>

#define TOTAL   100
#define ROW_NUMBER_DEFAULT 100


typedef struct _Argu_t
{
    NuIndex_t *PIndex;
	NuTable_t *Table;
	int       *Quit;
} Argu_t;

enum _TableSchema
{
    enID = 0,
	enName,
	enStock,
	enSide,
	enOrderQty,
	enOrderPrice,
	enCumQty,
	enAvgPx, 
	enNo
};
char *ColName[enNo] = { "ID", 
	                    "Name",
	                    "Stock", 
	                    "Side", 
	                    "OrderQty",
	                    "OrderPrice",
	                    "CumQty", 
	                    "AvgPx"
                      };


static void *_Thd_Work1(void *argu)
{
	int                 iRC = 0;
	Argu_t              *pArgu = (Argu_t *)argu;
    NuRecordSet_t       *pRS = NULL;
    NuDBMConnection_t   ConnId;
	char                szID[10+1] = {0};

	ConnId = NuDBMConnect();
    pRS = NuRSGet(ConnId, pArgu->Table);

	while(!(*(pArgu->Quit)))
	{
		/* Update */
		sprintf(szID, "%010d", 10);
		NuRSSetTran(pRS);
		iRC = NuRSSelectByIndex(pRS, pArgu->PIndex, szID);
		if (iRC == NU_OK)
		{
			NuRSDataSet(pRS, enOrderPrice, "999.0", 5);
			NuRSCommit(pRS);
//			printf("%ld update \n", NuThdSelf());
		}
		else
		{
			NuRSRollback(pRS);
//			printf("%ld update fail\n", NuThdSelf());
		}

		usleep(50);
	}
	return NULL;
}

static void *_Thd_Work2(void *argu)
{
	int                 iRC = 0;
	Argu_t              *pArgu = (Argu_t *)argu;
    NuRecordSet_t       *pRS = NULL;
    NuDBMConnection_t   ConnId;
	char                szID[10+1] = {0};

	ConnId = NuDBMConnect();
    pRS = NuRSGet(ConnId, pArgu->Table);

	while(!(*(pArgu->Quit)))
	{
		/* Update */
		sprintf(szID, "%010d", 10);
		NuRSSetTran(pRS);
		iRC = NuRSSelectByIndex(pRS, pArgu->PIndex, szID);
		if (iRC == NU_OK)
		{
			NuRSDataSet(pRS, enOrderPrice, "124.0", 5);
			NuRSCommit(pRS);
//			printf("%ld update \n", NuThdSelf());
		}
		else
		{
			NuRSRollback(pRS);
//			printf("%ld update fail\n", NuThdSelf());
		}
		usleep(100);
	}
	return NULL;
}

int main(int argc, char **argv)
{
    int                 iCnt = 0, Quit = 0;
    int                 iRC = 0;
	char                szID[10+1] = {0}, szName[20+1] = {0};
    NuTable_t           *pTable = NULL;
    NuIndex_t           *pPKey = NULL;
    NuRecordSet_t       *pRS = NULL;
    NuDBMConnection_t   ConnId;
	NuThread_t          Thd[2];

	Argu_t              Arg;

    clock_t             start = 0, end = 0;
    float               total = 0;

	/* initial DBM */
    iRC = NuDBMNew();
    if(iRC < 0)
    {
        printf("NuDBMNew!!\n");
        return 0;
    }

	/* Add Table */
	pTable = NuDBMAddTable("tbOrder", ROW_NUMBER_DEFAULT, enNo, ColName[enID], 10,
			                                                    ColName[enName], 20,
																ColName[enStock], 16,
																ColName[enSide], 16,
																ColName[enOrderQty], 10,
																ColName[enOrderPrice], 16,
																ColName[enCumQty], 10,
																ColName[enAvgPx], 16);

    pPKey = NuTableAddIndex(pTable, NuIndexKind_PKey, 1, enID);
    if(!pPKey)
    {
        printf("NuTableAddPKey!!\n");
        return 0;
    }

	/* Open DBM for connect */
    NuDBMOpen(".");

	Arg.Table = pTable;
	Arg.PIndex = pPKey;
	Arg.Quit = &Quit;



    ConnId = NuDBMConnect();
    printf("ConnectId[%d]\n", (int)ConnId);

    pRS = NuRSGet(ConnId, pTable);

    start = clock();
#if 1
//    for(iCnt = 0; iCnt < TOTAL; iCnt ++)
    for(iCnt = 0; iCnt < 5; iCnt ++)
    {
        NuRSSetTran(pRS);

        sprintf(szID, "%010d", iCnt);
		sprintf(szName, "IVAN-%d", iCnt);

        NuRSInsert(pRS);

		if (iCnt % 2 == 0)
		{
//			NuRSDataSetByRow(pRS, szID, szName, "2330", "B", "10", "136.5", "0", "0.00");
			NuRSDataSetByRow(pRS, szID, szName, "2330", "B", "10", "99999", "0", "0.00");
		}
		else
		{
			NuRSDataSetByRow(pRS, szID, szName, "2885", "S", "12", "12.7", "3", "12.90");
		}

       NuRSCommit(pRS);
		
    }
    end = clock();

    total = (float)(end - start)/CLOCKS_PER_SEC;
    printf("Insert[%d] end - start = [%f]\n", TOTAL, total);
#endif 

#if 1
	iRC = NuThdCreate(&_Thd_Work1, &Arg, &(Thd[0]));
	printf("create thd [%d]\n", iRC);

	iRC = NuThdCreate(&_Thd_Work2, &Arg, &(Thd[1]));
	printf("create thd [%d]\n", iRC);
#endif


	sleep(1);
	/* Select */
	for (iCnt = 0; iCnt < 11; iCnt++)
	{
//		sprintf(szID, "%010d", iCnt);
		sprintf(szID, "%010d", 10);
		iRC = NuRSSelectByIndex(pRS, pPKey, szID);
		printf("iRC = %d [%s]\n", iRC, szID);
		if (iRC == NU_OK)
		{
			printf("ID[%s], Name[%s], Stock[%s], Side[%s], [%s, %s, %s, %s]\n", 
					NuRSDataGet(pRS, enID), 
					NuRSDataGet(pRS, enName),
					NuRSDataGet(pRS, enStock), 
					NuRSDataGet(pRS, enSide),
					NuRSDataGet(pRS, enOrderQty),
					NuRSDataGet(pRS, enOrderPrice),
					NuRSDataGet(pRS, enCumQty),
					NuRSDataGet(pRS, enAvgPx));

		}
		usleep(30);
	}

	Quit = 1;
#if 1	
	NuThdJoin(Thd[0]);
	printf("join thd \n");
	NuThdJoin(Thd[1]);
	printf("join thd \n");
#endif
    NuDBMFree();

    return 1;
}

