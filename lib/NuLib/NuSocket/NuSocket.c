#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/uio.h>

#include "NuUtil.h"
#include "NuLock.h"
#include "NuTimer.h"
#include "NuSocket.h"

struct _NuSocketNode_t
{
    NuSocketTypeNode_t  TypeNode;
    NuSocketType_t      *Type;
    NuSocketProtocol_t  *Protocol;
    int                 Status;
    int                 RemoteTimeout;
    int                 RemoteTimeoutCnt;
    int                 LocalTimeout;
    int                 LocalTimeoutCnt;
    NuLock_t            Lock;
    NuSocket_t          *Socket;
};

struct _NuSocket_t
{
    int                 OnSvc;
    int                 EpollFD;
    base_vector_t       *NodeVec;
    NuInvokerGroup_t    *InvokerGroup;
    NuTimerEventHdlr_t  *TimerEvent;
    NuSocketNode_t      *InternalEventNode;
    NuLock_t            Lock;
    NuSocketLogCB       Log;
};

/* Status */
typedef enum _NuSocketStatusBit
{
    NuSocketStatusBitConnect = 0,
    NuSocketStatusBitPending,
    NuSocketStatusBitStatic,
    NuSocketStatusBitReconnect, 
	NuSocketStatusBitWork
} NuSocketStatusBit;

#define NodeConnect                     (1<<NuSocketStatusBitConnect)
#define NodePending                     (1<<NuSocketStatusBitPending)
#define NodeStatic                      (1<<NuSocketStatusBitStatic)
#define NodeReconnect                   (1<<NuSocketStatusBitReconnect)
#define NodeWork                        (1<<NuSocketStatusBitWork)

#define NodeHas(Node, NodeStatus)       (((Node)->Status) & (NodeStatus))
#define NodeSet(Node, NodeStatus)       (((Node)->Status) |= (NodeStatus))
#define NodeRemove(Node, NodeStatus)    (((Node)->Status) &= ~(NodeStatus))

#define NodeIsPendingConnect(Node)      (NodeHas(Node, NodeConnect) && NodeHas(Node, NodePending))
#define NodeIsConnected(Node)           (NodeHas(Node, NodeConnect) && !NodeHas(Node, NodePending))
#define NodeIsPendingDisconnect(Node)   (!NodeHas(Node, NodeConnect) && NodeHas(Node, NodePending))
#define NodeIsDisconnected(Node)        (!NodeHas(Node, NodeConnect) && !NodeHas(Node, NodePending))

#define NodeSetPendingConnect(Node)     NodeSet(Node, NodePending); NodeSet(Node, NodeConnect)
#define NodeSetConnected(Node)          NodeRemove(Node, NodePending); NodeSet(Node, NodeConnect)
#define NodeSetPendingDisconnect(Node)  NodeSet(Node, NodePending); NodeRemove(Node, NodeConnect)
#define NodeSetDisconnected(Node)       NodeRemove(Node, NodePending); NodeRemove(Node, NodeConnect)

static NuSocketNode_t *ExpandAvailableNodeAndGetOne(NuSocket_t *Socket, int ExpandCnt)
{
    NuSocketNode_t      *pNode = NULL, *pNode2 = NULL;

    pNode = pNode2 = (NuSocketNode_t *)malloc(sizeof(NuSocketNode_t) * ExpandCnt);
    memset(pNode, 0x00, sizeof(NuSocketNode_t) * ExpandCnt);

	NuLockLock(&(Socket->Lock));
    while(ExpandCnt --)
    {
        NuLockInit(&(pNode->Lock), &NuLockType_Mutex);
        NuStrNewPreAlloc(&(pNode->TypeNode.TargetAddr), 16);
        NuStrNewPreAlloc(&(pNode->TypeNode.LocalAddr), 16);
        NodeSetDisconnected(pNode);
        base_vector_push(Socket->NodeVec, pNode);
        ++ pNode;
    }
	NuLockUnLock(&(Socket->Lock));

    return pNode2;
}

