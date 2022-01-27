#include <assert.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <NuCStr.h>
/*
int main(int argc, char **argv)
{
	int i = 1234;
	char Buf[128] = {0};
	char *ptr = NULL;

	strcpy(Buf, "8=FIX.4.4\0019=00000\001");
	ptr = Buf + 12;

	NuCStrPrintInt(ptr, i, 5);
	ptr[5] = '#';

	printf("[%s]\n", Buf);

	return 0;
}
*/

void test_IntToStr(int num) 
{
    char buf[16+1];
    char cmp[16+1];

    memset(buf, 0, sizeof(buf));
    memset(cmp, 0, sizeof(cmp));

    sprintf(cmp, "%016d", num);
    NuIntToStrLPadZero(buf, num, 16);

    printf(" int [%s][%s][%d]\n", buf, cmp, strcmp(buf, cmp));
}

void test_UIntToStr(unsigned int num) 
{
    char buf[16+1];
    char cmp[16+1];

    memset(buf, 0, sizeof(buf));
    memset(cmp, 0, sizeof(cmp));

    sprintf(cmp, "%016u", num);
    NuUIntToStrLPadZero(buf, num, 16);

    printf("uint [%s][%s][%d]\n", buf, cmp, strcmp(buf, cmp));
}

void test_LongToStr(long num) 
{
    char buf[32+1];
    char cmp[32+1];

    memset(buf, 0, sizeof(buf));
    memset(cmp, 0, sizeof(cmp));

    sprintf(cmp, "%032ld", num);
    NuLongToStrLPadZero(buf, num, 32);

    printf(" long [%s][%s][%d]\n", buf, cmp, strcmp(buf, cmp));
}

void test_ULongToStr(unsigned long num) 
{
    char buf[32+1];
    char cmp[32+1];

    memset(buf, 0, sizeof(buf));
    memset(cmp, 0, sizeof(cmp));

    sprintf(cmp, "%032lu", num);
    NuULongToStrLPadZero(buf, num, 32);

    printf("ulong [%s][%s][%d]\n", buf, cmp, strcmp(buf, cmp));
}

void test_IntCopyToStr(int num) 
{
    char buf[16+1];
    char cmp[16+1];

    memset(buf, '#', sizeof(buf));
    memset(cmp, 0, sizeof(cmp));
    buf[16] = '\0';

    sprintf(cmp, "%016d", num);
    NuIntCopyToLPadZero(buf, num, 10);

    printf(" int [%s][%s][%d]\n", buf, cmp, strcmp(buf, cmp));
}

void test_UIntCopyToStr(unsigned int num) 
{
    char buf[16+1];
    char cmp[16+1];

    memset(buf, '#', sizeof(buf));
    memset(cmp, 0, sizeof(cmp));
    buf[16] = '\0';

    sprintf(cmp, "%016u", num);
    NuUIntCopyToLPadZero(buf, num, 10);

    printf(" int [%s][%s][%d]\n", buf, cmp, strcmp(buf, cmp));
}

void test_LongCopyToStr(long num) 
{
    char buf[16+1];
    char cmp[16+1];

    memset(buf, '#', sizeof(buf));
    memset(cmp, 0, sizeof(cmp));
    buf[16] = '\0';

    sprintf(cmp, "%016ld", num);
    NuLongCopyToLPadZero(buf, num, 10);

    printf(" int [%s][%s][%d]\n", buf, cmp, strcmp(buf, cmp));
}

void test_ULongCopyToStr(unsigned long num) 
{
    char buf[16+1];
    char cmp[16+1];

    memset(buf, '#', sizeof(buf));
    memset(cmp, 0, sizeof(cmp));
    buf[16] = '\0';

    sprintf(cmp, "%016lu", num);
    NuULongCopyToLPadZero(buf, num, 10);

    printf(" int [%s][%s][%d]\n", buf, cmp, strcmp(buf, cmp));
}


int main(int argc, char **argv)
{
    test_IntToStr(INT32_MAX);
    test_IntToStr(INT32_MIN);
    test_UIntToStr(UINT32_MAX);

    test_LongToStr(INT64_MAX);
    test_LongToStr(INT64_MIN);
    test_ULongToStr(UINT64_MAX);

    test_IntCopyToStr(55688);
    test_IntCopyToStr(-55688);
    test_UIntCopyToStr(55688);

    test_LongCopyToStr(55688);
    test_LongCopyToStr(-55688);
    test_ULongCopyToStr(55688);

	return 0;
}
