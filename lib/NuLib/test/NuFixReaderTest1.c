#include "NuCommon.h"
#include "NuStr.h"
#include "NuTuneTools.h"
#include "NuFixReader.h"

#define NONE "\033[m"
#define RED "\033[0;32;31m"
#define LIGHT_RED "\033[1;31m"
#define GREEN "\033[0;32;32m"
#define LIGHT_GREEN "\033[1;32m"
#define BLUE "\033[0;32;34m"
#define LIGHT_BLUE "\033[1;34m"
#define DARY_GRAY "\033[1;30m"
#define CYAN "\033[0;36m"
#define LIGHT_CYAN "\033[1;36m"
#define PURPLE "\033[0;35m"
#define LIGHT_PURPLE "\033[1;35m"
#define BROWN "\033[0;33m"
#define YELLOW "\033[1;33m"
#define LIGHT_GRAY "\033[0;37m"
#define WHITE "\033[1;37m"

#define _LOG(fmt, ...)    printf(WHITE"[OK] "fmt""NONE, ##__VA_ARGS__)
#define _ERR(fmt, ...)    printf(LIGHT_RED"[FAIL] "fmt""NONE, ##__VA_ARGS__)

typedef struct _argu_t
{
	char *ptr;
	char *end;
} argu_t;

static bool _TagIgnoreCB(const char *Tag, size_t TagLen, const char *Value, size_t ValueLen, void *Argu, void *Closure)
{
	return true;
}

static bool _TagRetentionCB(const char *Tag, size_t TagLen, const char *Value, size_t ValueLen, void *Argu, void *Closure)
{
	NuStr_t *str = (NuStr_t *)Closure;
//printf("%.*s\n", (int)TagLen, Tag);
	NuStrAppendPrintf(str, "%.*s=%.*s\001", TagLen, Tag, ValueLen, Value);

	return true;
}

static bool _TagPtrCB(const char *Tag, size_t TagLen, const char *Value, size_t ValueLen, void *Argu, void *Closure)
{
	argu_t *argu = (argu_t *)Closure;

	if (argu->end - argu->ptr < (2 + TagLen + ValueLen))
	{
		return false;
	}
	argu->ptr += sprintf(argu->ptr, "%.*s=%.*s\001", (int)TagLen, Tag, (int)ValueLen, Value);

	return true;
}


static void _TestReader1(char *fixmsg)
{
	int iRC = 0;
	NuFixReader_t *reader = NULL;
	NuStr_t *str = NULL;
	int len = strlen(fixmsg);

	iRC = NuStrNewPreAlloc(&str, 1024);
	NUCHKRC(iRC, EXIT);

	iRC = NuFixReaderNew(&reader);
	NUCHKRC(iRC, EXIT);

	NuFixReaderSetCallback( NuFixReaderGetNode(reader, "453", 3), &_TagIgnoreCB, NULL);
	NuFixReaderSetCallback( NuFixReaderGetNode(reader, "447", 3), &_TagIgnoreCB, NULL);
	NuFixReaderSetCallback( NuFixReaderGetNode(reader, "448", 3), &_TagIgnoreCB, NULL);
	NuFixReaderSetCallback( NuFixReaderGetNode(reader, "452", 3), &_TagIgnoreCB, NULL);

	NuFixReaderForEachWithCB(reader, fixmsg, len, &_TagRetentionCB, str);
	_LOG("[%s]\n", NuStrGet(str));

EXIT:
	
	if (reader != NULL)
	{
		NuFixReaderFree(reader);
	}

	if (str != NULL)
	{
		NuStrFree(str);
	}
}

