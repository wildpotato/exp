#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <NuCBMgr.h>
#include <NuFix.h>
#include <NuStr.h>

#define LOCK_IDX 0
#define WARM_IDX 1
#define AMT_IDX  2
#define LAST_IDX 3

NuCBMgr_t *ExecCbMgr = NULL;
NuCBMgr_t *RBCbMgr = NULL;

char *FIX_MSG[] = {
    "8=FIX.4.49=23135=D49=IOMSD2QOMS56=YUTAHKT34=83252=20150824-09:09:2930001=N100=TH207=TH11=M0000000781072=M0000000781=3300029C-00555=SCB54=226323=N336=C575=N40=159=738=1000021=199045=N26058=New Order60=20150824-09:09:2943=N10=078",
    "8=FIX.4.49=25735=G49=IOMSD2QOMS56=YUTAHKT34=83452=20150824-09:09:3330001=N100=TH207=TH11=M0000000801072=M00000008041=M000000078198=TH261=3300029C-00555=SCB54=226323=N336=C575=N40=159=738=500021=199045=N26058=Replace Order60=20150824-09:09:3343=N10=064",
    "send 8=FIX.4.49=25735=F49=IOMSD2QOMS56=YUTAHKT34=83152=20150824-09:09:0130001=N100=TH207=TH11=M0000000771072=M00000007741=M000000075198=TH251=3300029C-00555=SCB54=226323=N336=C575=N40=159=738=1000021=199045=N26058=Cancel Order60=20150824-09:09:0143=N10=235",
    "8=FIX.4.49=23135=D49=IOMSD2QOMS56=YUTAHKT34=82952=20150824-09:08:5230001=N100=TH207=TH11=M0000000751072=M0000000751=3300029C-00555=SCB54=126323=N336=C575=N40=159=738=500021=199045=N26058=New Order60=20150824-09:08:5243=N10=067",
    "send 8=FIX.4.49=25735=F49=IOMSD2QOMS56=YUTAHKT34=83152=20150824-09:09:0130001=N100=TH207=TH11=M0000000771072=M00000007741=M000000075198=TH251=3300029C-00555=SCB54=126323=N336=C575=N40=159=738=1000021=199045=N26058=Cancel Order60=20150824-09:09:0143=N10=235",
};

int MSG_LEN = sizeof(FIX_MSG) / sizeof(FIX_MSG[0]);

typedef struct _YTRiskItemHdr_t
{
    NuFixParser_t   *pParser;
    long            *reminQty;
    bool             isExec;
    bool             isFail;
    long             execQty;
} YTRiskItemHdr_t;

typedef struct _YTRiskMgr_t
{
    NuFixParser_t   *pParser;
    NuCBMgr_t       *pExecCbMgr;
    NuCBMgr_t       *pRbCbMgr;
} YTRiskMgr_t;

/************************************************/
/* Lock position                                */
/************************************************/

static void _LockPositionRB(const void *RaiseArgu, void *Argu)
{
    YTRiskItemHdr_t *Item = (YTRiskItemHdr_t *)Argu;

    if (!(Item->isExec))
    {
        return;
    }
    *(Item->reminQty) += Item->execQty;
    printf("[%s] isFail(%d) execQty(%ld) reminQty(%ld)\n", __func__, Item->isFail, Item->execQty, *(Item->reminQty));
    Item->execQty = 0;
}

static void _LockPositionExec(const void *RaiseArgu, void *Argu)
{
    const char *Msg = (const char *)RaiseArgu;
    YTRiskItemHdr_t *Item = (YTRiskItemHdr_t *)Argu;
    NuFixParser_t *pParser = Item->pParser;
    NuFixParserNode_t *pMsgTypeNode = NuFixParserGetNode(pParser, 35);
    NuFixParserNode_t *pSymbolNode = NuFixParserGetNode(pParser, 55);
    NuFixParserNode_t *pOrderQtyNode = NuFixParserGetNode(pParser, 38);
    NuStr_t *pMsgType = NULL;
    NuStr_t *pSymbol = NULL;
    long LockedQty = 0;
    long OrderQty = 0;

    NuStrNewPreAlloc(&pMsgType, 12);
    NuStrNewPreAlloc(&pSymbol, 16);
    NuFixParserSetToStr(pMsgTypeNode, pMsgType);
    NuFixParserSetToStr(pSymbolNode, pSymbol);
    NuFixParserSetToLong(pOrderQtyNode, &OrderQty);
    NuFixParserForEach(pParser, Msg, NULL);

    NuFixParserRemoveCallback(pMsgTypeNode);
    NuFixParserRemoveCallback(pSymbolNode);
    NuFixParserRemoveCallback(pOrderQtyNode);

    if (!strcmp(NuStrGet(pMsgType), "F"))
    {
        OrderQty = -OrderQty;
    }

    if (OrderQty > 8000)
    {
        Item->isFail = true;
    }
    Item->isExec = true;
    if (OrderQty > 7000)
    {
        LockedQty = 7000;
    }
    else if (OrderQty > -7000)
    {
        LockedQty = OrderQty;
    }
    else
    {
        LockedQty = -7000;
    }

    Item->execQty = LockedQty;
    *(Item->reminQty) -= LockedQty;
    printf("[%s] isFail(%d) execQty(%ld) reminQty(%ld)\n", __func__, Item->isFail, Item->execQty, *(Item->reminQty));

    if (pMsgType)
    {
        NuStrFree(pMsgType);
    }
    if (pSymbol)
    {
        NuStrFree(pSymbol);
    }
}

