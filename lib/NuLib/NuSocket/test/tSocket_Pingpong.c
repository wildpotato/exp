
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "NuSocketType_Pipe.h"
#include "NuSocketType_InetStrmClnt.h"

#include "NuSocket.h"

#define DATA                "123456789012345678901234567890"
#define DATALEN             sizeof(DATA)
#define DATAHDRLEN          512
#define DEFAULT_TARGETCNT   1
#define DEFAULT_THREADNO    3

struct timeval  Start, Stop;
struct timezone Tz;
int             RecvCnt = 0;
int             TargetCnt = DEFAULT_TARGETCNT;

static void OnConnectCB_Clnt(struct _NuSocketNode_t *Node, void *Argu) 
{
    printf("-->OnConnect\n");
    sleep(3);

    gettimeofday(&Start, &Tz);
    NuSocketSend(Node, DATA, DATALEN);
    return;
}

static void OnDataArriveCB_Clnt(struct _NuSocketNode_t *Node, void *Argu)
{
    char        Buf[DATALEN + DATAHDRLEN] = "\0";

//    printf("-->OnDataArrive\n");
    while(NuSocketRecvInLen(Node, Buf, DATALEN, DATALEN) > 0)
    {
        if(++ RecvCnt >= TargetCnt)
        {
            gettimeofday(&Stop, &Tz);
        }
        else
        {
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
    printf("Usage: %s <TargetAddr> <TargetPort> <LocalAddr> <LocalPort> [SendNum] [WorkThreads]\n", ProgName);

    return;
}

int main(int Argc, char **Argv)
{
    int                     WorkThreadNo = DEFAULT_THREADNO;
    struct _NuSocket_t      *pSocket = NULL;
    NuSocketProtocol_t      ProtoClnt;
    long                    SecDiff = 0, USecDiff = 0;

    signal(SIGHUP, Signal);
    signal(SIGPIPE, Signal);

    if(Argc < 5)
    {
        Usage(Argv[0]);
        return 0;
    }
    else
    {
        if(Argc > 5)
        {
            TargetCnt = strtol(Argv[5], (char **)NULL, 10);
        }

        if(Argc > 6)
        {
            WorkThreadNo = strtol(Argv[6], (char **)NULL, 10);
        }
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

    if(!(NuSocketAdd(pSocket, &InetStrmClnt, &ProtoClnt, Argv[1], strtol(Argv[2], (char **)NULL, 10), Argv[3], strtol(Argv[4], (char **)NULL, 10), NULL)))
    {
        printf("Error::NuSocketAdd(TargetAddr[%s] TargetPort[%ld], LocalAddr[%s] LocalPort[%ld]) failed.\n", Argv[1], strtol(Argv[2], (char **)NULL, 10), Argv[3], strtol(Argv[4], (char **)NULL, 10));

        Usage(Argv[0]);
        return 0;
    }

    printf("Let Rock & Roll via %d threads!\n", WorkThreadNo);

    while(1)
    {
        sleep(1);
        if(RecvCnt >= TargetCnt)
        {
            break;
        }
    };

    SecDiff = Stop.tv_sec - Start.tv_sec;
    USecDiff = Stop.tv_usec - Start.tv_usec;

    if(USecDiff < 0)
    {
        SecDiff -= 1;
        USecDiff += 1000000;
    }

    printf("PingPong in [%d] times:\n", TargetCnt);
    printf("Start @ [%ld.%06ld]s\n", Start.tv_sec , Start.tv_usec);
    printf("Stop @ [%ld.%06ld]s\n", Stop.tv_sec , Stop.tv_usec);
    printf("Stop - Start = [%ld.%06ld]s\n", SecDiff, USecDiff);
    printf("---------- Average[%ld]us ----------\n", (SecDiff * 1000000 + USecDiff) / RecvCnt);

    return 0;
}

