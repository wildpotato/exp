
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "NuThread.h"
#include "NuCircularQ.h"
#include "NuTuneTools.h"


typedef struct _data_t {
    char   Sym[6+1];
    int    Qty;
    double Px;
} data_t;

typedef struct _param_t {
    NuCQ_t *Q;
    int    Cnt;
} param_t;

void *test1_thd1(void *args) {
    param_t *param = (param_t *)args;
    //char szBuf[128] = {0};
    data_t data;
    size_t len = sizeof(data_t);
    int i = 0;

    memset(&data, 0x00, len);

    for (i = 0; i < param->Cnt; i++) {
        sprintf(data.Sym, "%06d", i);
        data.Qty = i * 10;
        data.Px  = (double)i * (double)10 / (double)3;
        NuCQEnqueue(param->Q, (char *)&data, len);
    }
    printf("enQ end\n");

    return NULL;
}

void test1(int cnt) {
    int    rc  = 0;
    NuCQ_t *cq = NULL;
    data_t *p  = NULL;
    char   szBuf[128] = {0};
    size_t len = 0;
    NuThread_t thd = 0;
    param_t param;

    rc = NuCQNew(&cq, sizeof(data_t), 10);
    //rc = NuCQNew(&cq, 128, 10);
    printf("[new] rc = %d\n", rc);

    // create enQ thread
    param.Q = cq;
    param.Cnt = cnt;
    NuThdCreate(&test1_thd1, &param, &thd);
    // ----------------------------
    
    printf("[deq] start, %d\n", cnt);

    do {
        if (NuCQDequeue(cq, szBuf, &len) == NU_OK) {
            p = (data_t *)szBuf;
            printf("len = %ld, [%s, %d, %.3f]\n", len, p->Sym, p->Qty, p->Px);
            //printf("len = %ld, [%s]\n", len, szBuf);
        }
        --cnt;
    } while(cnt > 0);

    printf("[deq] end, %d\n", cnt);

    NuCQClose(cq);
    NuCQFree(cq);
}

int main(int argc, char **argv)
{
    int cnt = 1;
    if (argc > 1) {
        cnt = atoi(argv[1]);
    }
    printf("cnt = %d\n", cnt);
    test1(cnt);

	return 0;
}
