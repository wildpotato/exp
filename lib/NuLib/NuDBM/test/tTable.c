
#include "NuDBM.h"
#include "NuThread.h"
#include <time.h>

//#define TOTAL   1000000
//#define ROW_NUMBER_DEFAULT 1000000
#define TOTAL   100
#define ROW_NUMBER_DEFAULT 100


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

int main(int argc, char **argv)
{
    int                 iCnt = 0;
    int                 iRC = 0;
	char                szID[10+1] = {0}, szName[20+1] = {0};
    NuTable_t           *pTable = NULL;
    NuIndex_t           *pPKey = NULL;
    NuRecordSet_t       *pRS = NULL;
    NuDBMConnection_t   ConnId;

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

    ConnId = NuDBMConnect();
    printf("ConnectId[%d]\n", (int)ConnId);

    pRS = NuRSGet(ConnId, pTable);

    start = clock();

    for(iCnt = 0; iCnt < TOTAL; iCnt ++)
    {
        NuRSSetTran(pRS);

        sprintf(szID, "%010d", iCnt);
		sprintf(szName, "IVAN-%d", iCnt);

        NuRSInsert(pRS);

		if (iCnt % 2 == 0)
		{
			NuRSDataSetByRow(pRS, szID, szName, "2330", "B", "10", "136.5", "0", "0.00");
		}
		else
		{
			NuRSDataSetByRow(pRS, szID, szName, "2885", "S", "12", "12.7", "3", "12.90");
		}
//        NuTableInsert(pTable, col, col, "abcdefgh", "12345678", col, col, "Col6", "9876543210", col, col);

        NuRSCommit(pRS);
		
    }
    end = clock();

    total = (float)(end - start)/CLOCKS_PER_SEC;
    printf("Insert[%d] end - start = [%f]\n", TOTAL, total);


	/* Update */
	sprintf(szID, "%010d", 1004);
    NuRSSetTran(pRS);
	iRC = NuRSSelectByIndex(pRS, pPKey, szID);
	if (iRC == NU_OK)
	{
        NuRSDataSet(pRS, enOrderPrice, "123.0", 5);
        NuRSCommit(pRS);
		printf("update \n");
	}
	else
	{
        NuRSRollback(pRS);
		printf("updae fail \n");
	}

	/* Select */
	for (iCnt = 10; iCnt < 20; iCnt++)
	{
		sprintf(szID, "%010d", iCnt);
		iRC = NuRSSelectByIndex(pRS, pPKey, szID);
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
	}


    NuDBMFree();

    return 1;
}

