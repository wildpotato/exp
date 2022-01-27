
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "NuSocketType_Pipe.h"
#include "NuSocketType_InetStrmClnt.h"
#include "NuSocketType_InetStrmSvr.h"
#include "NuSocketType_InetDgrmClnt.h"
#include "NuSocketType_InetDgrmSvr.h"
#include "NuSocketType_InetMcstClnt.h"
#include "NuSocketType_InetMcstSvr.h"
#include "NuSocketType_UnixStrmClnt.h"
#include "NuSocketType_UnixStrmSvr.h"
#include "NuSocketType_UnixDgrmClnt.h"
#include "NuSocketType_UnixDgrmSvr.h"

#include "NuSocket.h"

#define SCALE               1000000
#define DATA                "123456789012345678901234567890"
#define DATALEN             sizeof(DATA) + 100
#define DEFAULT_NO          1
#define DEFAULT_THREADNO    3
    
static char Msg[DATALEN] = "\0";

typedef struct _SocketType
{
    char            *ShortName;
    char            *Name;
    NuSocketType_t  *Type;
} SocketType;

static SocketType Type[] =    {
                                {"pip", "Pipe", &Pipe},
                                {"iss", "Internet Stream Server", &InetStrmSvr},
                                {"isc", "Internet Stream Client", &InetStrmClnt},
                                {"ids", "Internet Datagram Server", &InetDgrmSvr},
                                {"idc", "Internet Datagram Client", &InetDgrmClnt},
                                {"ims", "Internet Multicast Server", &InetMcstSvr},
                                {"imc", "Internet Multicast Client", &InetMcstClnt},
                                {"uss", "Unix Stream Server", &UnixStrmSvr},
                                {"usc", "Unix Stream Client", &UnixStrmClnt},
                                {"uds", "Unix Datagram Server", &UnixDgrmSvr},
                                {"udc", "Unix Datagram Client", &UnixDgrmClnt}
                            };

#define TypeNo  (sizeof(Type)/sizeof(SocketType))

typedef struct _NodeData
{
    int     RecvCnt;
    time_t  Start;
    time_t  Stop;
} NodeData;

static void OnConnectCB_Clnt(struct _NuSocketNode_t *Node, void *Argu) 
{
    NodeData    *pNodeData = (NodeData *)Argu;
	int iRC = 0;

    printf("-->OnConnect\n");
    sleep(3);

    pNodeData->Start = time(NULL);
    iRC = NuSocketSend(Node, DATA, DATALEN);

	printf("send %d\n", iRC);
    return;
}

static void OnConnectCB_Svr(struct _NuSocketNode_t *Node, void *Argu) 
{
    printf("-->OnConnect\n");

    return;
}

static void OnDataArriveCB_Clnt(struct _NuSocketNode_t *Node, void *Argu)
{
    NodeData    *pNodeData = (NodeData *)Argu;
    char        Buf[DATALEN] = "\0";
	int         iCnt = 0;
	int i = 0;

    printf("-->OnDataArrive\n");
//    while((iCnt = NuSocketRecvInLen(Node, Buf, DATALEN, DATALEN)) > 0)
    while((iCnt = NuSocketRecvInTime(Node, Buf, DATALEN, 1)) > 0)
    {
printf("iCnt = %d, RecvCnt[%d]\n", iCnt, pNodeData->RecvCnt);
        if(++ (pNodeData->RecvCnt) >= SCALE)
        {
printf("Stop!\n");
            pNodeData->Stop = time(NULL);
            break;
        }
        else
        {
printf("Send!\n");
//            NuSocketSend(Node, DATA, DATALEN);
        }

		if (++i == 5)
		{
			printf("client break\n");
			break;
		}
    }

   	NuSocketSetRecvBuf(Node, Msg, DATALEN);

    return;
}

