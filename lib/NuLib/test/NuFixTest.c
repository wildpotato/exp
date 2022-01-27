#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <NuCStr.h>
#include <NuFix.h>
#include <NuStr.h>

char *FIX_MSG[] = {
    "8=FIX.4.235=E49=JINGLE109=1202321008=130117162510-00121010=130117162512-00166=7006881394=368=173=111=150811154656-00074-1202367=121050=2015081176=98871=991513221=35047=MJWXX000121046=11055=110126301=T26111=054=138=500040=159=0207=TW336=DAY26355=N26323=N198=b000521059=1202326314=JINGLE26326=N26327=N26332=N26347=N26348=N26381=026382=026388=N30001=150811154525-00041-1202326391=N60=20150811-07:46:56.833",
    "8=FIX.4.235=G49=JINGLE109=1202311=150812161215-00022-1202341=150812161125-00015-1202321=155=233026301=T54=160=20150812-08:12:15.49938=1500040=121059=1202326314=JINGLE207=TW59=0336=DAY26334=YUANTA26355=N26347=N26348=N26394=N",
    "8=FIX.4.235=F49=JINGLE109=1202311=150811154656-00073-1202341=150811154525-00049-1202355=110154=140=160=20150811-07:46:56.69338=5000207=TW26301=T59=0336=DAY21059=1202326314=JINGLE26334=YUANTA26394=N",
    "8=FIX.4.234=76452329=42635=E49=217956=0.0025155=.0732109=2179521008=150731104429-00121010=150731104432-00166=3690524394=368=173=111=150803094120-00002-2179567=1284=ALL21050=2015080376=98871=990824621=121046=01055=288526301=T26111=054=138=100040=244=1559=1432=2015-08-30126=15:00:00.000207=TW336=DAY26355=N21059=2179526314=2179526326=N26327=N26332=N26347=N26348=N26381=026382=026388=N30001=Auto26391=N60=20150803-01:41:20.720"
};
int FIX_MSG_LEN = sizeof(FIX_MSG)/sizeof(FIX_MSG[0]);
NuFixParser_t *pParser = NULL;

typedef struct _YT_OrderRepeating_t {
    NuStr_t *ClOrdID;                   // Tag 11
    NuStr_t *OrigClOrdID;               // Tag 41
    int      ListSeqNo;                 // Tag 67
    NuStr_t *ExecBrokerID;              // Tag 76
    NuStr_t *Account;                   // Tag 1
    char     HandlInst;                 // Tag 21
    NuStr_t *Symbol;                    // Tag 55
    char     CreditFlag;                // Tag 26111
    char     Side;                      // Tag 54
    NuStr_t *TransactTime;              // Tag 60
    int      OrderQty;                  // Tag 38
    char     OrdType;                   // Tag 40
    double   Price;                     // Tag 44
    NuStr_t *SalesID;                   // Tag 21046
    NuStr_t *SecurityExchange;          // Tag 207
    NuStr_t *Market;                    // Tag 26301
    char     TimeInForce;               // Tag 59
    NuStr_t *TradingSessionID;          // Tag 336
    char     LockFlag;                  // Tag 26355
    NuStr_t *DeskID;                    // Tag 284
    char     CashDayTradeFlag;          // Tag 26388
    NuStr_t *ExpireDate;                // Tag 432
    NuStr_t *ExpireTime;                // Tag 126
} YT_OrderRepeating_t;

typedef struct _YT_Order_t {
    NuStr_t *MsgType;                   // Tag 35
    NuStr_t *SenderCompID;              // Tag 49
    NuStr_t *ConnectID;                 // Tag 109
    NuStr_t *CustomerID;                // Tag 21008
    NuStr_t *GroupID;                   // Tag 21010
    NuStr_t *ListID;                    // Tag 66
    double   AvgPx;                     // Tag 6
    char     BidType;                   // Tag 394
    int      TotNoOrders;               // Tag 68
    int      NoOrders;                  // Tag 73
    long     MsgBodyLen;                // Tag 9
    long     MsgSeqNum;                 // Tag 34
    float    SettlCurrFxRate;           // Tag 155
    YT_OrderRepeating_t **Orders;
} YT_Order_t;