static NuSocketNode_t *GetAvailableNode(NuSocket_t *Socket, int FD)
{
    base_vector_t       *Vec = Socket->NodeVec;
    base_vector_it      VecIt;
    NuSocketNode_t      *pNode = NULL;

    base_vector_it_set(VecIt, Vec);
    while(VecIt != base_vector_it_end(Vec))
    {
        pNode = (NuSocketNode_t *)(*VecIt);

        if(NodeIsDisconnected(pNode) && !NodeHas(pNode, NodeStatic))
        {
            break;
        }

        ++ VecIt;
    }

    if(VecIt == base_vector_it_end(Vec))
    {
        pNode = ExpandAvailableNodeAndGetOne(Socket, 10);
    }

    pNode->TypeNode.InFD = pNode->TypeNode.OutFD = FD;
    NuStrClear(pNode->TypeNode.TargetAddr);
    pNode->TypeNode.TargetPort = 0;

    pNode->TypeNode.RecvBuf.ptr = NULL;
    pNode->TypeNode.RecvBuf.Len = 0;
    pNode->TypeNode.RecvBuf.RecvLen = 0;

    NuStrClear(pNode->TypeNode.LocalAddr);
    pNode->TypeNode.LocalPort = 0;
    pNode->TypeNode.ByteLeftForRecv = 1;

    pNode->RemoteTimeout = pNode->LocalTimeout = 100000;
    pNode->RemoteTimeoutCnt = pNode->LocalTimeoutCnt = 0;
    pNode->Socket = Socket;

    return pNode;
}

static void Close(NuSocketTypeNode_t *TypeNode)
{
    if(TypeNode->InFD != 0)
    {
        close(TypeNode->InFD);
        TypeNode->InFD = 0;
    }

    if(TypeNode->OutFD != 0)
    {
        close(TypeNode->OutFD);
        TypeNode->OutFD = 0;
    }

    return;
}

static int CheckAddrValue(NuSocketType_t *Type, int Property, char *Addr)
{
    if(Type->Property & Property)
    {
        if(Addr != NULL)
        {
            if(*Addr != '\0')
            {
                return 1;
            }
        }

        return -1;
    }

    return 0;
}

static int CheckPortValue(NuSocketType_t *Type, int Property, int Port)
{
    if(Type->Property & Property)
    {
        if(Port > 0)
        {
            return 1;
        }

        return -1;
    }

    return 0;
}

#include "NuSocket_Default.c"

#ifdef __linux__

#ifdef _SELECT
#include "NuSocket_Select.c"
#else
#include "NuSocket_Epoll.c"
#endif

#else
#include "NuSocket_Select.c"
#endif

NuSocketTypeNode_t *NuSocketTypeNodeFork(NuSocketTypeNode_t *TypeNode, NuSocketType_t *Type, int FD)
{
    NuSocketNode_t      *pParrentNode = ((NuSocketNode_t *)TypeNode);
    NuSocket_t          *Socket = pParrentNode->Socket;
    NuSocketNode_t      *pNode = NULL;

    NuLockLock(&(Socket->Lock));

    pNode = GetAvailableNode(Socket, FD);
    NodeSetPendingConnect(pNode);

    pNode->Type = TypeCompletion(Type);
    pNode->Protocol = pParrentNode->Protocol;

    pNode->TypeNode.CBArgu = pParrentNode->TypeNode.CBArgu;

    NuLockUnLock(&(Socket->Lock));

    return &(pNode->TypeNode);
}

void NuSocketTypeLog(NuSocketTypeNode_t *TypeNode, char *Format, ...)
{
    NuSocketNode_t  *SocketNode = (NuSocketNode_t *)TypeNode;
    va_list         ArguList;
    char            SocketFormat[1024] = "\0";

    sprintf(SocketFormat, "%s(%s:%d, %s:%d): %s", SocketNode->Type->Name, NuStrGet(TypeNode->TargetAddr), TypeNode->TargetPort, NuStrGet(TypeNode->LocalAddr), TypeNode->LocalPort, Format);

    va_start(ArguList, Format);
    SocketNode->Socket->Log(SocketFormat, ArguList, TypeNode->CBArgu);
    va_end(ArguList);

    return;
}

void NuSocketSetArgu(NuSocketNode_t *Node, void *CBArgu)
{
    Node->TypeNode.CBArgu = CBArgu;
    return;
}

void NuSocketSetRecvBuf(NuSocketNode_t *Node, void *Buf, size_t BufLen)
{
    Node->TypeNode.RecvBuf.ptr = Buf;
    Node->TypeNode.RecvBuf.Len = BufLen;

    return;
}

int NuSocketGetRecvBufLen(NuSocketNode_t *Node)
{
    return Node->TypeNode.RecvBuf.RecvLen;
}

int NuSocketSend(NuSocketNode_t *Node, void *Msg, size_t MsgLen)
{
    char                *pSend = (void *)Msg;
    NuSocketTypeNode_t  *pTypeNode = &(Node->TypeNode);
    size_t              SendLen = 0;

    if(!NodeHas(Node, NodeConnect))
    {
        return NuSocketReturnOfflined;
    }

    NuLockLock(&(Node->Lock));
    while(MsgLen)
    {
        if((SendLen = Node->Type->Send(pTypeNode, pSend, MsgLen)) == NuSocketTypeCBError)
        {
            break;
        }
        else if(SendLen == NuSocketTypeCBPass)
        {
            continue;
        }

        MsgLen -= SendLen;
        pSend += SendLen;
    }

    Node->LocalTimeoutCnt = 0;
    NuLockUnLock(&(Node->Lock));

    return pSend - (char *)Msg;
}

