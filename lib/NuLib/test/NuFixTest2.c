#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <NuCStr.h>
#include <NuFix.h>
#include <NuStr.h>

static char *FIX_MSG = "8=FIX.4.29=029135=D49=BBGFUT_P56=YUTAFO34=21152=20161018-01:45:4250=935628760=20161018-01:45:421=UOMMIRA63=064=2016101838=315048=sub-acct40=1100=FTX11=20161018-00001-0000115=TWD167=FUT48=FTV6FTX6200=20161021=322=A54=155=TXFJ6/K6205=1958=TWAP FROM 13:15~13:25 Taipei local time59=010=046";

bool _NuFixParserWork(int Tag, const char *Value, size_t ValueLen, void *Argu, void *Closure)
{
	NuStr_t *str = (NuStr_t *)Closure;

	NuStrAppendPrintf(str, "%d=%.*s\001", Tag, ValueLen, Value);
	return true;
}

bool _NuFixParserWork2(int Tag, const char *Value, size_t ValueLen, void *Argu, void *Closure)
{

	return true;
}

int main(int argc, char **argv)
{
	NuStr_t *str = NULL;
	NuFixParser_t *parser = NULL;

	NuStrNewPreAlloc(&str, 512);

	NuFixParserNew(&parser);
	/* ------------------------------------ */

    NuFixParserSetCallback(NuFixParserGetNode(parser, 8), &_NuFixParserWork2, NULL);
    NuFixParserSetCallback(NuFixParserGetNode(parser, 9), &_NuFixParserWork2, NULL);
    NuFixParserSetCallback(NuFixParserGetNode(parser, 35), &_NuFixParserWork2, NULL);
    NuFixParserSetCallback(NuFixParserGetNode(parser, 49), &_NuFixParserWork2, NULL);
    NuFixParserSetCallback(NuFixParserGetNode(parser, 56), &_NuFixParserWork2, NULL);
    NuFixParserSetCallback(NuFixParserGetNode(parser, 52), &_NuFixParserWork2, NULL);
    NuFixParserSetCallback(NuFixParserGetNode(parser, 34), &_NuFixParserWork2, NULL);
    NuFixParserSetCallback(NuFixParserGetNode(parser, 10), &_NuFixParserWork2, NULL);

	NuFixParserForEachByCB(parser, FIX_MSG, strlen(FIX_MSG), &_NuFixParserWork, (void *)str);

	printf("[%s]\n", NuStrGet(str));
	/* ------------------------------------ */
	NuFixParserFree(parser);

	NuStrFree(str);

    return 0;
}