int _YT_OrderRepeatingNew(YT_OrderRepeating_t **ppYTOrderRep)
{
    YT_OrderRepeating_t *pYTOrderRep = NULL;
    pYTOrderRep = (YT_OrderRepeating_t *)malloc(sizeof(YT_OrderRepeating_t));
    if (!pYTOrderRep)
    {
        *ppYTOrderRep = NULL;
        return -1;
    }
    *ppYTOrderRep = pYTOrderRep;
    memset(pYTOrderRep, 0, sizeof(YT_OrderRepeating_t));

    NuStrNewPreAlloc(&(pYTOrderRep->ClOrdID), 16);
    NuStrClear(pYTOrderRep->ClOrdID);
    NuStrNewPreAlloc(&(pYTOrderRep->OrigClOrdID), 16);
    NuStrClear(pYTOrderRep->OrigClOrdID);
    NuStrNewPreAlloc(&(pYTOrderRep->ExecBrokerID), 8);
    NuStrClear(pYTOrderRep->ExecBrokerID);
    NuStrNewPreAlloc(&(pYTOrderRep->Account), 8);
    NuStrClear(pYTOrderRep->Account);
    NuStrNewPreAlloc(&(pYTOrderRep->Symbol), 16);
    NuStrClear(pYTOrderRep->Symbol);
    NuStrNewPreAlloc(&(pYTOrderRep->TransactTime), 20);
    NuStrClear(pYTOrderRep->TransactTime);
    NuStrNewPreAlloc(&(pYTOrderRep->SalesID), 8);
    NuStrClear(pYTOrderRep->SalesID);
    NuStrNewPreAlloc(&(pYTOrderRep->SecurityExchange), 4);
    NuStrClear(pYTOrderRep->SecurityExchange);
    NuStrNewPreAlloc(&(pYTOrderRep->Market), 8);
    NuStrClear(pYTOrderRep->Market);
    NuStrNewPreAlloc(&(pYTOrderRep->TradingSessionID), 8);
    NuStrClear(pYTOrderRep->TradingSessionID);
    NuStrNewPreAlloc(&(pYTOrderRep->DeskID), 16);
    NuStrClear(pYTOrderRep->DeskID);
    NuStrNewPreAlloc(&(pYTOrderRep->ExpireDate), 12);
    NuStrClear(pYTOrderRep->ExpireDate);
    NuStrNewPreAlloc(&(pYTOrderRep->ExpireTime), 12);
    NuStrClear(pYTOrderRep->ExpireTime);
    return 0;
}

void _YT_OrderRepeatingFree(YT_OrderRepeating_t *pYTOrderRep)
{
    if (pYTOrderRep == NULL)
    {
        return;
    }
    NuStrFree(pYTOrderRep->ClOrdID);
    NuStrFree(pYTOrderRep->OrigClOrdID);
    NuStrFree(pYTOrderRep->ExecBrokerID);
    NuStrFree(pYTOrderRep->Account);
    NuStrFree(pYTOrderRep->Symbol);
    NuStrFree(pYTOrderRep->TransactTime);
    NuStrFree(pYTOrderRep->SalesID);
    NuStrFree(pYTOrderRep->SecurityExchange);
    NuStrFree(pYTOrderRep->Market);
    NuStrFree(pYTOrderRep->TradingSessionID);
    NuStrFree(pYTOrderRep->DeskID);
    NuStrFree(pYTOrderRep->ExpireDate);
    NuStrFree(pYTOrderRep->ExpireTime);
 
    memset(pYTOrderRep, 0, sizeof(YT_OrderRepeating_t));
    free(pYTOrderRep);
}

void _YTOrderRepPrint(YT_OrderRepeating_t *pOrders)
{
    printf("[NuFix]ClOrdID(%s)\n", NuStrGet(pOrders->ClOrdID));
    printf("[NuFix]OrigClOrdID(%s)\n", NuStrGet(pOrders->OrigClOrdID));
    printf("[NuFix]ListSeqNo(%d)\n", pOrders->ListSeqNo);
    printf("[NuFix]ExecBrokerID(%s)\n", NuStrGet(pOrders->ExecBrokerID));
    printf("[NuFix]Account(%s)\n", NuStrGet(pOrders->Account));
    printf("[NuFix]HandlInst(%c)\n", pOrders->HandlInst);
    printf("[NuFix]Symbol(%s)\n", NuStrGet(pOrders->Symbol));
    printf("[NuFix]CreditFlag(%c)\n", pOrders->CreditFlag);
    printf("[NuFix]Side(%c)\n", pOrders->Side);
    printf("[NuFix]TransactTime(%s)\n", NuStrGet(pOrders->TransactTime));
    printf("[NuFix]OrderQty(%d)\n", pOrders->OrderQty);
    printf("[NuFix]OrdType(%c)\n", pOrders->OrdType);
    printf("[NuFix]Price(%lf)\n", pOrders->Price);
    printf("[NuFix]SalesID(%s)\n", NuStrGet(pOrders->SalesID));
    printf("[NuFix]SecurityExchange(%s)\n", NuStrGet(pOrders->SecurityExchange));
    printf("[NuFix]Market(%s)\n", NuStrGet(pOrders->Market));
    printf("[NuFix]TimeInForce(%c)\n", pOrders->TimeInForce);
    printf("[NuFix]TradingSessionID(%s)\n", NuStrGet(pOrders->TradingSessionID));
    printf("[NuFix]LockFlag(%c)\n", pOrders->LockFlag);
    printf("[NuFix]DeskID(%s)\n", NuStrGet(pOrders->DeskID));
    printf("[NuFix]CashDayTradeFlag(%c)\n", pOrders->CashDayTradeFlag);
    printf("[NuFix]ExpireDate(%s)\n", NuStrGet(pOrders->ExpireDate));
    printf("[NuFix]ExpireTime(%s)\n", NuStrGet(pOrders->ExpireTime));
}

