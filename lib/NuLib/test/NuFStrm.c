#include <assert.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "NuLog.h"


#define DATA "testfunction"
int main()
{
	NuLog_t *pLog = NULL;

	NuLogOpen(&pLog, "./", "testlog");

	
	NuLog(pLog, "%s\n", DATA);
	NuErr(pLog, "%s\n", DATA);
	NuLogBin(pLog, DATA, strlen(DATA));

	NuLogClose(pLog);
	return 0;
}

