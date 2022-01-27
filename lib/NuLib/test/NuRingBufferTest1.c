
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "NuThread.h"
#include "NuRingBuffer.h"

#define PrintSize(o)    printf("caps = %ld, size = %ld\n", NuRBufGetCapicity((o)), NuRBufGetSize((o)))
//#define PrintBuffer(o)  printf("buffer [%.*s], caps = %ld, head[%ld], tail[%ld]\n", (int)NuRBufGetCapicity((o)), (o)->base->buffer, NuRBufGetCapicity((o)), (o)->head, (o)->tail)
#define PrintBuffer(o)  printf("buffer [%.*s], head[%ld], tail[%ld]\n", (int)NuRBufGetCapicity((o)), (o)->base->buffer, (o)->base->head, (o)->base->tail)
/*
#define RB_Append_Str(o, str) do {               \
	strcpy(data, (str));                         \
	rc = NuRBufWrite((o), (str), strlen((str))); \
	assert( rc == strlen(data) );                \
	PrintBuffer(rb);                             \
	PrintSize(rb);                               \
} while(0)
*/
#define RB_Append_Str(o, str) do {               \
	strcpy(data, (str));                         \
	rc = NuRBufWrite((o), (str), strlen((str))); \
	PrintBuffer(rb);                             \
	PrintSize(rb);                               \
} while(0)

bool test1() {
	int    rc = 0;
	size_t sz = 10;
	char data[20+1] = {0};
	char buf[10+1]  = {0};
	NuRBuf_t *rb = NULL;

	rb = NuRBufNew(sz);
	assert( rb != NULL );
	assert( NuRBufGetCapicity(rb) == 16 );

	printf("caps = %ld, init sz = %ld\n", NuRBufGetCapicity(rb), sz);

	/* add data */
	RB_Append_Str(rb, "12345");
	RB_Append_Str(rb, "ABCD");
	RB_Append_Str(rb, "@#$%^&");

	rc = NuRBufWrite(rb, "TIGER", 5);
	assert( rc == 0 );
	if (rc == 0) {
		rc = NuRBufRead(rb, buf, 10);
		assert( rc == 10 );
		buf[rc] = '\0';

		assert( strcmp(buf, "12345ABCD@") == 0 );
		printf("read, rc = %d, buf = [%s]\n", rc, buf);

		PrintSize(rb);
		RB_Append_Str(rb, "TIGER");
	}
	PrintBuffer(rb);
	PrintSize(rb);

	/* try read 5 byte */
	rc = NuRBufTryRead(rb, buf, 5);
	assert( rc == 5 );
	buf[rc] = '\0';
	assert( strcmp(buf, "#$%^&") == 0 );
	printf("try read rc = %d, buf = [%s]\n", rc, buf);

	/* read 5 byte */
	rc = NuRBufRead(rb, buf, 5);
	assert( rc == 5 );
	buf[rc] = '\0';
	assert( strcmp(buf, "#$%^&") == 0 );
	printf("read rc = %d, buf = [%s]\n", rc, buf);

	/* no enougth data can read */
	rc = NuRBufRead(rb, buf, 10);
	assert( rc == 0 );

	/* read 5 byte */
	rc = NuRBufRead(rb, buf, 5);
	assert( rc == 5 );
	buf[rc] = '\0';
	assert( strcmp(buf, "TIGER") == 0 );
	printf("read rc = %d, buf = [%s]\n", rc, buf);

	/* no data can read */
	rc = NuRBufRead(rb, buf, 10);
	assert( rc == 0 );

	/* test skip */
	RB_Append_Str(rb, "TEST SKIP DATA");
	rc = NuRBufRead(rb, buf, 5);
	assert( rc == 5 );
	buf[rc] = '\0';
	assert( strcmp(buf, "TEST ") == 0 );

	rc = NuRBufSkip(rb, 2);
	assert( rc == 2 );

	rc = NuRBufRead(rb, buf, 7);
	buf[rc] = '\0';
	assert( strcmp(buf, "IP DATA") == 0 );

	NuRBufFree(rb);
	return true;
}

bool test2() {
	int          rc = 0;
	size_t       sz = 10;
	char data[20+1] = {0};
	char  buf[10+1] = {0};
	NuRBuf_t    *rb = NULL;

	rb = NuRBufNew(sz);
	assert( rb != NULL );
	assert( NuRBufGetCapicity(rb) == 16 );

	printf("caps = %ld, init sz = %ld\n", NuRBufGetCapicity(rb), sz);

	/* add data */
	RB_Append_Str(rb, "0123456789abcde");

	rc = NuRBufRead(rb, buf, 3);
	RB_Append_Str(rb, "XXX");
	rc = NuRBufRead(rb, buf, 3);
	RB_Append_Str(rb, "GGG");

	if (rc == 0) {
		rc = NuRBufRead(rb, buf, 10);
		assert( rc == 10 );
		buf[rc] = '\0';

		assert( strcmp(buf, "12345ABCD@") == 0 );
		printf("read, rc = %d, buf = [%s]\n", rc, buf);

		PrintSize(rb);
		RB_Append_Str(rb, "TIGER");
	}
	PrintBuffer(rb);
	PrintSize(rb);

	/* try read 5 byte */
	//rc = NuRBufTryRead(rb, buf, 5);
	//assert( rc == 5 );
	//buf[rc] = '\0';
	//assert( strcmp(buf, "#$%^&") == 0 );
	//printf("try read rc = %d, buf = [%s]\n", rc, buf);

	NuRBufFree(rb);
	return true;
}
int main(int argc, char **argv)
{
	test1();
	//test2();
	return 0;
}