void _YTOrderPrint(YT_Order_t *pOrder)
{
    int Idx = 0;
    printf("[NuFix]MsgType(%s)\n", NuStrGet(pOrder->MsgType));
    printf("[NuFix]MsgSeqNum(%ld)\n", pOrder->MsgSeqNum);
    printf("[NuFix]MsgBodyLen(%ld)\n", pOrder->MsgBodyLen);
    printf("[NuFix]SenderCompID(%s)\n", NuStrGet(pOrder->SenderCompID));
    printf("[NuFix]AvgPx(%lf)\n", pOrder->AvgPx);
    printf("[NuFix]SettlCurrFxRate(%f)\n", pOrder->SettlCurrFxRate);
    printf("[NuFix]ConnectID(%s)\n", NuStrGet(pOrder->ConnectID));
    printf("[NuFix]CustomerID(%s)\n", NuStrGet(pOrder->CustomerID));
    printf("[NuFix]GroupID(%s)\n", NuStrGet(pOrder->GroupID));
    printf("[NuFix]ListID(%s)\n", NuStrGet(pOrder->ListID));
    printf("[NuFix]BidType(%c)\n", pOrder->BidType);
    printf("[NuFix]TotNoOrders(%d)\n", pOrder->TotNoOrders);
    printf("[NuFix]NoOrders(%d)\n", pOrder->NoOrders);

    for(; Idx < pOrder->NoOrders; Idx++)
    {
        printf("[NuFix]====== repeating =====\n");
        _YTOrderRepPrint(*(pOrder->Orders + Idx));
    }
}

void _YTOrderClear(YT_Order_t *pOrder)
{
    int Idx = 0;

    NuStrClear(pOrder->MsgType);
    NuStrClear(pOrder->SenderCompID);
    NuStrClear(pOrder->ConnectID);
    NuStrClear(pOrder->CustomerID);
    NuStrClear(pOrder->GroupID);
    NuStrClear(pOrder->ListID);

    if (pOrder->NoOrders)
    {
        for (Idx = 0; Idx < pOrder->NoOrders; Idx++)
        {
            _YT_OrderRepeatingFree(*(pOrder->Orders + Idx));
        }
        free(pOrder->Orders);
    }

    pOrder->AvgPx = 0;
    pOrder->BidType = 0;
    pOrder->TotNoOrders = 0;
    pOrder->NoOrders = 0;
    pOrder->MsgBodyLen = 0;
    pOrder->MsgSeqNum = 0;
    pOrder->SettlCurrFxRate = 0;
}
/*
 * Parse Test
 */
bool _NuFixParserSetToChr(int Tag, const char *Value, size_t ValueLen, void *Argu, void *Closure)
{
    *((char *)Argu) = *Value;
    return true;
}

