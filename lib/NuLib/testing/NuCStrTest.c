
#include "NuCStr.h"


static void _SplitCBFn(unsigned int idx, const char *Msg, size_t MsgSize, void *Argu)
{
	switch(idx)
	{
		case 0:
			assert_memory_equal(Msg, "8=FIX.4.3", MsgSize);
			break;
		case 1:
			assert_memory_equal(Msg, "9=175", MsgSize);
			break;
		case 2:
			assert_memory_equal(Msg, "35=D", MsgSize);
			break;
		case 3:
			assert_memory_equal(Msg, "49=IVAN1", MsgSize);
			break;
		case 4:
			assert_memory_equal(Msg, "56=YUTAFOTEST", MsgSize);
			break;
		case 5:
			assert_memory_equal(Msg, "34=85", MsgSize);
			break;
		case 6:
			assert_memory_equal(Msg, "52=20111230-01:26:40", MsgSize);
			break;
		case 7:
			assert_int_equal(MsgSize, 0);
			break;
		default:
			break;
	}
}

static void _TestSplitFn()
{
	char *data = "8=FIX.4.39=17535=D49=IVAN156=YUTAFOTEST34=8552=20111230-01:26:40";
	NuCStrSplit(data, '\001', strlen(data), &_SplitCBFn, NULL);
}

void nucstr_test(void **state) 
{
	int    iRC = 0;
	char   Data[128];
	char   Sub[32];
	char   Num[32];
	int    iNum = 0;
	long   lNum = 0;
	float  fNum = 0;
	double dNum = 0;

	/* replace                                  */
	strcpy(Data, "this is a test program for nulib string functions.");
	NuCStrReplaceRangeChr(Data, ' ', '@', 30);
	assert_string_equal(Data, "this@is@a@test@program@for@nulib string functions.");

	NuCStrReplaceChr(Data, '@', ' ');
	assert_string_equal(Data, "this is a test program for nulib string functions.");
	/* ---------------------------------------- */

	/* trim                                     */
	strcpy(Data, "  Data trim test A   ");
	NuCStrRTrimChr(Data, ' ');
	assert_string_equal(Data, "  Data trim test A");

	NuCStrLTrimChr(Data, ' ');
	assert_string_equal(Data, "Data trim test A");

	NuCStrSubStr(Sub, Data, 4);
	assert_string_equal(Sub, "Data");
	/* ---------------------------------------- */

	/* numeric                                  */
    iRC = NuCStrPrintInt(Num, 213469234, 10);
	assert_string_equal(Num, "0213469234");

    iRC = NuCStrPrintInt(Num, 21346.0000, 10);
	assert_string_equal(Num, "0000021346");

    iRC = NuCStrPrintInt(Num, 21346.9234, 10);
	assert_int_not_equal(iRC, 0);

    iRC = NuCStrPrintLong(Num, 213469234000, 15);
	assert_string_equal(Num, "000213469234000");

    iRC = NuCStrPrintLong(Num, 213469234.000, 15);
	assert_string_equal(Num, "000000213469234");

    iRC = NuCStrPrintLong(Num, 213469234.99, 15);
	assert_int_not_equal(iRC, 0);

	strcpy(Data, "213469234");
	iNum = NuCStrToInt(Data, strlen(Data));
	assert_int_equal(iNum, 213469234);


	strcpy(Data, "213469234000");
	lNum = NuCStrToLong(Data, strlen(Data));
	assert_int_equal(lNum, 213469234000);

	strcpy(Data, "3469.234");
    fNum =  NuCStrToFloat(Data, strlen(Data));
	assert_int_equal(fNum * 1000, 3469.234 * 1000);

	strcpy(Data, "213469.234");
	dNum = NuCStrToDouble(Data, strlen(Data));
	assert_int_equal(dNum * 1000, 213469.234 * 1000);

	assert_true(NuCStrIsNumeric("123456.12"));
	assert_false(NuCStrIsNumeric("123456.12.1"));
	/* ---------------------------------------- */

	_TestSplitFn();

}


