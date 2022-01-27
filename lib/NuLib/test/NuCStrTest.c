#include <assert.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <NuCStr.h>

#define errChk(rc) \
do \
{ \
	if (!(rc)) { printf("Check Fail(%d): in line(%d)\n", rc, __LINE__); } \
} while(0)

static void _CStrSplitPrint(unsigned int idx, const char *Msg, size_t MsgSize, void *Argu)
{
	printf("[NuCStr]<%d>Split(%.*s)(%ld)\n", idx, (int)MsgSize, Msg, MsgSize);
}

static void _TestToInt(char *Buf, int Precision)
{
	int Tmp1 = 0, Tmp2 = 0;

	Tmp2 = atoi(Buf);
	Tmp1 = NuCStrToInt(Buf, strlen(Buf));
	if (Precision >= 0)
	{
		assert(abs(Tmp1 - Tmp2) <= Precision);
	}
	else
	{
		printf("[NuCStr] NuCStrToInt unhandled behavior(%s)->(%d)\n", Buf, Tmp1);
	}
}

static void _TestToLong(char *Buf, int Precision)
{
	long Tmp1 = 0l, Tmp2 = 0l;

	Tmp2 = atol(Buf);
	Tmp1 = NuCStrToLong(Buf, strlen(Buf));
	if (Precision >= 0)
	{
		assert(abs(Tmp1 - Tmp2) <= Precision);
	}
	else
	{
		printf("[NuCStr] NuCStrToLong unhandled behavior(%s)->(%ld)\n", Buf, Tmp1);
	}

}

static void _TestToFloat(char *Buf, float Precision)
{
	float Tmp1 = 0.0f, Tmp2 = 0.0f;

	Tmp2 = (float)atof(Buf);
	Tmp1 = NuCStrToFloat(Buf, strlen(Buf));
	if (Precision >= 0)
	{
		assert(fabs(Tmp1 - Tmp2) <= Precision);
	}
	else
	{
		printf("[NuCStr] NuCStrToFloat unhandled behavior(%s)->(%f)\n", Buf, Tmp1);
	}
}

static void _TestToDouble(char *Buf, double Precision)
{
	double Tmp1 = 0.0, Tmp2 = 0.0;

	Tmp2 = atof(Buf);
	Tmp1 = NuCStrToDouble(Buf, strlen(Buf));
	if (Precision >= 0)
	{
		assert(fabs(Tmp1 - Tmp2) <= Precision);
	}
	else
	{
		printf("[NuCStr] NuCStrToFloat unhandled behavior(%s)->(%lf)\n", Buf, Tmp1);
	}
}

void _TestIsNumeric(const char *Buf, bool QuessRes)
{
	bool IsNumeric = false;

	IsNumeric = NuCStrIsNumeric(Buf);
	assert(IsNumeric == QuessRes);

}

void _TestCharToBinStr(char *Buf)
{
	printf("8=%s\n", NuCharToBinString(8, Buf));
}