int NuSocketRecvInLen(NuSocketNode_t *Node, void *Msg, size_t AtLeastLen, size_t AtMostLen)
{
    NuSocketTypeNode_t  *pTypeNode = &(Node->TypeNode);
    int                 RecvLen = 0;
    char                *pRecv = Msg;
    int                 PassCnt = 0;

    if(!NodeHas(Node, NodeConnect))
    {
        return NuSocketReturnOfflined;
    }

    do
    {
        switch(RecvLen = Node->Type->Recv(pTypeNode, pRecv, AtMostLen))
        {
        case NuSocketTypeCBError:
        case NuSocketTypeCBDone:
            AtLeastLen = 0;
        case NuSocketTypeCBPass:
            if((++ PassCnt) > 10)
            {
                sleep(0);
                PassCnt = 0;
            }
            break;
        default:
            pRecv += RecvLen;
            AtMostLen -= RecvLen;
            break;
        }
    }
    while(pRecv - (char *)Msg < AtLeastLen);

    pTypeNode->ByteLeftForRecv -= (RecvLen = pRecv - (char *)Msg);

    return RecvLen;
}

int NuSocketRecvInTime(NuSocketNode_t *Node, void *Msg, size_t NeedLen, int Sec)
{
    NuSocketTypeNode_t  *pTypeNode = &(Node->TypeNode);
    int                 RecvLen = 0;
    int                 Cnt = 0;
    char                *pRecv = Msg;
    time_t              Now = time(NULL);

    if(!NodeHas(Node, NodeConnect))
    {
        return NuSocketReturnOfflined;
    }

    while(NeedLen > 0)
    {
		switch(RecvLen = Node->Type->Recv(pTypeNode, pRecv, NeedLen))
		{
			case NuSocketTypeCBError:
				break;  /* not recv any data */
			case NuSocketTypeCBDone:
				NeedLen = 0;
				RecvLen = 0;
				break;
			case NuSocketTypeCBPass:
				break;
			default:
				pRecv += RecvLen;
				NeedLen -= RecvLen;
				break;
		}

		if((++ Cnt) > 30)
		{
			if(time(NULL) - Now >= Sec)
			{
				break;
			}

			sleep(0);
			Cnt = 0;
		}
    }

    RecvLen = pRecv - (char *)Msg;
    pTypeNode->ByteLeftForRecv -= RecvLen;

    return RecvLen;
}

int NuSocketRecvOneTime(NuSocketNode_t *Node, void *Msg, size_t NeedLen)
{
    NuSocketTypeNode_t  *pTypeNode = &(Node->TypeNode);
    int                 RecvLen = 0;
    char                *pRecv = Msg;

	if(!NodeHas(Node, NodeConnect))
	{
		return NuSocketReturnOfflined;
	}

	switch(RecvLen = Node->Type->Recv(pTypeNode, pRecv, NeedLen))
	{
		case NuSocketTypeCBError:
		case NuSocketTypeCBDone:
			NeedLen = 0;
			RecvLen = 0;
			break;
		case NuSocketTypeCBPass:
			break;
		default:
			pRecv += RecvLen;
			NeedLen -= RecvLen;
			break;
	}

	RecvLen = pRecv - (char *)Msg;
	pTypeNode->ByteLeftForRecv -= RecvLen;

	return RecvLen;
}

void NuSocketSetTimeout(NuSocketNode_t *Node, int RemoteTimeout, int LocalTimeout)
{
    if(RemoteTimeout > 0)
    {
        Node->RemoteTimeout = RemoteTimeout;
    }

    if(LocalTimeout > 0)
    {
        Node->LocalTimeout = LocalTimeout;
    }

    return;
}

void NuSocketSetAutoReconnect(NuSocketNode_t *Node, int AutoReconnect)
{
    if(AutoReconnect)
    {
        NodeSet(Node, NodeReconnect);
    }
    else
    {
        NodeRemove(Node, NodeReconnect);
    }

    return;
}

char *NuSocketGetAddr(NuSocketNode_t *Node)
{
    return (char *)NuStrGet(Node->TypeNode.TargetAddr);
}

int NuSocketGetPort(NuSocketNode_t *Node)
{
    return Node->TypeNode.TargetPort;
}