bool _NuFixParserSetToRep(int Tag, const char *Value, size_t ValueLen, void *Argu, void *Closure)
{
    int Idx = *((int *)Closure);
    YT_OrderRepeating_t **ppOrders = ((YT_Order_t *)Argu)->Orders;
    YT_OrderRepeating_t *pOrders = NULL;
    if (ppOrders == NULL)
    {
        return false;
    }
    if (Idx >= 0 && Tag != 11)
    {
        pOrders = *(ppOrders + Idx);
    }
    switch(Tag)
    {
    case 11:
        (*((int *)Closure))++;
        NuStrNCpy((*(ppOrders + *((int *)Closure)))->ClOrdID, Value, ValueLen);
        break;
    case 41:
        NuStrNCpy(pOrders->OrigClOrdID, Value, ValueLen);
        break;
    case 67:
        pOrders->ListSeqNo = NuCStrToInt(Value, ValueLen);
        break;
    case 76:
        NuStrNCpy(pOrders->ExecBrokerID, Value, ValueLen);
        break;
    case 1:
        NuStrNCpy(pOrders->Account, Value, ValueLen);
        break;
    case 21:
        pOrders->HandlInst = *Value;
        break;
    case 55:
        NuStrNCpy(pOrders->Symbol, Value, ValueLen);
        break;
    case 26111:
        pOrders->CreditFlag = *Value;
        break;
    case 54:
        pOrders->Side = *Value;
        break;
    case 60:
        NuStrNCpy(pOrders->TransactTime, Value, ValueLen);
        break;
    case 38:
        pOrders->OrderQty = NuCStrToInt(Value, ValueLen);
        break;
    case 40:
        pOrders->OrdType = *Value;
        break;
    case 44:
        pOrders->Price = NuCStrToDouble(Value, ValueLen);
        break;
    case 21046:
        NuStrNCpy(pOrders->SalesID, Value, ValueLen);
        break;
    case 207:
        NuStrNCpy(pOrders->SecurityExchange, Value, ValueLen);
        break;
    case 26301:
        NuStrNCpy(pOrders->Market, Value, ValueLen);
        break;
    case 59:
        pOrders->TimeInForce = *Value;
        break;
    case 336:
        NuStrNCpy(pOrders->TradingSessionID, Value, ValueLen);
        break;
    case 26355:
        pOrders->LockFlag = *Value;
        break;
    case 284:
        NuStrNCpy(pOrders->DeskID, Value, ValueLen);
        break;
    case 26388:
        pOrders->CashDayTradeFlag = *Value;
        break;
    case 432:
        NuStrNCpy(pOrders->ExpireDate, Value, ValueLen);
        break;
    case 126:
        NuStrNCpy(pOrders->ExpireTime, Value, ValueLen);
        break;
    default:
        break;
    }
    return true;
}

void _NuFixSetRepeatingCB(NuFixParser_t *pParser, int Tag, void *Argu)
{
    NuFixParserNode_t *pNode = NuFixParserGetNode(pParser, Tag);

    NuFixParserSetCallback(pNode, &_NuFixParserSetToRep, Argu);
}

bool _NuFixParserSetToOrders(int Tag, const char *Value, size_t ValueLen, void *Argu, void *Closure)
{
    int Idx = 0;
    YT_Order_t *pOrder = (YT_Order_t *)Argu;
    pOrder->NoOrders = NuCStrToInt(Value, ValueLen);
    *((int *)Closure) = -1;

    pOrder->Orders = (YT_OrderRepeating_t **)malloc(sizeof(YT_OrderRepeating_t *));
    for (; Idx < pOrder->NoOrders; Idx++)
    {
        _YT_OrderRepeatingNew(pOrder->Orders + Idx);
    }

    return true;    
}

void _NuFixSetRepNoCB(NuFixParser_t *pParser, int Tag, void *Argu)
{
    NuFixParserNode_t *pNode = NuFixParserGetNode(pParser, Tag);

    switch(Tag)
    {
    case 73:
        NuFixParserSetCallback(pNode, &_NuFixParserSetToOrders, Argu);
        break;
    default:
        break;
    }
}

void _NuFixSetChrCB(NuFixParser_t *pParser, int Tag, char *Storage)
{
    NuFixParserNode_t *pNode = NuFixParserGetNode(pParser, Tag);

    NuFixParserSetCallback(pNode, &_NuFixParserSetToChr, Storage);
}

void _NuFixSetLongCB(NuFixParser_t *pParser, int Tag, long *Storage)
{
    NuFixParserNode_t *pNode = NuFixParserGetNode(pParser, Tag);

    NuFixParserSetToLong(pNode, Storage);
}

void _NuFixSetDoubleCB(NuFixParser_t *pParser, int Tag, double *Storage)
{
    NuFixParserNode_t *pNode = NuFixParserGetNode(pParser, Tag);

    NuFixParserSetToDouble(pNode, Storage);
}

void _NuFixSetFloatCB(NuFixParser_t *pParser, int Tag, float *Storage)
{
    NuFixParserNode_t *pNode = NuFixParserGetNode(pParser, Tag);

    NuFixParserSetToFloat(pNode, Storage);
}

