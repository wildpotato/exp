#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "NuCStr.h"
#include "NuStr.h"
#include "NuTuneTools.h"
#include "NuFix.h"

static bool _TagIgnoreCB(int Tag, const char *Value, size_t ValueLen, void *Argu, void *Closure)
{
	return true;
}

static bool _TagRetentionCB(int Tag, const char *Value, size_t ValueLen, void *Argu, void *Closure)
{
	NuStr_t *str = (NuStr_t *)Closure;
	NuStrAppendPrintf(str, "%d=%.*s\001", Tag, ValueLen, Value);

	return true;
}

static void _TestLatency1(char *fixmsg)
{
	NuStr_t *str = NULL;
	NuFixParser_t *parser = NULL;
	int len = strlen(fixmsg);
	int i = 0;

	NuStrNewPreAlloc(&str, 1024);

	NuFixParserNew(&parser);
	/* ------------------------------------ */

    NuFixParserSetCallback(NuFixParserGetNode(parser, 453), &_TagIgnoreCB, NULL);
    NuFixParserSetCallback(NuFixParserGetNode(parser, 447), &_TagIgnoreCB, NULL);
    NuFixParserSetCallback(NuFixParserGetNode(parser, 448), &_TagIgnoreCB, NULL);
    NuFixParserSetCallback(NuFixParserGetNode(parser, 452), &_TagIgnoreCB, NULL);

	NuTuneSetStart();
	for (i = 0; i < 1000000; i++)
	{
		NuStrClear(str);
		NuFixParserForEachByCB(parser, fixmsg, len, &_TagRetentionCB, (void *)str);
	}

	NuTuneSetStop();

	printf("Latency : %ld sec %ld ns\n", NuTuneGetSecElapse(), NuTuneGetNanoSecElapse());
	printf("Latency : %ld ns\n", (NuTuneGetSecElapse() * 1000000000 + NuTuneGetNanoSecElapse()) / 1000000);

	/* ------------------------------------ */
	if (parser != NULL)
	{
		NuFixParserFree(parser);
	}

	if (str != NULL)
	{
		NuStrFree(str);
	}
}

int main(int argc, char **argv)
{
    char szFixMsg[512] = "8=FIX.4.39=17535=D49=IVAN156=YUTAFOTEST34=8552=20111230-01:26:4011=20111230:05453=3448=9921285447=D452=2448=123447=2452=521=118=055=TXFA222=10054=138=40=244=700058=cc=Discretion|ac=629012|lc=TPE60=20111230-01:26:4059=020082=IVAN210=235";

	_TestLatency1(szFixMsg);

    return 0;
}