void NuSocketInvoke(NuSocket_t *Socket, NuInvokerDelegate_t *Delegate)
{
    NuInvokerInvoke(Socket->InvokerGroup, Delegate);

    return;
}

void NuSocketReconnect(NuSocketNode_t *Node)
{
    NodeSet(Node, NodeReconnect);
    SetOffline(Node);
    return;
}

void NuSocketDisconnect(NuSocketNode_t *Node)
{
    NodeRemove(Node, NodeReconnect);
    SetOffline(Node);
    return;
}

/* socket recv/send function for uio */
int NuSocketSendV(NuSocketNode_t *Node, struct iovec *iov, int iovCnt)
{
    NuSocketTypeNode_t  *pTypeNode = &(Node->TypeNode);
    size_t              SendLen = 0;

    if(!NodeHas(Node, NodeConnect))
    {
        return NuSocketReturnOfflined;
    }

    NuLockLock(&(Node->Lock));

	SendLen = Node->Type->SendV(pTypeNode, iov, iovCnt);

	if (SendLen > 0)
	{
		Node->LocalTimeoutCnt = 0;
	}

    NuLockUnLock(&(Node->Lock));

    return SendLen;
}

int NuSocketReadV_InLen(NuSocketNode_t *Node, struct iovec *iov, int iovCnt)
{
    NuSocketTypeNode_t  *pTypeNode = &(Node->TypeNode);
    int                 Cnt = 0;
	ssize_t             rtn = 0, len = 0;

	struct iovec *v = iov;
	int    n = iovCnt;

	while (n > 0)
	{
		if(!NodeHas(Node, NodeConnect))
		{
			return NuSocketReturnOfflined;
		}
		
		len = Node->Type->ReadV(pTypeNode, v, n);

		if (len > 0)
		{
			rtn += len;
			while(len > 0 && n > 0)
			{
				if (len < v->iov_len)
				{
					v->iov_len -= len;
					v->iov_base += len;
					rtn +=len;
					break;
				}
				else
				{
					len -= v->iov_len;
					v->iov_len = 0;

					--n;
					++v;
				}
			}
		}
		else if(len == 0 ||
				(len < 0 && errno != EAGAIN &&
				 errno != EWOULDBLOCK))
		{
			rtn = 0;
			goto EXIT;
		}
		else if(++ Cnt > 30)
		{
			sleep(0);
			Cnt = 0;
		}

	}

    pTypeNode->ByteLeftForRecv -= rtn;

EXIT:
    return rtn;
}

int NuSocketReadV_InTime(NuSocketNode_t *Node, struct iovec *iov, int iovCnt, int Sec)
{
    NuSocketTypeNode_t  *pTypeNode = &(Node->TypeNode);
    int                 Cnt = 0;
    time_t              Now = time(NULL);
	ssize_t             len = 0, rtn = 0;

	struct iovec *v = iov;
	int    n = iovCnt;

    if(!NodeHas(Node, NodeConnect))
    {
        return NuSocketReturnOfflined;
    }

	while (n > 0)
	{
		if(!NodeHas(Node, NodeConnect))
		{
			return NuSocketReturnOfflined;
		}
		
		len = Node->Type->ReadV(pTypeNode, v, n);

		if (len > 0)
		{
			rtn += len;
			while(len > 0 && n > 0)
			{
				if (len < v->iov_len)
				{
					v->iov_len -= len;
					v->iov_base += len;
					rtn +=len;
					break;
				}
				else
				{
					len -= v->iov_len;
					v->iov_len = 0;

					--n;
					++v;
				}
			}
		}
		else if(len == 0 ||
				(len < 0 && errno != EAGAIN &&
				 errno != EWOULDBLOCK))
		{
			rtn = 0;
			goto EXIT;
		}
		else if(++ Cnt > 30)
		{
			if(time(NULL) - Now >= Sec)
			{
				break;
			}

			sleep(0);
			Cnt = 0;
		}

	}

    pTypeNode->ByteLeftForRecv -= rtn;

EXIT:
    return rtn;
}

int NuSocketReadV_OneTime(NuSocketNode_t *Node, struct iovec *iov, int iovCnt)
{
    NuSocketTypeNode_t  *pTypeNode = &(Node->TypeNode);

	ssize_t n = 0;

	if(!NodeHas(Node, NodeConnect))
	{
		return NuSocketReturnOfflined;
	}

	n = Node->Type->ReadV(pTypeNode, iov, iovCnt);

	if (n > 0)
	{
		pTypeNode->ByteLeftForRecv -= n;
	}

	return n;
}

