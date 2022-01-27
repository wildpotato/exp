#include <pthread.h>
typedef struct _SelectArgu_t
{
	fd_set  Read;
	fd_set  Error;
	int     Cnt;
	void    *Argu;
} SelectArgu_t;

static void SetOffline(void *Argu)
{
    NuSocketNode_t      *pNode = (NuSocketNode_t *)Argu;
    NuSocketTypeNode_t  *pTypeNode = &(pNode->TypeNode);

    if(NodeHas(pNode, NodeConnect))
    {
        NodeSetPendingDisconnect(pNode);

        shutdown(pTypeNode->InFD, SHUT_RDWR);
        shutdown(pTypeNode->OutFD, SHUT_RDWR);
        pNode->LocalTimeoutCnt = 0;

        if((pNode->Type->SetOffline(pTypeNode) == NuSocketTypeCBPass))
        {
            pNode->Protocol->OnDisconnect(pNode, pTypeNode->CBArgu);
        }
    }

	NodeRemove(pNode, NodeWork);

    return;
}

static void AddToMonitor(NuSocketNode_t *Node)
{
    if(NodeIsConnected(Node))
    {
		NodeRemove(Node, NodeWork);
    }

    return;
}

#include "NuSocket_Invoker.c"

static void WorkThreadOnDataArrive(void *Argu)
{
    NuSocketNode_t      *pNode = (NuSocketNode_t *)Argu;
    NuSocketTypeNode_t  *pTypeNode = &(pNode->TypeNode);
    int                 iRC = 0;
	int                 iCnt = 0;

	do 
    {
		iRC = pNode->Type->OnEvent(pTypeNode);
		if (iRC == NuSocketTypeCBError)
		{
			if (iCnt == 0)
			{
				SetOffline(pNode);
				return;
			}
			else
			{
				break;
			}
		}

        if(!NodeIsConnected(pNode))
        {
            break;
        }

        pNode->RemoteTimeoutCnt = 0;
        if(iRC == NuSocketTypeCBPass)
        {
            pNode->Protocol->OnDataArrive(pNode, pTypeNode->CBArgu);
        }

        if(!NodeIsConnected(pNode))
        {
            break;
        }

		++iCnt;
    }while(iCnt < 5);

    AddToMonitor(pNode);

    return;
}

static int _NodeVecForeach_SetFD(void *Item, void *Argu)
{
    NuSocketNode_t *pNode = (NuSocketNode_t *)Item;
	SelectArgu_t   *pObj = (SelectArgu_t *)Argu;

	if (NodeIsConnected(pNode) && !NodeHas(pNode, NodeWork))
	{
		FD_SET(pNode->TypeNode.InFD, &(pObj->Read));
		FD_SET(pNode->TypeNode.InFD, &(pObj->Error));
		++(pObj->Cnt);
	}

	return NU_OK;
}

static int _NodeVecForeach_DoWork(void *Item, void *Argu)
{
    NuSocketNode_t *pNode = (NuSocketNode_t *)Item;
	SelectArgu_t   *pObj = (SelectArgu_t *)Argu;
    NuInvokerDelegate_t *Delegate = (NuInvokerDelegate_t *)(pObj->Argu);

	if (FD_ISSET(pNode->TypeNode.InFD, &(pObj->Error)) && !NodeHas(pNode, NodeWork))
	{
		NodeSet(pNode, NodeWork);

        Delegate->Fn = &SetOffline;
        Delegate->Argu = pNode;

		return NU_FAIL;
	}
	else if (FD_ISSET(pNode->TypeNode.InFD, &(pObj->Read)) && !NodeHas(pNode, NodeWork))
	{
		NodeSet(pNode, NodeWork);

		Delegate->Fn = &WorkThreadOnDataArrive;
        Delegate->Argu = pNode;

		return NU_FAIL;
	}
	return NU_OK;
}