/************************************************/
/* Warn stock                                   */
/************************************************/

static void _WarnStockRB(const void *RaiseArgu, void *Argu)
{
    YTRiskItemHdr_t *Item = (YTRiskItemHdr_t *)Argu;

    if (!(Item->isExec))
    {
        return;
    }
    printf("[%s] isFail(%d) execQty(%ld) reminQty(%ld)\n", __func__, Item->isFail, Item->execQty, *(Item->reminQty));
    Item->execQty = 0;
}

static void _WarnStockExec(const void *RaiseArgu, void *Argu)
{
    const char *Msg = (const char *)RaiseArgu;
    YTRiskItemHdr_t *Item = (YTRiskItemHdr_t *)Argu;
    NuFixParser_t *pParser = Item->pParser;
    NuFixParserNode_t *pMsgTypeNode = NuFixParserGetNode(pParser, 35);
    NuFixParserNode_t *pSymbolNode = NuFixParserGetNode(pParser, 55);
    NuFixParserNode_t *pOrderQtyNode = NuFixParserGetNode(pParser, 38);
    NuStr_t *pMsgType = NULL;
    NuStr_t *pSymbol = NULL;
    long OrderQty = 0;

    NuStrNewPreAlloc(&pMsgType, 12);
    NuStrNewPreAlloc(&pSymbol, 16);
    NuFixParserSetToStr(pMsgTypeNode, pMsgType);
    NuFixParserSetToStr(pSymbolNode, pSymbol);
    NuFixParserSetToLong(pOrderQtyNode, &OrderQty);
    NuFixParserForEach(pParser, Msg, NULL);

    NuFixParserRemoveCallback(pMsgTypeNode);
    NuFixParserRemoveCallback(pSymbolNode);
    NuFixParserRemoveCallback(pOrderQtyNode);

    if (!strcmp(NuStrGet(pMsgType), "F"))
    {
        OrderQty = -OrderQty;
    }
    else if ((rand() % 100) < 35)
    {
        Item->isFail = true;
    }

    Item->isExec = true;
    Item->execQty = OrderQty;
    printf("[%s] isFail(%d) execQty(%ld) reminQty(%ld)\n", __func__, Item->isFail, Item->execQty, *(Item->reminQty));

    if (pSymbol)
    {
        NuStrFree(pSymbol);
    }
    if (pMsgType)
    {
        NuStrFree(pMsgType);
    }
}

/************************************************/
/* Amount test                                  */
/************************************************/

static void _AmountRB(const void *RaiseArgu, void *Argu)
{
    YTRiskItemHdr_t *Item = (YTRiskItemHdr_t *)Argu;

    if (!(Item->isExec))
    {
        return;
    }
    printf("[%s] isFail(%d) execQty(%ld) reminQty(%ld)\n", __func__, Item->isFail, Item->execQty, *(Item->reminQty));
    Item->execQty = 0;
}

static void _AmountExec(const void *RaiseArgu, void *Argu)
{
    const char *Msg = (const char *)RaiseArgu;
    YTRiskItemHdr_t *Item = (YTRiskItemHdr_t *)Argu;
    NuFixParser_t *pParser = Item->pParser;
    NuFixParserNode_t *pMsgTypeNode = NuFixParserGetNode(pParser, 35);
    NuFixParserNode_t *pSymbolNode = NuFixParserGetNode(pParser, 55);
    NuFixParserNode_t *pOrderQtyNode = NuFixParserGetNode(pParser, 38);
    NuFixParserNode_t *pPriceNode = NuFixParserGetNode(pParser, 44);
    NuStr_t *pMsgType = NULL;
    NuStr_t *pSymbol = NULL;
    long OrderQty = 0;
    double Price = 0.0;

    NuStrNewPreAlloc(&pMsgType, 12);
    NuStrNewPreAlloc(&pSymbol, 16);
    NuFixParserSetToStr(pMsgTypeNode, pMsgType);
    NuFixParserSetToStr(pSymbolNode, pSymbol);
    NuFixParserSetToLong(pOrderQtyNode, &OrderQty);
    NuFixParserSetToDouble(pPriceNode, &Price);
    NuFixParserForEach(pParser, Msg, NULL);

    NuFixParserRemoveCallback(pMsgTypeNode);
    NuFixParserRemoveCallback(pSymbolNode);
    NuFixParserRemoveCallback(pOrderQtyNode);
    NuFixParserRemoveCallback(pPriceNode);

    if (!strcmp(NuStrGet(pMsgType), "F"))
    {
        OrderQty = -OrderQty;
    }

    Item->isExec = true;
    Item->isFail = false;
    Item->execQty = *(Item->reminQty);
    printf("[%s] isFail(%d) execQty(%ld) reminQty(%ld)\n", __func__, Item->isFail, Item->execQty, *(Item->reminQty));

    if (pSymbol)
    {
        NuStrFree(pSymbol);
    }
    if (pMsgType)
    {
        NuStrFree(pMsgType);
    }
}

