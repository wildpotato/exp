#include <stdio.h>
#include <stdlib.h>
#include <sched.h>

#include "NuMutex.h"

void testSub(NuMutex_t *mtx) {
	return;
}

void testMain(NuMutex_t *mtx) {
	return;
}

int main() {
	pid_t pid = 0;
	NuMutex_t mtx;

 

	/* fork sub process */
    Pid = fork();
	if (pid < 0) {
		printf("fork fail. \n");
		goto EXIT;
	} else if (pid == 0) {
		testSub(&mtx);
	} 

	/* main process loop */
	testMain(&mtx);



EXIT:
	return 0;
}