static void _TestReaderLatency1(char *fixmsg)
{
	int iRC = 0;
	NuFixReader_t *reader = NULL;
	NuStr_t *str = NULL;
	int len = strlen(fixmsg);
	int i = 0;

	iRC = NuStrNewPreAlloc(&str, 1024);
	NUCHKRC(iRC, EXIT);

	iRC = NuFixReaderNew(&reader);
	NUCHKRC(iRC, EXIT);

	NuFixReaderSetCallback( NuFixReaderGetNode(reader, "453", 3), &_TagIgnoreCB, NULL);
	NuFixReaderSetCallback( NuFixReaderGetNode(reader, "447", 3), &_TagIgnoreCB, NULL);
	NuFixReaderSetCallback( NuFixReaderGetNode(reader, "448", 3), &_TagIgnoreCB, NULL);
	NuFixReaderSetCallback( NuFixReaderGetNode(reader, "452", 3), &_TagIgnoreCB, NULL);

	NuTuneSetStart();

	for (i = 0; i < 1000000; i++)
	{
		NuStrClear(str);
		NuFixReaderForEachWithCB(reader, fixmsg, len, &_TagRetentionCB, str);
		//NuFixReaderForEach(reader, fixmsg, len, str);
	}

	NuTuneSetStop();

	_LOG("Latency : %ld sec %ld ns\n", NuTuneGetSecElapse(), NuTuneGetNanoSecElapse());
	_LOG("Latency : %ld ns\n", (NuTuneGetSecElapse() * 1000000000 + NuTuneGetNanoSecElapse()) / 1000000);

	//_LOG("[%s]\n", NuStrGet(str));

EXIT:
	
	if (reader != NULL)
	{
		NuFixReaderFree(reader);
	}

	if (str != NULL)
	{
		NuStrFree(str);
	}
}

static void _TestReaderLatency2(char *fixmsg)
{
	int iRC = 0;
	NuFixReader_t *reader = NULL;
	NuStr_t *str = NULL;
	int len = strlen(fixmsg);
	int i = 0;
	char *buf = NULL;
	argu_t argu;

	buf = (char *)malloc(sizeof(char) * 2048);

	argu.ptr = buf;
	argu.end = buf + 2048;

	iRC = NuStrNewPreAlloc(&str, 1024);
	NUCHKRC(iRC, EXIT);

	iRC = NuFixReaderNew(&reader);
	NUCHKRC(iRC, EXIT);

	NuFixReaderSetCallback( NuFixReaderGetNode(reader, "453", 3), &_TagIgnoreCB, NULL);
	NuFixReaderSetCallback( NuFixReaderGetNode(reader, "447", 3), &_TagIgnoreCB, NULL);
	NuFixReaderSetCallback( NuFixReaderGetNode(reader, "448", 3), &_TagIgnoreCB, NULL);
	NuFixReaderSetCallback( NuFixReaderGetNode(reader, "452", 3), &_TagIgnoreCB, NULL);

	NuTuneSetStart();

	for (i = 0; i < 1000000; i++)
	{
		argu.ptr = buf;
		buf[0] = '\0';
		NuFixReaderForEachWithCB(reader, fixmsg, len, &_TagPtrCB, &argu);
	}

	NuTuneSetStop();

	_LOG("Latency : %ld sec %ld ns\n", NuTuneGetSecElapse(), NuTuneGetNanoSecElapse());
	_LOG("Latency : %ld ns\n", (NuTuneGetSecElapse() * 1000000000 + NuTuneGetNanoSecElapse()) / 1000000);

EXIT:
	
	if (reader != NULL)
	{
		NuFixReaderFree(reader);
	}

	if (str != NULL)
	{
		NuStrFree(str);
	}
}

int main()
{
    char szFixMsg[512] = "8=FIX.4.39=17535=D49=IVAN156=YUTAFOTEST34=8552=20111230-01:26:4011=20111230:05453=3448=9921285447=D452=2448=123447=2452=521=118=055=TXFA222=10054=138=40=244=700058=cc=Discretion|ac=629012|lc=TPE60=20111230-01:26:4059=020082=IVAN210=235";
	//printf("[%s]\n", szFixMsg);	

	_TestReader1(szFixMsg);
    
	_TestReaderLatency1(szFixMsg);
	_TestReaderLatency2(szFixMsg);

    return 0;
}