/************************************************/
/* Prepare for registering CB funcs             */
/************************************************/

static void _LoadCbByMsg(YTRiskMgr_t *pRiskMgr, const char *Msg)
{
    int i = 0;
    long OrderQty = 0;
    NuFixParser_t *pParser = pRiskMgr->pParser;
    NuCBMgr_t *pExecCbMgr = pRiskMgr->pExecCbMgr;
    NuCBMgr_t *pRbCbMgr = pRiskMgr->pRbCbMgr;
    NuFixParserNode_t *pMsgTypeNode = NuFixParserGetNode(pParser, 35);
    NuFixParserNode_t *pSideNode = NuFixParserGetNode(pParser, 54);
    NuFixParserNode_t *pOrderQtyNode = NuFixParserGetNode(pParser, 38);
    NuCBMgrHdlr_t *pLockExecHdlr = NULL;
    NuCBMgrHdlr_t *pLockRbHdlr = NULL;
    NuStr_t *pMsgType = NULL;
    NuStr_t *pSide = NULL;
    YTRiskItemHdr_t Item[3] = {{0}, {0}, {0}};

    printf("[%s] get msg(%s)\n", __func__, Msg);
    for (; i < LAST_IDX; i++)
    {
        Item[i].pParser = pParser;
        Item[i].reminQty = &OrderQty;
    }

    NuCBMgrReset(pExecCbMgr);
    NuCBMgrReset(pRbCbMgr);
    
    NuStrNewPreAlloc(&pMsgType, 12);
    NuStrNewPreAlloc(&pSide, 1);
    NuFixParserSetToStr(pMsgTypeNode, pMsgType);
    NuFixParserSetToStr(pSideNode, pSide);
    NuFixParserSetToLong(pOrderQtyNode, &OrderQty);

    NuFixParserForEach(pParser, Msg, NULL);

    NuFixParserRemoveCallback(pMsgTypeNode);
    NuFixParserRemoveCallback(pSideNode);
    NuFixParserRemoveCallback(pOrderQtyNode);
    
    if (!strcmp(NuStrGet(pMsgType), "F"))
    {
        OrderQty = -OrderQty;
    }
    pLockExecHdlr = NuCBMgrRegisterEvent(pExecCbMgr, _LockPositionExec, (void *)&(Item[LOCK_IDX]));
    NuCBMgrRegisterEvent(pExecCbMgr, _WarnStockExec, (void *)&(Item[WARM_IDX]));
    NuCBMgrRegisterEvent(pExecCbMgr, _AmountExec, (void *)&(Item[AMT_IDX]));

    NuCBMgrRegisterEvent(pRbCbMgr, _AmountRB, (void *)&(Item[AMT_IDX]));
    NuCBMgrRegisterEvent(pRbCbMgr, _WarnStockRB, (void *)&(Item[WARM_IDX]));
    pLockRbHdlr = NuCBMgrRegisterEvent(pRbCbMgr, _LockPositionRB, (void *)&(Item[LOCK_IDX]));
    if (!strcmp(NuStrGet(pSide), "1"))
    {
        NuCBMgrUnRegisterEvent(pLockExecHdlr);
        NuCBMgrUnRegisterEvent(pLockRbHdlr);
    }

    NuCBMgrRaiseEvent(pExecCbMgr, (const void *)Msg);

    for (i = 0 ; i < LAST_IDX; i++)
    {
        if (Item[i].isFail)
        {
            NuCBMgrRaiseEvent(pRbCbMgr, NULL);
            break;
        }
    }

    if (pMsgType)
    {
        NuStrFree(pMsgType);
    }
    if (pSide)
    {
        NuStrFree(pSide);
    }
}


int main(int argc, char **argv)
{
    int i = 0;
    YTRiskMgr_t Mgr = {0};

    Mgr.pExecCbMgr = NuCBMgrAdd();
    Mgr.pRbCbMgr = NuCBMgrAdd();
    NuFixParserNew(&(Mgr.pParser));

    srand(time(NULL));
    for(; i < MSG_LEN; i++)
    {
        _LoadCbByMsg(&Mgr, FIX_MSG[i]);
    }

    NuCBMgrDel(Mgr.pExecCbMgr);
    NuCBMgrDel(Mgr.pRbCbMgr);
    NuFixParserFree(Mgr.pParser);
    return EXIT_SUCCESS;
}
