
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "NuSocketType_InetStrmSvr.h"
#include "NuSocket.h"

#include "tData.h"

typedef struct _NodeArgu_t
{
	int RcvCnt;
	
} NodeArgu_t;

static void _OnConnect(NuSocketNode_t *Node, void *Argu)
{
	printf("[%s:%d] OnConnect\n", NuSocketGetAddr(Node), NuSocketGetPort(Node));
}

static void _OnDisconnect(NuSocketNode_t *Node, void *Argu)
{
	printf("[%s:%d] OnDisconnect\n", NuSocketGetAddr(Node), NuSocketGetPort(Node));
}

static void _OnRemoteTimeout(NuSocketNode_t *Node, void *Argu)
{
	printf("[%s:%d] Remote Timeout\n", NuSocketGetAddr(Node), NuSocketGetPort(Node));
}

static void _OnLocalTimeout(NuSocketNode_t *Node, void *Argu)
{
	printf("[%s:%d] Local Timeout\n", NuSocketGetAddr(Node), NuSocketGetPort(Node));
}

static void _OnDataArrived(NuSocketNode_t *Node, void *Argu)
{
	int iRC = 0;
	char Buf[1024] = {0};
	
	printf("[%s:%d] DataArrived\n", NuSocketGetAddr(Node), NuSocketGetPort(Node));

	while( (iRC = NuSocketRecvOneTime(Node, Buf, DATALEN)) > 0)
	{
		Buf[iRC] = 0x00;
		printf("[%s:%d] > %s\n", NuSocketGetAddr(Node), NuSocketGetPort(Node), Buf);
		
		switch(Buf[0])
		{
			case 'R':
    			iRC = NuSocketSend(Node, Buf, strlen(Buf));
				printf("[%s:%d:%d] < %s\n", NuSocketGetAddr(Node), NuSocketGetPort(Node), iRC, Buf);
				break;
			default:
				break;
		}
	}
		
}
static void Signal(int Sig)
{
    printf("Sig[%d]\n", Sig);
    return;
}

static void OnLog(char *Format, va_list ArguList, void *Argu)
{
    vprintf(Format, ArguList);
    return;
}

int main (int argc, char **argv)
{
	int  iRC = 0;
	NuSocket_t *pSocket = NULL;
    NuSocketProtocol_t Protocol;
	NuSocketNode_t *pSvrNode;
	NodeArgu_t NArgu;

	char *ptr = NULL;
	int  i = 0;
	char szListenIP[15+1] = {0};
	int  iListenPort = 0;
    signal(SIGHUP, Signal);
    signal(SIGPIPE, Signal);

	for (i = 0; i < argc; i++)
	{
		ptr = argv[i];

		if (!strcmp(ptr, "--ip"))
		{
			++i;
			strcpy(szListenIP, argv[i]);
		}
	}

	if (szListenIP[0] == 0x00 ||
	   iListenPort == 0)
	{
		return 0;
	}

	/* create socket object */
	iRC = NuSocketNew(&pSocket, 5, &OnLog);
	if (iRC < 0)
	{
		return 0;
	}

	/* setting protocol */
    Protocol.OnConnect = &_OnConnect;
    Protocol.OnDataArrive = &_OnDataArrived;
    Protocol.OnRemoteTimeout = &_OnRemoteTimeout;
    Protocol.OnLocalTimeout = &_OnLocalTimeout;
    Protocol.OnDisconnect = &_OnDisconnect;

	/* add svr */
	pSvrNode = NuSocketAdd(pSocket, &UnixStrmSvr, &Protocol, "", 0, szListenIP, iListenPort, &NArgu);
	if (pSvrNode == NULL)
	{
		printf("add server fail \n");
		return 0;
	}

	while(1)
	{
		sleep(1);
	}

}
