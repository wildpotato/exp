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

    NuFixMsgTemplateAddRepeatingGroup(fix, NuFixMsgKindBody, 453, 3, 448, 5, 447, 5, 452, 5);
}

#define _ShowTag(FIX, Tag) \
do { \
	printf("%d[%s]\n", (Tag),(NuFixMsgGetVal(NuFixMsgFieldGet((FIX), (Tag))) == NULL ) ? "NULL" : \
			                                     NuFixMsgGetVal(NuFixMsgFieldGet((FIX), (Tag)))); \
}while(0)

#define _ShowTag_Grp(FIX, Tag, Idx) \
do { \
	printf("%d-%d[%s]\n", (Tag), (Idx),( NuFixMsgGetGroupStr((FIX), (Tag), (Idx)) == NULL ) ? "NULL" : \
			                    NuFixMsgGetGroupStr((FIX), (Tag), (Idx))); \
}while(0)

int main()
{
    char szMsg[512] = "8=FIX.4.39=17535=D49=IVAN156=YUTAFOTEST34=8552=20111230-01:26:4011=20111230:05453=3448=9921285447=D452=2448=123447=2452=521=118=055=TXFA222=10054=138=40=244=700058=cc=Discretion|ac=629012|lc=TPE60=20111230-01:26:4059=020082=IVAN210=235";

    NuFixMsg_t *pFixMsg = NULL;

    NuFixMsgNew(&pFixMsg);

	InitFixMsg(pFixMsg);


printf("=================================================\n");
printf("origin msg[%s]\n", szMsg);
printf("-------------------------------------------------\n");
NuFixMsgParse(pFixMsg, szMsg);

NuFixMsgGenHB(pFixMsg);
printf("HB    msg[%s]\n", NuFixMsgTakeOutMsg(pFixMsg));
NuFixMsgGenMsg(pFixMsg);
printf("      msg[%s]\n", NuFixMsgTakeOutMsg(pFixMsg));
printf("=================================================\n");

	_ShowTag_Grp(pFixMsg, 452, 0);
	_ShowTag_Grp(pFixMsg, 452, 1);

	NuFixMsgGroupRemove(pFixMsg, 447, 0);
	NuFixMsgInstanceRemove(pFixMsg, 453, 1);
NuFixMsgGenMsg(pFixMsg);
printf("      msg[%s]\n", NuFixMsgTakeOutMsg(pFixMsg));

    NuFixMsgFree(pFixMsg);
    
    return 0;
}
