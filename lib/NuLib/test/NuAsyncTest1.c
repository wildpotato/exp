
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "NuTime.h"
#include "NuAsync.h"


void invokefn1(void *args) {
	char szTime[15+1];
	NuGetTime(szTime);
	printf("test1 %s, tid = %ld\n", szTime, pthread_self());	
}

void invokefn2(void *args) {
	char szTime[15+1];
	NuGetTime(szTime);
	printf("test2 %s, tid = %ld\n", szTime, pthread_self());	
}

void test1(NuAsync_t *obj) {
	int i = 0;
	for (i = 0; i < 10; i++) {
		NuAsyncInvoke(obj, &invokefn1, NULL);
	}
}

void test2(NuAsync_t *obj) {
	int i = 0;
	for (i = 0; i < 10; i++) {
		NuAsyncInvoke(obj, &invokefn2, NULL);
	}
}

int main(int argc, char **argv) {
	int rc = 0;
	NuAsync_t *obj = NULL;
	
	rc = NuAsyncNew(&obj, 16);
	if (rc < 0) {
		printf("new error, rc = %d\n", rc);
		return -1;
	}

	NuAsyncAddThread(obj, 1);

	NuAsyncStart(obj);

	test1(obj);
	test2(obj);

	printf("before sleep\n");
	sleep(5);
	printf("before stop\n");
	NuAsyncStop(obj);

	NuAsyncFree(obj);

	return 0;
}
