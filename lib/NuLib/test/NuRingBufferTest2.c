
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "NuCStr.h"
#include "NuFile.h"
#include "NuThread.h"
#include "NuRingBuffer.h"

#define PrintSize(o)    printf("caps = %ld, size = %ld\n", NuRBufGetCapicity((o)), NuRBufGetSize((o)))
#define PrintBuffer(o)  printf("buffer [%.*s]\n", (int)NuRBufGetCapicity((o)), (o)->base->buffer)
//#define PrintDBG(o)  printf("buffer [%.*s], h[%c], t[%c]\n", (int)NuRBufGetCapicity((o)), (o)->base->buffer, *((o)->base->buffer + (o)->base->head), *((o)->base->buffer + (o)->base->tail))
#define PrintDBG(o)  printf("write[%c][%ld], read[%c][%ld]\n", *((o)->base->buffer + (o)->base->head), (o)->base->head, *((o)->base->buffer + (o)->base->tail), (o)->base->tail)

#define FILENAME "./rbTest"
//#define FILETEXT "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHI"
#define FILETEXT "0123456789abcdefghijklmnopqrstu"
#define APPENDTEXT "TT123"

bool genFile(char *fName) {
	FILE *fhdl = NULL;
	if (NuIsFile(fName) == false) {
		fhdl = fopen(FILENAME, "a+");
		fwrite(FILETEXT, sizeof(FILETEXT), sizeof(char), fhdl);
		fclose(fhdl);

		printf("file data[%s]\n", FILETEXT);
		return true;
	} else {
		return false;
	}
}

bool appendFile(char *fName) {
	FILE *fhdl = NULL;
	if (NuIsFile(fName) == false) {
		fhdl = fopen(FILENAME, "a+");
		if (fhdl == NULL) {
			printf("append fail\n");
			return false;
		}
		fwrite(APPENDTEXT, sizeof(APPENDTEXT), sizeof(char), fhdl);
		fclose(fhdl);
		return true;
	} else {
		return false;
	}
}


void test1() {
	int      sz    = 10;
	FILE     *fhdl = NULL;
	int      fd    = 0;
	int      rc    = 0;
	size_t   readn = 0;
	NuRBuf_t *rb   = NULL;
	char buf[10+1] = {0};
	bool     needDel = false;
	
	rb = NuRBufNew(sz);
	assert( rb != NULL );
	assert( NuRBufGetCapicity(rb) == 16 );

	needDel = genFile(FILENAME);

	fhdl = fopen(FILENAME, "r");
	assert ( fhdl != NULL );

	fd = fileno(fhdl);
	printf("fd = %d\n", fd);

	/* ----------------------------- */
	rc = NuRBufRecv(rb, fd, 5, &readn, 0);
	printf("read n = %ld\n", readn);
	PrintSize(rb);
	PrintBuffer(rb);

	rc = NuRBufRecv(rb, fd, 0, &readn, 0);
	printf("read n = %ld\n", readn);
	PrintSize(rb);
	PrintBuffer(rb);
	// --
	rc = NuRBufRecv(rb, fd, 5, &readn, 0);
	assert( rc == 0 );
	printf("read n = %ld\n", readn);
	PrintSize(rb);
	PrintBuffer(rb);

	rc = NuRBufRecv(rb, fd, 0, &readn, 0);
	assert( rc == 0 );
	printf("read n = %ld\n", readn);
	PrintSize(rb);
	PrintBuffer(rb);

	// --
	rc = NuRBufRead(rb, buf, 10);
	buf[rc] = '\0';
	printf("read, rc = %d, buf = [%s]\n", rc, buf);
	// --
	rc = NuRBufRecv(rb, fd, 0, &readn, 0);
	printf("read n = %ld\n", readn);
	PrintSize(rb);
	PrintBuffer(rb);

	/* ----------------------------- */
	NuRBufFree(rb);
	fclose(fhdl);

	if (needDel) {
		remove(FILENAME);
	}
}

void test2() {
	int      sz    = 10;
	FILE     *fhdl = NULL;
	int      fd    = 0;
	int      rc    = 0;
	size_t   readn = 0;
	NuRBuf_t *rb   = NULL;
	char buf[10+1] = {0};
	bool     needDel = false;
	
	rb = NuRBufNew(sz);
	assert( rb != NULL );
	assert( NuRBufGetCapicity(rb) == 16 );

	needDel = genFile(FILENAME);

	fhdl = fopen(FILENAME, "r");
	assert ( fhdl != NULL );

	fd = fileno(fhdl);
	printf("fd = %d\n", fd);

	/* ----------------------------- */
	rc = NuRBufRecv(rb, fd, 6, &readn, 0);
	printf("recv n = %ld, rc = %d\n", readn, rc);
	PrintSize(rb);
	PrintBuffer(rb);

	// --
	rc = NuRBufRead(rb, buf, 3);
	buf[rc] = '\0';
	printf("read, rc = %d, buf = [%s]\n", rc, buf);
	// --
	rc = NuRBufRecv(rb, fd, 15, &readn, 0);
	printf("recv 15 n = %ld, rc = %d\n", readn, rc);
	PrintSize(rb);
	PrintBuffer(rb);

	rc = NuRBufRecv(rb, fd, 8, &readn, 0);
	printf("recv 8 n = %ld, rc = %d\n", readn, rc);
	PrintSize(rb);
	PrintBuffer(rb);

	rc = NuRBufRecv(rb, fd, 0, &readn, 0);
	printf("recv 0 n = %ld, rc = %d\n", readn, rc);
	PrintSize(rb);
	PrintBuffer(rb);
	// --
	rc = NuRBufRead(rb, buf, 8);
	buf[rc] = '\0';
	printf("read, rc = %d, buf = [%s]\n", rc, buf);

	/* ----------------------------- */
	NuRBufFree(rb);
	fclose(fhdl);

	if (needDel) {
		remove(FILENAME);
	}
}