void _NuFixSetStrCB(NuFixParser_t *pParser, int Tag, NuStr_t *Storage)
{
    NuFixParserNode_t *pNode = NuFixParserGetNode(pParser, Tag);

    NuFixParserSetToStr(pNode, Storage);
}

void _NuFixSetIntCB(NuFixParser_t *pParser, int Tag, int *Storage)
{
    NuFixParserNode_t *pNode = NuFixParserGetNode(pParser, Tag);

    NuFixParserSetToInt(pNode, Storage);
}

void _NuFixRemoveCB(NuFixParser_t *pParser, int Tag)
{
    NuFixParserNode_t *pNode = NuFixParserGetNode(pParser, Tag);

    NuFixParserRemoveCallback(pNode);
}

bool _NuFixParserMsgType(int Tag, const char *Value, size_t ValueLen, void *Argu, void *Closure)
{
    YT_Order_t *pOrder = (YT_Order_t *)Argu;
    const char *pMsgType = NULL;

    NuStrNCpy(pOrder->MsgType, Value, ValueLen);

    pMsgType = NuStrGet(pOrder->MsgType);
    if (!strcmp(pMsgType, "E"))
    {
        _NuFixSetStrCB(pParser, 49, pOrder->SenderCompID);
        _NuFixSetStrCB(pParser, 109, pOrder->ConnectID);
        _NuFixSetStrCB(pParser, 21008, pOrder->CustomerID);
        _NuFixSetStrCB(pParser, 21010, pOrder->GroupID);
        _NuFixSetStrCB(pParser, 66, pOrder->ListID);
        _NuFixSetFloatCB(pParser, 155, &(pOrder->SettlCurrFxRate));
        _NuFixSetChrCB(pParser, 394, &(pOrder->BidType));
        _NuFixSetIntCB(pParser, 68, &(pOrder->TotNoOrders));
        _NuFixRemoveCB(pParser, 41);
        _NuFixSetRepNoCB(pParser, 73, pOrder);
        _NuFixSetRepeatingCB(pParser, 67, pOrder);
        _NuFixSetRepeatingCB(pParser, 76, pOrder);
        _NuFixSetRepeatingCB(pParser, 1, pOrder);
        _NuFixSetRepeatingCB(pParser, 21, pOrder);
        _NuFixSetRepeatingCB(pParser, 55, pOrder);
        _NuFixSetRepeatingCB(pParser, 54, pOrder);
        _NuFixSetRepeatingCB(pParser, 38, pOrder);
        _NuFixSetRepeatingCB(pParser, 40, pOrder);
        _NuFixSetRepeatingCB(pParser, 44, pOrder);
        _NuFixSetRepeatingCB(pParser, 21046, pOrder);
        _NuFixSetRepeatingCB(pParser, 207, pOrder);
        _NuFixSetRepeatingCB(pParser, 26301, pOrder);
        _NuFixSetRepeatingCB(pParser, 59, pOrder);
        _NuFixSetRepeatingCB(pParser, 336, pOrder);
        _NuFixSetRepeatingCB(pParser, 26355, pOrder);
        _NuFixSetRepeatingCB(pParser, 284, pOrder);
        _NuFixSetRepeatingCB(pParser, 26388, pOrder);
        _NuFixSetRepeatingCB(pParser, 432, pOrder);
        _NuFixSetRepeatingCB(pParser, 126, pOrder);
    }
    else if (!strcmp(pMsgType, "F"))
    {
        pOrder->Orders = (YT_OrderRepeating_t **)malloc(sizeof(YT_OrderRepeating_t *));
        _YT_OrderRepeatingNew(pOrder->Orders);
        pOrder->NoOrders = 1;

        _NuFixSetRepeatingCB(pParser, 41, pOrder);
        _NuFixRemoveCB(pParser, 76);
        _NuFixRemoveCB(pParser, 21);
        _NuFixRemoveCB(pParser, 55);
        _NuFixRemoveCB(pParser, 38);
        _NuFixRemoveCB(pParser, 40);
        _NuFixRemoveCB(pParser, 44);
        _NuFixRemoveCB(pParser, 21046);
        _NuFixRemoveCB(pParser, 207);
        _NuFixRemoveCB(pParser, 26301);
        _NuFixRemoveCB(pParser, 59);
        _NuFixRemoveCB(pParser, 336);
        _NuFixRemoveCB(pParser, 26355);
        _NuFixRemoveCB(pParser, 284);
        _NuFixRemoveCB(pParser, 26388);
        _NuFixRemoveCB(pParser, 432);
        _NuFixRemoveCB(pParser, 126);
    }
    else if (!strcmp(pMsgType, "G"))
    {
        pOrder->Orders = (YT_OrderRepeating_t **)malloc(sizeof(YT_OrderRepeating_t *));
        _YT_OrderRepeatingNew(pOrder->Orders);
        pOrder->NoOrders = 1;

        _NuFixSetRepeatingCB(pParser, 41, pOrder);
        _NuFixSetRepeatingCB(pParser, 40, pOrder);
        _NuFixSetRepeatingCB(pParser, 44, pOrder);
        _NuFixSetRepeatingCB(pParser, 38, pOrder);
        _NuFixRemoveCB(pParser, 76);
        _NuFixRemoveCB(pParser, 21);
        _NuFixRemoveCB(pParser, 55);
        _NuFixRemoveCB(pParser, 21046);
        _NuFixRemoveCB(pParser, 207);
        _NuFixRemoveCB(pParser, 26301);
        _NuFixRemoveCB(pParser, 59);
        _NuFixRemoveCB(pParser, 336);
        _NuFixRemoveCB(pParser, 26355);
        _NuFixRemoveCB(pParser, 284);
        _NuFixRemoveCB(pParser, 26388);
        _NuFixRemoveCB(pParser, 432);
        _NuFixRemoveCB(pParser, 126);
    }
    else
    {
        printf("[NuFix]Unsupport Msgtype(%s)\n", pMsgType);
        return false;
    }

    return true;
}

