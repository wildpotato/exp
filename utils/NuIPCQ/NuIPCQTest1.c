
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#include "NuTime.h"
#include "NuIPCQ.h"

#define IPCKEY "0x51f00001"

#define MAXBUFSZ 1024

#define HEXVAL(x)       (((x) >= 'A') ? ((x)-'A'+10) : (x) - '0')
unsigned int str_hex_to_uint(char *szHex) {
	char szHexVal[32] = {0};          /* 0x38000306 */
	int  len = 0;
	unsigned int rlng = 0;
	char *p = NULL;

	strcpy(szHexVal, szHex);

	len = strlen(szHexVal);

	if (len < 3 || len > 10) {
		return -1;
	}

	if ( !(szHexVal[0] == '0' && (szHexVal[1] == 'x' || szHexVal[1] == 'X')) ) {
		return -1;
	}

	for (p = (szHexVal + 2); (p - szHexVal) < len && *p != '\0'; p++) {
		rlng = rlng * 16 + HEXVAL( toupper(*p) );
	}
	return rlng;
}

typedef struct stData {
	NuIPCQMsgHdr_t  Hdr;
	struct timespec TStart;
	struct timespec TEnd;
	char            Data[1];
} stData;

static void showTimeDiff(int idx, stData *p) {
	
	double s = (double)p->TStart.tv_sec + (double)(1.0e-9)*(p->TStart.tv_nsec);
	double e = (double)p->TEnd.tv_sec + (double)(1.0e-9)*(p->TEnd.tv_nsec);

	printf("%04d|%.10f sec\n", idx, e - s);
}

static void enQ(NuIPCQ_t *q, int dataSz, int dataCnt) {
	int i = 0;
	int sndSz = sizeof(stData) + dataSz;

	stData *pData = NULL;
	char *data = (char *)malloc(sizeof(char) * sndSz);
	if (data == NULL) {
		return;
	}

	pData = (stData *)data;
	pData->Hdr.MsgType = 1;
	sndSz = sizeof(stData) - sizeof(NuIPCQMsgHdr_t);


	for (i = 0; i < dataCnt; i++) {
		clock_gettime(CLOCK_MONOTONIC, &pData->TStart);
		NuIPCQEnqueue(q, (NuIPCQMsgHdr_t *)pData, sndSz);
		usleep(10 * 1000);
		//	printf("%.10f sec\n", (double)pData->TStart.tv_sec + (double)(1.0e-9)*(pData->TStart.tv_nsec));
	}

	if (data != NULL) {
		free(data);
	}
	return;
}

static void deQ(NuIPCQ_t *q, int dataSz, int dataCnt, int act) {
	int rc = 0;
	int i = 0;
	int sndSz = sizeof(stData) + dataSz;

	stData *pData = NULL;
	char *aryData[dataCnt];
	for (i = 0; i < dataCnt; i++) {
		aryData[i] = (char *)malloc(sizeof(char) * sndSz);
		if (aryData[i] == NULL) {
			return;
		}
	}
	// ---------------------------------------------
	sndSz = sizeof(stData) - sizeof(NuIPCQMsgHdr_t);

	i = 0;
	if (act == 0) {
		while(i < dataCnt) {
			pData = (stData *)aryData[i];

			rc = NuIPCQBlockingDequeue(q, (NuIPCQMsgHdr_t *)pData, sndSz);
			if (rc == 0) {
				continue;
			} else if (rc > 0) {
				clock_gettime(CLOCK_MONOTONIC, &pData->TEnd);
				i++;
			} else {
				return;
			}
		}
	} else if (act == -1 ){
		while(i < dataCnt) {
			pData = (stData *)aryData[i];

			rc = NuIPCQDequeue(q, (NuIPCQMsgHdr_t *)pData, sndSz);
			if (rc == 0) {
				continue;
			} else if (rc > 0) {
				clock_gettime(CLOCK_MONOTONIC, &pData->TEnd);
				i++;
			} else {
				return;
			}
		}
	}

	printf("recv completed, i = %d\n", i);
	// ---------------------------------------------
	for (i = 0; i < dataCnt; i++) {
		pData = (stData *)aryData[i];

		showTimeDiff(i, pData);
		free(aryData[i]);
	}
}

static void Usage(const char *Prog)
{
    printf("%s -m <s/c> -s size -c count \n", Prog);
    printf("m  : s=server, S=server nonblocking, c=client \n");
    printf("s  : data size \n");
    printf("c  : data count \n");

    return;
}

int main(int argc, char **argv)
{
    int iRC = 0;
    int i = 0;
    int cnt = 100;
	int sz = 1024;

    int action = 0; /* deQ = 0, enQ = 1 */

    for (i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-c")) {
			++i;
            cnt = atoi(argv[i]);
        } else if (!strcmp(argv[i], "-s")) {
            ++i;
            sz = atoi(argv[i]);
        } else if (!strcmp(argv[i], "-m")) {
            ++i;
            if (!strcmp(argv[i], "c")) {
                action = 1;
            } else if (!strcmp(argv[i], "S")) {
				action = -1;
			}
        } else if (!strcmp(argv[i], "-h")) { 
			Usage(argv[0]);
			goto EXIT;
		}
    }

    printf("mod = %s, cnt = %d, size = %d\n", (action == 1) ? "enQ" : "deQ", cnt, sz);

    // --------------------------------------------------
	NuIPCQ_t   *q = NULL;
	key_t    qKey = str_hex_to_uint(IPCKEY);

	iRC = NuIPCQNew(&q);
	if (iRC < 0){
		printf("err : ipcq new fail. rc = %d\n", iRC);
		goto EXIT;
	}

	iRC = NuIPCQCreate(q, &qKey);
	if (iRC < 0){
		printf("err : ipcq create fail. rc = %d\n", iRC);
		goto EXIT;
	}

    switch(action) {
        case 0:
            deQ(q, sz, cnt, action);
            break;
        case 1:
            enQ(q, sz, cnt);
            break;
        default:
            break;
    }


	if (q != NULL) {
		NuIPCQFree(q);
	}

EXIT:
	return 0;
}