void test3() {
	int      sz    = 10;
	FILE     *fhdl = NULL;
	int      fd    = 0;
	int      rc    = 0;
	size_t   readn = 0;
	NuRBuf_t *rb   = NULL;
	char buf[10+1] = {0};
	bool     needDel = false;
	
	rb = NuRBufNew(sz);
	assert( rb != NULL );
	assert( NuRBufGetCapicity(rb) == 16 );

	needDel = genFile(FILENAME);

	fhdl = fopen(FILENAME, "r");
	assert ( fhdl != NULL );

	fd = fileno(fhdl);
	printf("fd = %d\n", fd);

	/* ----------------------------- */
	memcpy(rb->base->buffer, "012345", 6);
	PrintDBG(rb);
	NuHexDump (rb->base->buffer, (int)rb->base->caps);
	int i = 0;
	for (i = 0; i < 4; i++) {
		printf("---------------------\n");
		rc = NuRBufRecv(rb, fd, 0, &readn, 0);
		//printf("recv n = %ld\n", readn);
		PrintDBG(rb);
		NuHexDump (rb->base->buffer, (int)rb->base->caps);
		printf("---------------------\n");

		rc = NuRBufRead(rb, buf, 5);
		buf[rc] = '\0';
		//printf("read, rc = %d, [%s]\n", rc, buf);
		NuHexDump (buf, rc);
		PrintDBG(rb);

		rc = NuRBufRead(rb, buf, 5);
		buf[rc] = '\0';
		//printf("read, rc = %d, [%s]\n", rc, buf);
		NuHexDump (buf, rc);
		PrintDBG(rb);

		if (i > 1) {
			rc = NuRBufRead(rb, buf, 5);
			buf[rc] = '\0';
			//printf("read, rc = %d, [%s]\n", rc, buf);
			NuHexDump (buf, rc);
			PrintDBG(rb);
		}
		if (i == 2) {
			//clearerr(fhdl) ;
			printf("reopen file\n");
			//appendFile(FILENAME);

			fhdl = fopen(FILENAME, "r");
			fd = fileno(fhdl);
		}
	}


	/* ----------------------------- */
	NuRBufFree(rb);
	fclose(fhdl);

	if (needDel) {
		remove(FILENAME);
	}
}

void test4() {
	int      sz    = 10;
	FILE     *fhdl = NULL;
	int      fd    = 0;
	int      rc    = 0;
	size_t   readn = 0;
	NuRBuf_t *rb   = NULL;
	char buf[10+1] = {0};
	bool     needDel = false;
	size_t   availableSz = 0;
	
	rb = NuRBufNew(sz);
	assert( rb != NULL );
	assert( NuRBufGetCapicity(rb) == 16 );

	needDel = genFile(FILENAME);

	fhdl = fopen(FILENAME, "r");
	assert ( fhdl != NULL );

	fd = fileno(fhdl);
	printf("fd = %d\n", fd);

	/* ----------------------------- */
	memcpy(rb->base->buffer, "012345", 6);
	PrintDBG(rb);
	NuHexDump (rb->base->buffer, (int)rb->base->caps);

	NuRBufRecv(rb, fd, 0, &readn, 0);
	availableSz = NuRBufGetSize(rb);

	for (;;) {
		availableSz = NuRBufGetSize(rb);
		printf("availableSz = %ld\n", availableSz);

		if (availableSz < 5) {
			printf("---- READ DATA ----\n");
			printf("availableSz = %ld\n", availableSz);
			rc = NuRBufRecv(rb, fd, 0, &readn, 0);
			//rc = NuRBufRecvInTime(rb, fd, 5, &readn, 5);
			if (rc >= 0 && readn > 0) {
				NuHexDump (rb->base->buffer, (int)rb->base->caps);
			} else {
				printf("recv rc = %d, readn = %ld.\n", rc, readn);
				//sleep(1);
				//continue;
				break;
			}
		}

		rc = NuRBufRead(rb, buf, 5);
		buf[rc] = '\0';
		printf("buf = [%s]\n", buf);
		//NuHexDump (buf, rc);
	}


	/* ----------------------------- */
	NuRBufFree(rb);
	fclose(fhdl);

	if (needDel) {
		remove(FILENAME);
	}
}

int main(int argc, char **argv)
{
	//test1();
	//test2();
	//test3();
	test4();
	return 0;
}