void _ParserTest(YT_Order_t *pOrder)
{
    NuFixParserNode_t *pNode = NULL;
    int Idx = 0;
    int RepIdx = -1;

    if (NuFixParserNew(&pParser) < 0)
    {
        return;
    }

	printf("[NuFix]===== NuFixParser =====\n");
    pNode = NuFixParserGetNode(pParser, 35);
    NuFixParserSetCallback(pNode, &_NuFixParserMsgType, (void *)pOrder);

    _NuFixSetLongCB(pParser, 34, &(pOrder->MsgSeqNum));
    _NuFixSetLongCB(pParser, 9, &(pOrder->MsgBodyLen));
    _NuFixSetDoubleCB(pParser, 6, &(pOrder->AvgPx));
    _NuFixSetRepeatingCB(pParser, 60, pOrder);
    _NuFixSetRepeatingCB(pParser, 11, pOrder);

    for (Idx = 0; Idx < FIX_MSG_LEN; Idx++)
    {
        printf("[NuFix]===== NuFixParser =====\n");
        RepIdx = -1;
        _YTOrderClear(pOrder);
        NuFixParserForEach(pParser, FIX_MSG[Idx], &RepIdx);
        _YTOrderPrint(pOrder);
    }
    NuFixParserFree(pParser);
}

/*
 * Compose Test
 */
char *_ComposeGroupDouble(NuFixComposerRepeatingGroup_t *pGroup, const char *Tag, double Value, size_t Len, bool Use)
{
    NuFixComposerField_t *pField = NuFixComposerRepeatingGroupAddField(pGroup, Tag);
    char *Str = (char *)malloc(sizeof(char) * Len);
    size_t ValueLen = sprintf(Str, "%lf", Value);

    NuFixComposerFieldSet(pField, Str, ValueLen);
    if (!Use || !ValueLen)
    {
        NuFixComposerFieldSetUse(pField, false);
    }
    printf("[NuFix]Tag(%s)SetUse(%d)\n", Tag, NuFixComposerFieldIsUse(pField));
    return Str;
}

char *_ComposeGroupInt(NuFixComposerRepeatingGroup_t *pGroup, const char *Tag, int Value, size_t Len, bool Use)
{
    NuFixComposerField_t *pField = NuFixComposerRepeatingGroupAddField(pGroup, Tag);
    char *Str = (char *)malloc(sizeof(char) * Len);
    size_t ValueLen = sprintf(Str, "%d", Value);

    NuFixComposerFieldSet(pField, Str, ValueLen);
    if (!Use || !ValueLen)
    {
        NuFixComposerFieldSetUse(pField, false);
    }
    printf("[NuFix]Tag(%s)SetUse(%d)\n", Tag, NuFixComposerFieldIsUse(pField));
    return Str;
}

void _ComposeGroupChr(NuFixComposerRepeatingGroup_t *pGroup, const char *Tag, char *Value, bool Use)
{
    NuFixComposerField_t *pField = NuFixComposerRepeatingGroupAddField(pGroup, Tag);
    
    NuFixComposerFieldSet(pField, Value, 1);
    if (!Use)
    {
        NuFixComposerFieldSetUse(pField, false);
    }
    printf("[NuFix]Tag(%s)SetUse(%d)\n", Tag, NuFixComposerFieldIsUse(pField));
}

