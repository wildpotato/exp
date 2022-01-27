#include <assert.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "NuAsyncLog.h"
#include "NuTuneTools.h"

#define DATA "testfunction"


void test(NuAsyncLog_t *pLog) {
    int i = 0;
	NuTuneSetStart();
    for (i = 0; i < 100; i++) {
        NuAsyncLog(pLog, "TEST1 %s\n", DATA);
    }
    NuAsyncLog(pLog, "================================\n");

    for (i = 0; i < 100; i++) {
        NuAsyncErr(pLog, "TEST2 %s\n", DATA);
    }
	NuTuneSetStop();
	printf("%ld sec, %ld nsec\n", NuTuneGetSecElapse(), NuTuneGetNanoSecElapse());
}

int main() {
    NuAsyncLog_t *pLog = NULL;

    NuAsyncLogOpen2(&pLog, "./", "asynlog", 128, 64);
    //NuAsyncLogOpen2(&pLog, "./", "asynlog", 256, 256);
    //NuAsyncLogOpen(&pLog, "./", "asynlog");

    //NuAsynLogSetLogLv(pLog, enNuAsyncLogType_MSG);
   
    test(pLog);

    NuAsyncLogClose(pLog);

    return 0;	
}