static int SelectWaitFn(void *Hdlr, void *Argu) /* Argu = Invoker delegate */
{
	int                 iRC = 0;
    NuSocket_t          *Socket = (NuSocket_t *)Hdlr;
	SelectArgu_t        fd_argu;
	struct timeval      Cycle = { .tv_sec = 1, 
		                          .tv_usec = 0
								}; 

	FD_ZERO(&(fd_argu.Read));
	FD_ZERO(&(fd_argu.Error));
	fd_argu.Cnt = 0;
	fd_argu.Argu = Argu;

	NuLockLock(&(Socket->Lock));
	base_vector_foreach2(Socket->NodeVec, &_NodeVecForeach_SetFD, &fd_argu);
	NuLockUnLock(&(Socket->Lock));

	if (fd_argu.Cnt > 0 && 
		select(FD_SETSIZE, &(fd_argu.Read), NULL, &(fd_argu.Error), &Cycle) > 0 )
	{
		NuLockLock(&(Socket->Lock));
		iRC = base_vector_foreach2(Socket->NodeVec, &_NodeVecForeach_DoWork, &fd_argu);
		NuLockUnLock(&(Socket->Lock));

		if (iRC == NU_OK)
		{
			goto EXIT;
		}

		/* execute delegate */
		return NU_OK;
	}
	else
	{
		sleep(1); /* avoid busy loop call */
	}

EXIT:
	/* back to thread pool */
    return NU_FAIL;
}

static void FunctionPerSec(void *Argu)
{
    NuSocket_t              *Socket = (NuSocket_t *)Argu;
    NuSocketNode_t          *pNode = NULL;
    NuSocketTypeNode_t      *pTypeNode = NULL;
    NuSocketProtocol_t      *pProtocol = NULL;
    base_vector_t           *Vec = Socket->NodeVec;
    base_vector_it          VecIt;

    NuLockLock(&(Socket->Lock));

    base_vector_it_set(VecIt, Vec);
    while(VecIt != base_vector_it_end(Vec))
    {
        pNode = (NuSocketNode_t *)(*VecIt);
        pTypeNode = &(pNode->TypeNode);
        pProtocol = pNode->Protocol;

        if(NodeIsPendingDisconnect(pNode))
        {
            if(NodeHas(pNode, NodeStatic) && NodeHas(pNode, NodeReconnect))
            {
                Close(pTypeNode);

				switch(pNode->Type->SetOnline(pTypeNode))
				{
					case NuSocketTypeCBPass:
						NodeSetConnected(pNode);
						pNode->Protocol->OnConnect(pNode, pTypeNode->CBArgu);
						AddToMonitor(pNode);
						break;
					case NuSocketTypeCBDone:
						NodeSetConnected(pNode);
						AddToMonitor(pNode);
						break;
					default:
						break;
				}
            }
            else if((++(pNode->LocalTimeoutCnt)) >= 5) /* over 5 sec, set node be reuse */
            {
                Close(pTypeNode);
                NodeSetDisconnected(pNode);
            }
        }
        else if(NodeIsPendingConnect(pNode))
        {
            NodeSetConnected(pNode);
            pNode->Protocol->OnConnect(pNode, pTypeNode->CBArgu);
            AddToMonitor(pNode);
        }
        else if(NodeIsConnected(pNode))
        {
            if(!(pNode->Type->Property & NuSocketTypeProperty_NoOnTimeout))
            {
                if((++(pNode->RemoteTimeoutCnt)) > pNode->RemoteTimeout)
                {
                    Invoke(pProtocol->OnRemoteTimeout, pNode);
                    pNode->RemoteTimeoutCnt = 0;
                }

                if((++ pNode->LocalTimeoutCnt) > pNode->LocalTimeout)
                {
                    Invoke(pProtocol->OnLocalTimeout, pNode);
                    pNode->LocalTimeoutCnt = 0;
                }
            }
        }
        else
        { /* Disconnected */
        }

        ++ VecIt;
    }

    NuLockUnLock(&(Socket->Lock));

    return;
}

