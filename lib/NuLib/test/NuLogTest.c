#include <assert.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "NuFLog.h"
#include "NuTuneTools.h"


#define DATA "testfunction"
#if 0
int main()
{
	NuFLog_t *pLog = NULL;

	NuFLogOpen(&pLog, "./", "testlog");

	
	NuFLog(pLog, "%s\n", DATA);
	NuErr(pLog, "%s\n", DATA);
	NuFLogBin(pLog, DATA, strlen(DATA));

	NuFLogClose(pLog);
	return 0;
}
#else
void test(NuFLog_t *pLog) {
    int i = 0;
	NuTuneSetStart();
    for (i = 0; i < 100; i++) {
        NuFLog(pLog, "TEST1 %s\n", DATA);
    }
    NuFLog(pLog, "================================\n");

    for (i = 0; i < 100; i++) {
        NuFLog(pLog, "TEST2 %s\n", DATA);
        NuFLogFlush(pLog);
    }
	NuTuneSetStop();

	printf("%ld sec, %ld nsec\n", NuTuneGetSecElapse(), NuTuneGetNanoSecElapse());
}

int main() {
	NuFLog_t *pLog = NULL;

	NuFLogOpen(&pLog, "./", "testlog");
    
    test(pLog);

	NuFLogClose(pLog);

    return 0;	
}
#endif