static void OnDataArriveCB_Svr(struct _NuSocketNode_t *Node, void *Argu)
{
    char    Buf[DATALEN] = "\0";
	int iRC = 0;
	int RecvCnt;
	int i = 0;

    printf("-->OnDataArrive\n");
    while( (RecvCnt = NuSocketRecvInLen(Node, Buf, DATALEN, DATALEN)) > 0)
    {
printf("RecvCnt[%d]\n", RecvCnt);
        iRC = NuSocketSend(Node, DATA, DATALEN);
		printf("reply %d\n", iRC);

		if (++i == 5)
		{
			break;
		}
    }

    NuSocketSetRecvBuf(Node, Msg, DATALEN);
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

static void OnDisconnectCB_Svr(struct _NuSocketNode_t *Node, void *Argu)
{
    printf("-->OnDisconnect\n");
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
    int Cnt = 0;

    printf("Usage: %s <SocketType> <TargetAddr> <TargetPort> <LocalAddr> <LocalPort> [Number] [WorkThreads]\n", ProgName);
    printf("   <SocketType>:\n");
    for(Cnt = 0; Cnt < TypeNo; ++ Cnt)
    {
        printf("    %s: %s\n", Type[Cnt].ShortName, Type[Cnt].Name);
    }

    return;
}

int main(int Argc, char **Argv)
{
    int                     Cnt = 0, No = DEFAULT_NO, WorkThreadNo = DEFAULT_THREADNO;
    double                  Average = 0;
    struct _NuSocket_t      *pSocket = NULL;
    SocketType              *pType = NULL;
    NuSocketProtocol_t      ProtoSvr, ProtoClnt, *pProto = &ProtoClnt;
    NuSocketNode_t          *Node = NULL;
    NodeData                *pNodeData = NULL;

    signal(SIGHUP, Signal);
    signal(SIGPIPE, Signal);

    if(Argc < 6)
    {
        Usage(Argv[0]);
        return 0;
    }
    else
    {
        for(Cnt = 0; Cnt < TypeNo; ++ Cnt)
        {
            if(!strcmp(Type[Cnt].ShortName, Argv[1]))
            {
                pType = &(Type[Cnt]);

                if(pType->ShortName[strlen(pType->ShortName) - 1] == 's')
                {
                    pProto = &ProtoSvr;
                }

                break;
            }
        }

        if(!pType)
        {
            Usage(Argv[0]);
            return 0;
        }

        if(Argc > 6)
        {
            No = strtol(Argv[6], (char **)NULL, 10);
        }

        if(Argc > 7)
        {
            WorkThreadNo = strtol(Argv[7], (char **)NULL, 10);
        }
    }

    pNodeData = (NodeData *)malloc(sizeof(NodeData) * No);

    if(NuSocketNew(&pSocket, WorkThreadNo, &OnLog) < 0)
    {
        printf("NuSocketNew Error!\n");
        return 0;
    }

    ProtoSvr.OnConnect = &OnConnectCB_Svr;
    ProtoSvr.OnDataArrive = &OnDataArriveCB_Svr;
    ProtoSvr.OnRemoteTimeout = &OnRemoteTimeoutCB;
    ProtoSvr.OnLocalTimeout = &OnLocalTimeoutCB;
    ProtoSvr.OnDisconnect = &OnDisconnectCB_Svr;

    ProtoClnt.OnConnect = &OnConnectCB_Clnt;
    ProtoClnt.OnDataArrive = &OnDataArriveCB_Clnt;
    ProtoClnt.OnRemoteTimeout = &OnRemoteTimeoutCB;
    ProtoClnt.OnLocalTimeout = &OnLocalTimeoutCB;
    ProtoClnt.OnDisconnect = &OnDisconnectCB_Clnt;

    for(Cnt = 0; Cnt < No; ++ Cnt)
    {
        if(!(Node = NuSocketAdd(pSocket, pType->Type, pProto, 
						Argv[2], strtol(Argv[3], (char **)NULL, 10), 
						Argv[4], strtol(Argv[5], (char **)NULL, 10), &(pNodeData[Cnt]))))
        {
            printf("Error::Cnt[%d] NuSocketAdd(TargetAddr[%s] TargetPort[%ld], LocalAddr[%s] LocalPort[%ld]) failed.\n", Cnt, Argv[2], strtol(Argv[3], (char **)NULL, 10), Argv[4], strtol(Argv[5], (char **)NULL, 10));

            Usage(Argv[0]);
            return 0;
        }

        NuSocketSetRecvBuf(Node, Msg, DATALEN);
    }

    printf("Let Rock & Roll with %s in %d instance(s) via %d threads!\n", pType->Name, No, WorkThreadNo);

    int MyCnt = 0;
    while(1)
    {
        sleep(1);
   
//		printf("No = %d, MyCnt = %d\n", No, MyCnt);
        for(Cnt = 0; Cnt < No; ++ Cnt)
        {
            if(pNodeData[Cnt].RecvCnt < SCALE)
            {
//		printf("RcvCnt < SCALE => %d < %d\n", pNodeData[Cnt].RecvCnt, SCALE);
            if(pNodeData[Cnt].RecvCnt < SCALE)
                break;
            }
        }

        if(Cnt >= No || MyCnt > 30)
        {
            break;
        }

        ++ MyCnt;
    };

    for(Cnt = 0; Cnt < No; ++ Cnt)
    {
        printf("%s(%s): [%d] RTT[%ld]\n", pType->ShortName, pType->Name, SCALE, pNodeData[Cnt].Stop - pNodeData[Cnt].Start);
    
        Average += (pNodeData[Cnt].Stop - pNodeData[Cnt].Start);
    }

    printf("Average: %f us\n", Average / No);

    NuSocketFree(pSocket);

    return 0;
}

