#include "sys/time.h"
#include "NuFixMsg.h"


void InitFixMsg(NuFixMsg_t *fix)
{
    NuFixMsgTemplateAddHeader(fix, 35, 2);
    NuFixMsgTemplateAddHeader(fix, 34, 10);
    NuFixMsgTemplateAddHeader(fix, 49, 10);
    NuFixMsgTemplateAddHeader(fix, 52, 10);
    NuFixMsgTemplateAddHeader(fix, 56, 20);
    NuFixMsgTemplateAddHeader(fix, 115, 20);
    NuFixMsgTemplateAddHeader(fix, 116, 20);
    
    NuFixMsgTemplateAddBody(fix, 1, 7);
    NuFixMsgTemplateAddBody(fix, 6, 10);
    NuFixMsgTemplateAddBody(fix, 11, 10);
    NuFixMsgTemplateAddBody(fix, 14, 10);
    NuFixMsgTemplateAddBody(fix, 17, 10);
    NuFixMsgTemplateAddBody(fix, 18, 10);
    NuFixMsgTemplateAddBody(fix, 20, 10);
    NuFixMsgTemplateAddBody(fix, 22, 10);
    NuFixMsgTemplateAddBody(fix, 31, 10);
    NuFixMsgTemplateAddBody(fix, 32, 10);
    NuFixMsgTemplateAddBody(fix, 37, 10);
    NuFixMsgTemplateAddBody(fix, 38, 10);
    NuFixMsgTemplateAddBody(fix, 39, 10);
    NuFixMsgTemplateAddBody(fix, 40, 10);
    NuFixMsgTemplateAddBody(fix, 41, 10);
    NuFixMsgTemplateAddBody(fix, 44, 10);
    NuFixMsgTemplateAddBody(fix, 54, 10);
    NuFixMsgTemplateAddBody(fix, 55, 10);
    NuFixMsgTemplateAddBody(fix, 58, 64);
    NuFixMsgTemplateAddBody(fix, 59, 10);
    NuFixMsgTemplateAddBody(fix, 60, 10);
    NuFixMsgTemplateAddBody(fix, 150, 2);
    NuFixMsgTemplateAddBody(fix, 151, 2);
    NuFixMsgTemplateAddBody(fix, 167, 3);
    NuFixMsgTemplateAddBody(fix, 198, 5);
    NuFixMsgTemplateAddBody(fix, 200, 5);
    NuFixMsgTemplateAddBody(fix, 20082, 16);
}

#define _ShowTag(FIX, Tag) \
do { \
	printf("%d[%s]\n", (Tag),(NuFixMsgGetVal(NuFixMsgFieldGet((FIX), (Tag))) == NULL ) ? "NULL" : \
			                                     NuFixMsgGetVal(NuFixMsgFieldGet((FIX), (Tag)))); \
}while(0)

int main()
{
    char szMsg[512] = "8=FIX.4.29=27335=849=YUTAFO56=BARCJPFO_234=32052=20111103-00:54:2837=34000937198=5A84211=BFN3DRD420I:1=101=992023117=E202020=0150=239=255=MXF167=FUT200=20111154=238=440=244=755859=032=431=0007558.00151=014=46=7558.00060=20111103-00:54:2858=101012-Gateway:Filled.10=165";

    NuFixMsg_t *pFixMsg = NULL;


    NuFixMsgNew(&pFixMsg);

	InitFixMsg(pFixMsg);

printf("=================================================\n");
printf("origin msg[%s]\n", szMsg);
printf("-------------------------------------------------\n");
NuFixMsgParse(pFixMsg, szMsg);

NuFixMsgGenHB(pFixMsg);
printf("HB    msg[%s]\n", NuFixMsgTakeOutMsg(pFixMsg));
NuFixMsgFieldRemove(pFixMsg, 49);
NuFixMsgGenMsg(pFixMsg);
printf("      msg[%s]\n", NuFixMsgTakeOutMsg(pFixMsg));
printf("=================================================\n");

	_ShowTag(pFixMsg, 49);
	_ShowTag(pFixMsg, 34);
	_ShowTag(pFixMsg, 35);
	_ShowTag(pFixMsg, 9);

NuFixMsgClear(pFixMsg);
	NuFixMsgFieldPrintf(pFixMsg, 58, "test %d", 10);
	NuFixMsgFieldPrintf(pFixMsg, 34, "%d", 999);
	NuFixMsgFieldAssign(pFixMsg, 55, "2330", 4);

NuFixMsgGenMsg(pFixMsg);
printf("      msg[%s]\n", NuFixMsgTakeOutMsg(pFixMsg));

    NuFixMsgFree(pFixMsg);
    
    return 0;
}
