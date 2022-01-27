
#include <sys/epoll.h>

static void SetOffline(void *Argu)
{
    NuSocketNode_t      *Node = (NuSocketNode_t *)Argu;
    NuSocketTypeNode_t  *pTypeNode = &(Node->TypeNode);
    struct epoll_event  Event;

    if(NodeHas(Node, NodeConnect))
    {
        NodeSetPendingDisconnect(Node);
        epoll_ctl(Node->Socket->EpollFD, EPOLL_CTL_DEL, pTypeNode->InFD, &Event);

        shutdown(pTypeNode->InFD, SHUT_RDWR);
        shutdown(pTypeNode->OutFD, SHUT_RDWR);
        Node->LocalTimeoutCnt = 0;

        if((Node->Type->SetOffline(pTypeNode) == NuSocketTypeCBPass))
        {
            Node->Protocol->OnDisconnect(Node, pTypeNode->CBArgu);
        }
    }

    return;
}

static void AddToMonitor(NuSocketNode_t *Node)
{
    struct epoll_event  Event;

    if(NodeIsConnected(Node))
    {
        Event.events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLRDHUP | EPOLLONESHOT;
        Event.data.ptr = Node;
        if(epoll_ctl(Node->Socket->EpollFD, EPOLL_CTL_ADD, Node->TypeNode.InFD, &Event) < 0)
        {
            if(errno == EEXIST)
            {
                epoll_ctl(Node->Socket->EpollFD, EPOLL_CTL_MOD, Node->TypeNode.InFD, &Event);
            }
        }
    }

    return;
}

#include "NuSocket_Invoker.c"

static void WorkThreadOnDataArrive(void *Argu)
{
    NuSocketNode_t      *pNode = (NuSocketNode_t *)Argu;
    NuSocketTypeNode_t  *pTypeNode = &(pNode->TypeNode);
    int                 RC = 0;

    while((RC = pNode->Type->OnEvent(pTypeNode)) != NuSocketTypeCBError)
    {
        if(!NodeIsConnected(pNode))
        {
            break;
        }
        
        pNode->RemoteTimeoutCnt = 0;
        if(RC == NuSocketTypeCBPass)
        {
            pNode->Protocol->OnDataArrive(pNode, pTypeNode->CBArgu);
        }

        if(!NodeIsConnected(pNode))
        {
            break;
        }
    }

    AddToMonitor(pNode);

    return;
}

static int EpollWaitFn(void *Hdlr, void *Argu)
{
    NuSocket_t          *Socket = (NuSocket_t *)Hdlr;
    NuInvokerDelegate_t *Delegate = (NuInvokerDelegate_t *)Argu;
    struct epoll_event  Event;

    if(epoll_wait(Socket->EpollFD, &Event, 1, -1) > 0)
    {
        if(Event.events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
        {
            Delegate->Fn = &SetOffline;
        }
        else
        {
            Delegate->Fn = &WorkThreadOnDataArrive;
        }

        Delegate->Argu = Event.data.ptr;

        return NU_OK;
    }

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
            else if((++ pNode->LocalTimeoutCnt) >= pNode->LocalTimeout)
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
                if((++ pNode->RemoteTimeoutCnt) > pNode->RemoteTimeout)
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
        if(((*Socket)->EpollFD = epoll_create(10)) < 0)
        {
            NUGOTO(RC, NU_FAIL, EXIT);
        }
    
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
        Type.Block = &EpollWaitFn;
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

        if(Socket->EpollFD != 0)
        {
            close(Socket->EpollFD);
            Socket->EpollFD = 0;
        }

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