void _ComposeGroupStr(NuFixComposerRepeatingGroup_t *pGroup, const char *Tag, NuStr_t *Value, bool Use)
{
    NuFixComposerField_t *pField = NuFixComposerRepeatingGroupAddField(pGroup, Tag);

    NuFixComposerFieldSet(pField, NuStrGet(Value), NuStrSize(Value));
    if (!Use || !NuStrSize(Value))
    {
        NuFixComposerFieldSetUse(pField, false);
    }
    printf("[NuFix]Tag(%s)SetUse(%d)\n", Tag, NuFixComposerFieldIsUse(pField));
}


char *_ComposeDouble(NuFixComposer_t *pComposer, const char *Tag, double Value, size_t Len, bool Use)
{
    NuFixComposerField_t *pField = NuFixComposerGetField(pComposer, Tag);
    char *Str = (char *)malloc(sizeof(char) * Len);
    size_t ValueLen = sprintf(Str, "%lf", Value);

    NuFixComposerFieldSet(pField, Str, ValueLen);
    if (!Use || !ValueLen)
    {
        NuFixComposerFieldSetUse(pField, false);
    }
    printf("[NuFix]Tag(%s)SetUse(%d)\n", Tag, NuFixComposerFieldIsUse(pField));
    return Str;
}

char *_ComposeInt(NuFixComposer_t *pComposer, const char *Tag, int Value, size_t Len, bool Use)
{
    NuFixComposerField_t *pField = NuFixComposerGetField(pComposer, Tag);
    char *Str = (char *)malloc(sizeof(char) * Len);
    size_t ValueLen = sprintf(Str, "%d", Value);

    NuFixComposerFieldSet(pField, Str, ValueLen);
    if (!Use || !ValueLen)
    {
        NuFixComposerFieldSetUse(pField, false);
    }
    printf("[NuFix]Tag(%s)SetUse(%d)\n", Tag, NuFixComposerFieldIsUse(pField));
    return Str;
}

void _ComposeChr(NuFixComposer_t *pComposer, const char *Tag, char *Value, bool Use)
{
    NuFixComposerField_t *pField = NuFixComposerGetField(pComposer, Tag);
    
    NuFixComposerFieldSet(pField, Value, 1);
    if (!Use)
    {
        NuFixComposerFieldSetUse(pField, false);
    }
    printf("[NuFix]Tag(%s)SetUse(%d)\n", Tag, NuFixComposerFieldIsUse(pField));
}

void _ComposeStr(NuFixComposer_t *pComposer, const char *Tag, NuStr_t *Value, bool Use)
{
    NuFixComposerField_t *pField = NuFixComposerGetField(pComposer, Tag);

    NuFixComposerFieldSet(pField, NuStrGet(Value), NuStrSize(Value));
    if (!Use || !NuStrSize(Value))
    {
        NuFixComposerFieldSetUse(pField, false);
    }
    printf("[NuFix]Tag(%s)SetUse(%d)\n", Tag, NuFixComposerFieldIsUse(pField));
}

