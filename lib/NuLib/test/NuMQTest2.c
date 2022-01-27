
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "NuThread.h"
#include "NuMMapQ.h"
#include "NuTuneTools.h"

#define THREAD_NUM  1
#define DATA_NUM  10 

int _DumpFn(NuMQ_Msg_t *msg, void *args) {
    printf("idx[%ld], len[%ld] [%.*s]\n", msg->Idx, msg->DataLen, (int)msg->DataLen, msg->Data);
    return 0;
}

int main()
{
	int iRC = 0;
	int i = 0;
    size_t len = 0;
    char szSeqNo[5+1] = {0};

	NuMQ_t *Que = NULL;
    //NuMQ_Msg_t *pMsg = NULL;

	iRC = NuMQNew(&Que, 6, 100, "./test2.mq");
    if (iRC < 0) {
        printf("create mq fail.\n");
        return 0;
    }

    printf("Hdr[%ld, %ld, %ld, %ld]\n", Que->Hdr->StartIndex, 
                                    Que->Hdr->StopIndex,
                                    Que->Hdr->Capacity,
                                    Que->Hdr->MaxLen);
    
    printf("is empty [%d]\n", NuMQIsEmpty(Que));

    for (i = 0; i < 5; i++) {
        len = sprintf(szSeqNo, "B%04d", i);
        NuMQEnqueue(Que, szSeqNo, len);
        printf("EnQ Hdr[%ld, %ld]\n", Que->Hdr->StartIndex, Que->Hdr->StopIndex);
    }

    NuMQDump(Que, &_DumpFn, NULL);

//    for (i = 0; i < Que->Hdr->Capacity; i++) {
//        pMsg = Que->Arrays[i];
//        printf("idx[%ld], len[%ld] [%.*s]\n", pMsg->Idx, pMsg->DataLen, (int)pMsg->DataLen, pMsg->Data);
//    }

//    for (i = 0; i < 5; i++) {
//        if (NuMQDequeue(Que, szSeqNo, sizeof(szSeqNo), &len) < 0) {
//            printf("[%ld, %ld]\n", sizeof(szSeqNo), len);
//        } else {
//            printf("[%s]\n", szSeqNo);
//        }
//        printf("DeQ Hdr[%ld, %ld]\n", Que->Hdr->StartIndex, Que->Hdr->StopIndex);
//    }

	NuMQFree(Que);
	return 0;
}
