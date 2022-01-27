
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <NuNum.h>


int main() 
{
	int iRC= 0;
	NuNum_t *num1 = NULL, *num2 = NULL, *num3 = NULL;
	char Num[32] = {0};

	if ((iRC = NuNumNew(&num1, 10)) < 0)
	{
		printf(" new num1 rc(%d) \n", iRC);
		return 0;
	}

	if ((iRC = NuNumNew(&num2, 10)) < 0)
	{
		printf(" new num2 rc(%d) \n", iRC);
		return 0;
	}

	if ((iRC = NuNumNew(&num3, 10)) < 0)
	{
		printf(" new num3 rc(%d) \n", iRC);
		return 0;
	}

	NuNumFromStr(num1, "2850319380.0000");
	NuNumFromStr(num2, "23.35");

	NuNumMultiply(num1, num2, num3);

	NuNumToStr(num3, Num);

	printf("[%s]\n", Num);

	return 0;
}