int NuSocketNew(NuSocket_t **Socket, int ThreadNo, NuSocketLogCB LogFn)
{
    int                 RC = NU_OK;
    NuBlockingType_t    Type;

    *Socket = (NuSocket_t *)calloc(sizeof(NuSocket_t), 1);
    if(*Socket != NULL)
    {
        (*Socket)->EpollFD = 0;
    
        if(NuLockInit(&((*Socket)->Lock), &NuLockType_Mutex) < 0)
        {
            NUGOTO(RC, NU_FAIL, EXIT);
        }

        if(base_vector_new(&((*Socket)->NodeVec), 10) < 0)
        {
            NUGOTO(RC, NU_FAIL, EXIT);
        }
        ExpandAvailableNodeAndGetOne(*Socket, 10);

        (*Socket)->OnSvc = 1;
		
        Type.Init = NULL;
        Type.Destroy = NULL;
        Type.Block = &SelectWaitFn;
        Type.Wake = &ExternalInvokeFn;
        Type.WakeAll = &ExternalInvokeFn;
        (*Socket)->InvokerGroup = NuInvokerAddGroup(&Type, *Socket, ThreadNo, ThreadNo * 2);

        (*Socket)->TimerEvent = NuTimerRegister(1, 1, &FunctionPerSec, *Socket);

        /* InternalEventStart */
        (*Socket)->InternalEventNode = NuSocketAdd(*Socket, &Pipe, &Invoker, "\0", 0, "\0", 0, NULL);
        if(!((*Socket)->Log = LogFn))
        {
            (*Socket)->Log = &DefaultLogCB;
        }
    }

EXIT:
    if(RC < 0)
    {
        NuSocketFree(*Socket);
    }

    return RC;
}

void NuSocketFree(NuSocket_t *Socket)
{
    base_vector_t       *Vec = Socket->NodeVec;
    base_vector_it      VecIt;
    NuSocketNode_t      *pNode = NULL;

    if(Socket != NULL)
    {
        Socket->OnSvc = 0;
        sleep(3);

        if(Socket->TimerEvent != NULL)
        {
            NuTimerUnregister(Socket->TimerEvent);
            Socket->TimerEvent = NULL;
        }

        NuInvokerRemove(Socket->InvokerGroup);

        if(Socket->NodeVec != NULL)
        {
            base_vector_it_set(VecIt, Vec = Socket->NodeVec);
            while(VecIt != base_vector_it_end(Vec))
            {
                pNode = (NuSocketNode_t *)(*VecIt);
                NuSocketDisconnect(pNode);
                NuLockDestroy(&(pNode->Lock));

                ++ VecIt;
            }

            base_vector_free(Socket->NodeVec);
            Socket->NodeVec = NULL;
        }

        NuLockDestroy(&(Socket->Lock));

        free(Socket);
    }

    return;
}

NuSocketNode_t *NuSocketAdd(NuSocket_t *Socket, NuSocketType_t *Type, NuSocketProtocol_t *Protocol, char *TargetAddr, int TargetPort, char *LocalAddr, int LocalPort, void *CBArgu)
{
    NuSocketNode_t      *pNode = NULL;
    NuSocketTypeNode_t  *pTypeNode = NULL;

    if(!Type)
    {
        return NULL;
    }

    if(CheckAddrValue(Type, NuSocketTypeProperty_TargetAddr, TargetAddr) < 0)
    {
        return NULL;
    }

    if(CheckPortValue(Type, NuSocketTypeProperty_TargetPort, TargetPort) < 0)
    {
        return NULL;
    }

    if(CheckAddrValue(Type, NuSocketTypeProperty_LocalAddr, LocalAddr) < 0)
    {
        return NULL;
    }

    if(CheckPortValue(Type, NuSocketTypeProperty_LocalPort, LocalPort) < 0)
    {
        return NULL;
    }

    NuLockLock(&(Socket->Lock));
    pNode = GetAvailableNode(Socket, 0);
    pTypeNode = &(pNode->TypeNode);

    if(TargetAddr != NULL)
    {
        NuStrCpy(pTypeNode->TargetAddr, TargetAddr);
    }
    pTypeNode->TargetPort = TargetPort;

    if(LocalAddr != NULL)
    {
        NuStrCpy(pTypeNode->LocalAddr, LocalAddr);
    }
    pTypeNode->LocalPort = LocalPort;

    pTypeNode->CBArgu = CBArgu;

    pNode->Type = TypeCompletion(Type);
    pNode->Protocol = ProtocolCompletion(Protocol);
    NodeSetPendingDisconnect(pNode);
    NodeSet(pNode, NodeStatic);
    NodeSet(pNode, NodeReconnect);
    NuLockUnLock(&(Socket->Lock));

    return pNode;
}