void _ComposeTest(YT_Order_t *pOrder)
{
    NuFixComposer_t *pComposer = NULL;
    NuFixComposerRepeatingGroup_t *pGroup = NULL;
    NuFixComposerRepeatingGroup_t *pNestGroup = NULL;
    int Idx1 = 0;
    int Idx2 = 0;
    char MsgType = '8';
    char OrdStatus = 'A';
    char TifDay = '0';
    char TifGTD = '6';
    char *Tif = NULL;
    char *Price[2] = {0};
    char *OrderQty[2] = {0};


    if (NuFixComposerNew(&pComposer) < 0)
    {
        return;
    }

	printf("[NuFix]===== NuFixComposer =====\n");
    _ComposeChr(pComposer, "35", &MsgType, true);
    _ComposeChr(pComposer, "39", &OrdStatus, true);
    _ComposeChr(pComposer, "150", &OrdStatus, true);
    _ComposeStr(pComposer, "56", pOrder->SenderCompID, true);
    _ComposeStr(pComposer, "1", pOrder->Orders[0]->Account, true);
    _ComposeStr(pComposer, "66", pOrder->ListID, true);
    pGroup = NuFixComposerGetRepeatingGroup(pComposer, "73");
    NuFixComposerRepeatingGroupSetNo(pGroup, 2);

    for(; Idx1 < 2; Idx1++)
    {
        _ComposeGroupStr(pGroup, "55", pOrder->Orders[0]->Symbol, true);
        Price[Idx1] = _ComposeGroupDouble(pGroup, "44", pOrder->Orders[0]->Price, 16, true);
        OrderQty[Idx1] = _ComposeGroupInt(pGroup, "38", pOrder->Orders[0]->OrderQty, 8, true);
        _ComposeGroupChr(pGroup, "40", &(pOrder->Orders[0]->OrdType), true);
        _ComposeGroupChr(pGroup, "21", &(pOrder->Orders[0]->HandlInst), true);
        _ComposeGroupChr(pGroup, "54", &(pOrder->Orders[0]->Side), true);
        _ComposeGroupStr(pGroup, "207", pOrder->Orders[0]->SecurityExchange, true);
        _ComposeGroupStr(pGroup, "26301", pOrder->Orders[0]->Market, true);
        Tif = Idx1 ? &TifDay : &TifGTD;
        _ComposeGroupChr(pGroup, "59", Tif, true);
        _ComposeGroupStr(pGroup, "432", pOrder->Orders[0]->ExpireDate, *Tif == '6');
        _ComposeGroupStr(pGroup, "126", pOrder->Orders[0]->ExpireTime, *Tif == '6');
        pNestGroup = NuFixComposerRepeatingGroupAddRepeatingGroup(pGroup, "123");
        NuFixComposerRepeatingGroupSetNo(pNestGroup, 2);
        for (Idx2 = 0; Idx2 < 2; Idx2++)
        {
            _ComposeGroupStr(pNestGroup, "207", pOrder->Orders[0]->SecurityExchange, true);
            _ComposeGroupStr(pNestGroup, "26301", pOrder->Orders[0]->Market, true);
            
        }
    }

	printf("[NuFix]===== NuFixComposer Full =====\n");
    const char *Result = NuFixComposerCompose(pComposer);
    printf("[NuFix](%s)\n", Result);

	printf("[NuFix]===== NuFixComposer Clear least NestGroup =====\n");
    NuFixComposerRepeatingGroupClear(pNestGroup);
    Result = NuFixComposerCompose(pComposer);
    printf("[NuFix](%s)\n", Result);

	printf("[NuFix]===== NuFixComposer Clear Group =====\n");
    NuFixComposerRepeatingGroupClear(pGroup);
    Result = NuFixComposerCompose(pComposer);
    printf("[NuFix](%s)\n", Result);

	printf("[NuFix]===== NuFixComposer Clear Composer =====\n");
    NuFixComposerClear(pComposer);
    Result = NuFixComposerCompose(pComposer);
    printf("[NuFix](%s)\n", Result);

    if (*OrderQty)
    {
        free(OrderQty[0]);
        free(OrderQty[1]);
    }
    if (*Price)
    {
        free(Price[0]);
        free(Price[1]);
    }
    NuFixComposerClear(pComposer);
    NuFixComposerFree(pComposer);
}

int main(int argc, char **argv)
{
    YT_Order_t *pOrder = NULL;
    int  Idx = 0;
    
    pOrder = (YT_Order_t *)malloc(sizeof(YT_Order_t));
    if (!pOrder)
    {
        goto EXIT;
    }

    NuStrNewPreAlloc(&(pOrder->MsgType), 4);
    NuStrNewPreAlloc(&(pOrder->SenderCompID), 12);
    NuStrNewPreAlloc(&(pOrder->ConnectID), 12);
    NuStrNewPreAlloc(&(pOrder->CustomerID), 12);
    NuStrNewPreAlloc(&(pOrder->GroupID), 12);
    NuStrNewPreAlloc(&(pOrder->ListID), 12);

    _ParserTest(pOrder);
    _ComposeTest(pOrder);

EXIT:
    if (pOrder)
    {
        NuStrFree(pOrder->MsgType);
        NuStrFree(pOrder->SenderCompID);
        NuStrFree(pOrder->ConnectID);
        NuStrFree(pOrder->CustomerID);
        NuStrFree(pOrder->GroupID);
        NuStrFree(pOrder->ListID);
        if (pOrder->NoOrders)
        {
            for (Idx = 0; Idx < pOrder->NoOrders; Idx++)
            {
                _YT_OrderRepeatingFree(*(pOrder->Orders + Idx));
            }
            free(pOrder->Orders);
        }
        free(pOrder);
    }

    return EXIT_SUCCESS;
}
