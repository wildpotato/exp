#include "NuStr.h"

static int _nu_str_vprintf(NuStr_t *str, const char *fmt, ...)
{
	int iRC = 0;
    va_list argu_list;

    va_start(argu_list, fmt);
	iRC = NuStrVPrintf(str, 0, fmt, argu_list);
    va_end(argu_list);

	return iRC;
}

void nustr_test(void **state) 
{
	char *data = "NUSTR_TEST";
	char *TESTDATA[4] = {  "CAT",
		                   "DOG",
						   "CATDOG",
						   "CAT/DOG/CATDOG/"
		                };
	int iRC = 0;
	NuStr_t *str = NULL;

	/* new alloc                        */
	iRC = NuStrNewPreAlloc(&str, 32);
	assert_int_equal(iRC, 0);

	NuStrFree(str);
	str = NULL;
	assert_true(str == NULL);
	/* -------------------------------- */

	/* new alloc with string            */
	iRC = NuStrNew(&str, data);
	assert_int_equal(iRC, 0);
	assert_string_equal(NuStrGet(str), data);
	assert_int_equal(NuStrSize(str), strlen(data));
	/* --------------------------------------------------- */

	/* clear */
	NuStrClear(str);
	assert_string_equal(NuStrGet(str), "");
	assert_int_equal(NuStrSize(str), 0);
	/* --------------------------------------------------- */

	/* cat */
	NuStrCat(str, TESTDATA[0]);
	assert_string_equal(NuStrGet(str), TESTDATA[0]);
	assert_int_equal(NuStrSize(str), strlen(TESTDATA[0]));

	NuStrCat(str, TESTDATA[1]);
	assert_string_equal(NuStrGet(str), TESTDATA[2]);
	assert_int_equal(NuStrSize(str), strlen(TESTDATA[2]));

	NuStrClear(str);
	NuStrNCat(str, TESTDATA[3], 3);
	assert_string_equal(NuStrGet(str), TESTDATA[0]);
	assert_int_equal(NuStrSize(str), strlen(TESTDATA[0]));

	NuStrCatChr(str, TESTDATA[1][0]);
	NuStrCatChr(str, TESTDATA[1][1]);
	NuStrCatChr(str, TESTDATA[1][2]);
	assert_string_equal(NuStrGet(str), TESTDATA[2]);
	assert_int_equal(NuStrSize(str), strlen(TESTDATA[2]));

	NuStrCpy(str, TESTDATA[3]);
	assert_string_equal(NuStrGet(str), TESTDATA[3]);
	assert_int_equal(NuStrSize(str), strlen(TESTDATA[3]));

	NuStrNCpy(str, TESTDATA[3], 3);
	assert_string_equal(NuStrGet(str), TESTDATA[0]);
	assert_int_equal(NuStrSize(str), strlen(TESTDATA[0]));
	/* --------------------------------------------------- */

	/* printf */
	iRC = NuStrPrintf(str, 0, "%s/libs/%s", "/home/TEST", "src");
	assert_string_equal(NuStrGet(str), "/home/TEST/libs/src");
	assert_int_equal(iRC, strlen("/home/TEST/libs/src"));

	NuStrSetChr(str, 0, '@');
	NuStrSetChr(str, NuStrSize(str) - 1, '@');
	assert_string_equal(NuStrGet(str), "@home/TEST/libs/sr@");

	iRC = _nu_str_vprintf(str, "%s/libs/%s", "/home/TEST", "src");
	assert_string_equal(NuStrGet(str), "/home/TEST/libs/src");
	assert_int_equal(iRC, strlen("/home/TEST/libs/src"));

	NuStrAppendPrintf(str, "/%d", 123);
	assert_string_equal(NuStrGet(str), "/home/TEST/libs/src/123");
	assert_int_equal(NuStrSize(str), strlen("/home/TEST/libs/src/123"));
	/* --------------------------------------------------- */

	NuStrFree(str);
}

void nustr_tools_test(void **state)
{
	int iRC = 0, i = 0;
	NuStr_t *str_1 = NULL;
	NuStr_t *str_2 = NULL;
	char data[255 + 1] = {0};

	/* strcmp                           */
	for(i = 1; i < 256; i++)
	{
		data[i-1] = (char)i;
	}
	data[255] = '\0';

	iRC = NuStrNewPreAlloc(&str_1, 64);
	assert_int_equal(iRC, 0);
	NuStrCpy(str_1, data);

	iRC = NuStrNew(&str_2, data);
	assert_int_equal(iRC, 0);

	iRC = NuStrCmp(str_1, str_2);
	assert_int_equal(iRC, 0);

	NuStrSetChr(str_2, 9, 'A');
	iRC = NuStrCmp(str_1, str_2);

	assert_int_not_equal(iRC, 0);
	/* -------------------------------- */

	/* Trim                             */
	NuStrCpy(str_1, "   TEST123   ");
	NuStrRTrim(str_1);
	assert_string_equal(NuStrGet(str_1), "   TEST123");

	NuStrLTrim(str_1);
	assert_string_equal(NuStrGet(str_1), "TEST123");

	NuStrCpy(str_1, "TEST123");
	NuStrRTrim(str_1);
	assert_string_equal(NuStrGet(str_1), "TEST123");

	NuStrLTrim(str_1);
	assert_string_equal(NuStrGet(str_1), "TEST123");
	/* -------------------------------- */

	/* Replace                          */
	NuStrCpy(str_1, " 123@456@789@ZZZ   ");
	NuStrReplaceRangeChr(str_1, '@', ' ', 0, 10);
	assert_string_equal(NuStrGet(str_1), " 123 456 789@ZZZ   ");

	NuStrReplaceRangeChr(str_1, ' ', '@', 1, 10);
	assert_string_equal(NuStrGet(str_1), " 123@456@789@ZZZ   ");

	NuStrReplaceChr(str_1, '@', ' ');
	assert_string_equal(NuStrGet(str_1), " 123 456 789 ZZZ   ");
	/* -------------------------------- */

	assert_int_equal(NuStrGetChr(str_1, 3), '3');

	NuStrCpy(str_1, "12345");
	assert_int_equal(NuStrGetInt(str_1), 12345);

	NuStrCpy(str_1, "12345.995");
	assert_int_equal(NuStrGetDouble(str_1) * 1000, 12345995);

	NuStrFree(str_1);
	NuStrFree(str_2);
}

