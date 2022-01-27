
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include "NuSocketType_UnixStrmClnt.h"
#include "NuSocket.h"

#include "tData.h"

typedef struct _NodeArgu_t
{
	int Status;
	int RcvCnt;
} NodeArgu_t;

static void _OnConnect(NuSocketNode_t *Node, void *Argu)
{
	NodeArgu_t *pArgu = (NodeArgu_t *)Argu;
	pArgu->Status = 1;

	printf("[%s:%d] OnConnect\n", NuSocketGetAddr(Node), NuSocketGetPort(Node));
}

static void _OnDisconnect(NuSocketNode_t *Node, void *Argu)
{
	NodeArgu_t *pArgu = (NodeArgu_t *)Argu;
	pArgu->Status = 0;

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
	
	printf("[%s:%d] DataArrived %ld\n", NuSocketGetAddr(Node), NuSocketGetPort(Node), pthread_self());

	while( (iRC = NuSocketRecvOneTime(Node, Buf, DATALEN)) > 0)
	{
		Buf[iRC] = 0x00;
		printf("[%s:%d] > %s\n", NuSocketGetAddr(Node), NuSocketGetPort(Node), Buf);
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
	NuSocketNode_t *pClntNode;
	NodeArgu_t NArgu;

	char *ptr = NULL;
	int  i = 0;
	char szRemoteIP[15+1] = {0};
	int  iRemotePort = 0;

    signal(SIGHUP, Signal);
    signal(SIGPIPE, Signal);

	for (i = 0; i < argc; i++)
	{
		ptr = argv[i];

		if (!strcmp(ptr, "--ip"))
		{
			++i;
			strcpy(szRemoteIP, argv[i]);
		}
	}

	if (szRemoteIP[0] == 0x00)
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

	NArgu.Status = 0;
	/* add svr */
	pClntNode = NuSocketAdd(pSocket, &UnixStrmClnt, &Protocol, szRemoteIP, iRemotePort, "", 0, &NArgu);
	if (pClntNode == NULL)
	{
		printf("add client fail \n");
		return 0;
	}

	while(1)
	{
		sleep(1);

		if (NArgu.Status <= 0)
		{
			continue;
		}
//printf("prepare send \n");
		if (time(NULL) % 2 == 0)
		{
//printf("prepare send 1\n");
			iRC = NuSocketSend(pClntNode, DATAR, DATARLEN);
		}
		else
		{
//printf("prepare send 2\n");
			iRC = NuSocketSend(pClntNode, DATA, DATALEN);
		}
		printf("send len[%d]\n", iRC);
		if (iRC < 0)
		{
			return 0;
		}
	}

}
