#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "NuCBMgr.h"
#include "NuTuneTools.h"


static void _WorkFn(const void *RaiseArgu, void *Argu)
{
	int i = 0; 

	for (i = 0; i < 1000000; i++);

}

int main(int argc, char **argv)
{
	int i = 0;
	const int no[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	NuCBMgr_t *event = NuCBMgrAdd();

	for (i = 0; i < 1; i++)
	{
    	NuCBMgrRegisterEvent(event, &_WorkFn, (void *)&(no[i]));
	}

	NuTuneSetStart();
	//NuCBMgrRaiseEventParallel(event, NULL);
	NuTuneSetStop();

	printf("nano = %ld\n", NuTuneGetNanoSecElapse());

	NuTuneSetStart();
	NuCBMgrRaiseEvent(event, NULL);
	NuTuneSetStop();
	printf("nano = %ld\n", NuTuneGetNanoSecElapse());

    NuCBMgrDel(event);
    return 0;
}
