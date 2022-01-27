
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "NuSocketType_InetStrmClnt.h"
#include "NuSocket.h"

#define SCALE               1000000
#define DATA                "123456789012345678901234567890"
#define DATALEN             sizeof(DATA) + 100
#define DEFAULT_THREADNO    3
    
typedef struct _NodeData
{
    int     RecvCnt;
    time_t  Start;
    time_t  Stop;
} NodeData;

static NodeData _Node;

static void OnConnectCB_Clnt(struct _NuSocketNode_t *Node, void *Argu) 
{
    NodeData    *pNodeData = (NodeData *)Argu;

    printf("-->OnConnect\n");
    sleep(3);

    pNodeData->Start = time(NULL);
    NuSocketSend(Node, DATA, DATALEN);
    return;
}

static void OnDataArriveCB_Clnt(struct _NuSocketNode_t *Node, void *Argu)
{
    NodeData    *pNodeData = (NodeData *)Argu;
    char        Buf[DATALEN] = "\0";

    printf("-->OnDataArrive\n");
    while(NuSocketRecvInLen(Node, Buf, DATALEN, DATALEN) > 0)
    {
//printf("RecvCnt[%d]\n", pNodeData->RecvCnt);
        if(++ (pNodeData->RecvCnt) >= SCALE)
        {
//printf("Stop!\n");
            pNodeData->Stop = time(NULL);
            break;
        }
        else
        {
//printf("Send!\n");
            NuSocketSend(Node, DATA, DATALEN);
        }
    }

    return;
}

static void OnRemoteTimeoutCB(struct _NuSocketNode_t *Node, void *Argu)
{
    printf("-->OnRemoteTimeout\n");
    return;
}

static void OnLocalTimeoutCB(struct _NuSocketNode_t *Node, void *Argu)
{
    printf("-->OnLocalTimeout\n");
    return;
}

static void OnDisconnectCB_Clnt(struct _NuSocketNode_t *Node, void *Argu)
{
    printf("-->OnDisconnect\n");
    NuSocketDisconnect(Node);
    return;
}

static void OnLog(char *Format, va_list ArguList, void *Argu)
{
    vprintf(Format, ArguList);
    return;
}

static void Signal(int Sig)
{
    printf("Sig[%d]\n", Sig);
    return;
}

static void Usage(char *ProgName)
{
    printf("Usage: %s <TargetAddr> <TargetPort>\n", ProgName);

    return;
}

int main(int Argc, char **Argv)
{
    int                     Cnt = 0, WorkThreadNo = DEFAULT_THREADNO;
    struct _NuSocket_t      *pSocket = NULL;
    NuSocketProtocol_t      ProtoClnt;
    NuSocketNode_t          *Node = NULL;

    signal(SIGHUP, Signal);
    signal(SIGPIPE, Signal);

    if(Argc < 3)
    {
        Usage(Argv[0]);
        return 0;
    }

    if(NuSocketNew(&pSocket, WorkThreadNo, &OnLog) < 0)
    {
        printf("NuSocketNew Error!\n");
        return 0;
    }

    ProtoClnt.OnConnect = &OnConnectCB_Clnt;
    ProtoClnt.OnDataArrive = &OnDataArriveCB_Clnt;
    ProtoClnt.OnRemoteTimeout = &OnRemoteTimeoutCB;
    ProtoClnt.OnLocalTimeout = &OnLocalTimeoutCB;
    ProtoClnt.OnDisconnect = &OnDisconnectCB_Clnt;

    if(!(Node = NuSocketAdd(pSocket, &InetStrmClnt, &ProtoClnt, Argv[2], strtol(Argv[3], (char **)NULL, 10), NULL, 0, &_Node)))
    {
        printf("Error:: NuSocketAdd(TargetAddr[%s] TargetPort[%ld]) failed.\n", Argv[2], strtol(Argv[3], (char **)NULL, 10));

        Usage(Argv[0]);
        return 0;
    }

    printf("Add Socket OK!\n");

    Cnt = 30;
    while(Cnt --)
    {
        sleep(1);
    };

    printf("[%d] RTT[%ld]\n", SCALE, _Node.Stop - _Node.Start);

    NuSocketFree(pSocket);

    return 0;
}