int main(int argc, char **argv)
{
	char CSep = ' ';
	char Buf1[80] = {0};
	char Buf2[32] = {0};

	strncpy(Buf1, "1234567\001890123456\001789012345\00167890", sizeof(Buf1));
	printf("[NuCStr]===== NuCStrReplaceChr =====\n");
	NuCStrReplaceChr(Buf1, '\001', ' ');
	assert(strcmp(Buf1, "1234567 890123456 789012345 67890") == 0);
	printf("[NuCStr]===== NuCStrReplaceRangeChr =====\n");
	NuCStrReplaceRangeChr(Buf1, ' ', '\001', sizeof(Buf1));
	assert(strcmp(Buf1, "1234567\001890123456\001789012345\00167890") == 0);
	NuCStrReplaceRangeChr(Buf1, '\001', ' ', 20);
	assert(strcmp(Buf1, "1234567 890123456 789012345\00167890") == 0);

	printf("[NuCStr]===== NuCStrSplit =====\n");
	printf("[NuCStr]Buf(%.*s)Sep(%c)\n", (int)strlen(Buf1), Buf1, CSep);
	NuCStrSplit(Buf1, CSep, strlen(Buf1), _CStrSplitPrint, NULL);

	printf("[NuCStr]Buf(%.*s)Sep(%c)\n", 20, Buf1, CSep);
	NuCStrSplit(Buf1, CSep, 20, _CStrSplitPrint, NULL);
	printf("[NuCStr]Buf(%.*s)Sep(%c)\n", 18, Buf1, CSep);
	NuCStrSplit(Buf1, CSep, 18, _CStrSplitPrint, NULL);
	printf("[NuCStr]Buf(%.*s)Sep(%c)\n", 5, Buf1 + 20, CSep);
	NuCStrSplit(Buf1 + 20, CSep, 5, _CStrSplitPrint, NULL);
	printf("[NuCStr]Buf(%.*s)Sep(%c)\n", 5, Buf1 + 10, CSep);
	NuCStrSplit(Buf1 + 10, CSep, 5, _CStrSplitPrint, NULL);

	strncpy(Buf1, "           12345    234567    1234567    ", sizeof(Buf1));
	printf("[NuCStr]===== NuCStrRTrimChr =====\n");
	NuCStrRTrimChr(Buf1, ' ');
	assert(strcmp(Buf1, "           12345    234567    1234567") == 0);
	printf("[NuCStr]===== NuCStrLTrimChr =====\n");
	NuCStrLTrimChr(Buf1, ' ');
	assert(strcmp(Buf1, "12345    234567    1234567") == 0);

	printf("[NuCStr]===== NuCStrSubStr =====\n");
	NuCStrSubStr(Buf2, Buf1, 7);
	assert(strcmp(Buf2, "12345  ") == 0);

	printf("[NuCStr]===== NuCStrPrintInt =====\n");
	NuCStrPrintInt(Buf2, 2147483647, sizeof(Buf2) - 1);
	assert(strcmp(Buf2, "0000000000000000000002147483647") == 0);
	NuCStrPrintInt(Buf2, -2147483648, sizeof(Buf2) - 1);
	assert(strcmp(Buf2, "-000000000000000000002147483648") == 0);
	NuCStrPrintInt(Buf2, 0, sizeof(Buf2) - 1);
	assert(strcmp(Buf2, "0000000000000000000000000000000") == 0);

	printf("[NuCStr]===== NuCStrPrintLong =====\n");
	NuCStrPrintLong(Buf2, -9223372036854775808UL, sizeof(Buf2) - 1);
	assert(strcmp(Buf2, "-000000000009223372036854775808") == 0);
	NuCStrPrintLong(Buf2, 9223372036854775807UL, sizeof(Buf2) - 1);
	assert(strcmp(Buf2, "0000000000009223372036854775807") == 0);

	printf("[NuCStr]===== NuCStrIsNumeric =====\n");
	_TestIsNumeric("123456", true);
	_TestIsNumeric("-123456", true);
	_TestIsNumeric("123-456", false);
	_TestIsNumeric("123456-", false);
	_TestIsNumeric("000000000", true);
	_TestIsNumeric("123.456", true);
	_TestIsNumeric("123456.", true);
	_TestIsNumeric(".123456", true);
	_TestIsNumeric("1.23.456", false);
	_TestIsNumeric("1.23f456", false);
	_TestIsNumeric("aaaaaaaa", false);
	_TestIsNumeric("123,456", true);
	_TestIsNumeric("-123,456", true);
	_TestIsNumeric(".123,456", true);
	_TestIsNumeric("23,456,123.456,452", true);
	_TestIsNumeric("-23,456,123.456,452", true);
	_TestIsNumeric("-23,456f,123.456,452", false);

	printf("[NuCStr]===== NuCStrToInt =====\n");
	_TestToInt("2147483647", 0);
	_TestToInt("2147483648", -1);
	_TestToInt("-2147483648", 0);
	_TestToInt("-2147483649", -1);
	_TestToInt("-21474.83648", -1);
	_TestToInt("0", 0);
	_TestToInt("-0", 0);
	_TestToInt(" ", -1);
	_TestToInt("-", -1);
	_TestToInt("2,147,483,647", -1);
	_TestToInt("-2,147,483,648", -1);

	printf("[NuCStr]===== NuCStrToLong =====\n");
	_TestToLong("9223372036854775807", 0);
	_TestToLong("9223372036854775808", -1);
	_TestToLong("-9223372036854775808", 0);
	_TestToLong("-9223372036854775809", -1);
	_TestToLong("-21474.83648", -1);
	_TestToLong("0", 0);
	_TestToLong("-0", 0);
	_TestToLong("-", -1);
	_TestToLong(" ", -1);
	_TestToLong("9,223,372,036,854,775,807", -1);
	_TestToLong("-9,223,372,036,854,775,808", -1);

	printf("[NuCStr]===== NuCStrToFloat =====\n");
	_TestToFloat("8388608", 0);
	_TestToFloat("-8388608", 0);
	_TestToFloat("8388.608", 0.001);
	_TestToFloat("-8388.608", 0.001);
	_TestToFloat(".838608", 0.000001);
	_TestToFloat("-.567608", 0.000001);
	_TestToFloat(".838.608", -1);
	_TestToFloat("-.5676.08", -1);
	_TestToFloat("0", 0);
	_TestToFloat("-0", 0);
	_TestToFloat("-", 0);
	_TestToFloat(" ", -1);
	_TestToFloat("8,388,608", -1);
	_TestToFloat("-8,388,608", -1);
	_TestToFloat("8,388.608,45", -1);
	_TestToFloat("-8,388.608,45", -1);
	_TestToFloat(".838,608", -1);
	_TestToFloat("-.567,608", -1);

	printf("[NuCStr]===== NuCStrToDouble =====\n");
	_TestToDouble("4503599627370495", 0);
	_TestToDouble("-4503599627370495", 0);
	_TestToDouble("-21474.83648", 0.00001);
	_TestToDouble("21474.83648", 0.00001);
	_TestToDouble("-.83648", 0.00001);
	_TestToDouble(".83648", 0.00001);
	_TestToDouble("-.836.48", -1);
	_TestToDouble(".8364.8", -1);
	_TestToDouble("0", 0);
	_TestToDouble("-0", 0);
	_TestToDouble("-", 0);
	_TestToDouble(" ", -1);
	_TestToDouble("4,503,599,627,370,495", -1);
	_TestToDouble("-4,503,599,627,370,495", -1);
	_TestToDouble("-21,474.836,48", -1);
	_TestToDouble("21,474.836,48", -1);
	_TestToDouble("-.836,48", -1);
	_TestToDouble(".836,48", -1);
	_TestToDouble("-.836,880.48", -1);
	_TestToDouble(".836,4.8", -1);

    _TestCharToBinStr(Buf1);
	return EXIT_SUCCESS;
}
