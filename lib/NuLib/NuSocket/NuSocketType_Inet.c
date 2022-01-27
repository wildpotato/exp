
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <netdb.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include "NuSocketType.h"
#include "NuSocketType_InetStrmSvr.h"
#include "NuSocketType_InetStrmClnt.h"
#include "NuSocketType_InetDgrmSvr.h"
#include "NuSocketType_InetDgrmClnt.h"
#include "NuSocketType_InetMcstSvr.h"
#include "NuSocketType_InetMcstClnt.h"

static int _InetMcstClntTemplate(NuSocketTypeNode_t *TypeNode, int SocketDomain, int SocketType)
{
    int                 FD = 0, Flag = 1, RC = -1;
    struct addrinfo     Hint;
    struct addrinfo     *BindResult = NULL, *pAddr = NULL;
    char                Port[16] = "\0";
    char                *pMemberIP;

    if(SocketDomain != AF_INET && SocketDomain != AF_INET6)
    {
        NuSocketTypeLog(TypeNode, "Unsupport socket domain(%d)\n", SocketDomain);
        goto EXIT;
    }

    if(SocketType != SOCK_STREAM && SocketType != SOCK_DGRAM)
    {
        NuSocketTypeLog(TypeNode, "Unsupport socket type(%d)\n", SocketType);
        goto EXIT;
    }

    if((FD = socket(SocketDomain, SocketType, 0)) < 0)
    {
        NuSocketTypeLog(TypeNode, "socket(%s)\n", strerror(errno));
        goto EXIT;
    }

    Flag = 1;
    setsockopt(FD, SOL_SOCKET, SO_REUSEADDR, &Flag, sizeof(Flag));

    if(SocketType == SOCK_STREAM)
    {
        Flag = 1;
        setsockopt(FD, IPPROTO_TCP, TCP_NODELAY, &Flag, sizeof(Flag));

#ifdef __linux__
        Flag = 0;
        setsockopt(FD, IPPROTO_TCP, TCP_CORK, &Flag, sizeof(Flag));
#endif
    }

    if(TypeNode->LocalPort > 0)
    {
        sprintf(Port, "%d", TypeNode->LocalPort);

        memset(&Hint, 0, sizeof(Hint));
        Hint.ai_family = SocketDomain;
        Hint.ai_socktype = SocketType;
        Hint.ai_flags = AI_PASSIVE;

        if (NuStrSize(TypeNode->TargetAddr) > 0)
        {
            pMemberIP = (char *)NuStrGet(TypeNode->TargetAddr);
        }
        else
        {
            pMemberIP = NULL;
        }

        if((Flag = getaddrinfo(pMemberIP, Port, &Hint, &BindResult)) != 0)
        {
            NuSocketTypeLog(TypeNode, "getaddrinfo for bind(%s)\n", gai_strerror(Flag));

            goto EXIT;
        }

        pAddr = BindResult;
        while(pAddr != NULL)
        {
            if(bind(FD, pAddr->ai_addr, pAddr->ai_addrlen) == 0)
            {
                break;
            }

            pAddr = pAddr->ai_next;
        }

        if(!pAddr)
        {
            NuSocketTypeLog(TypeNode, "bind(%s)\n", strerror(errno));

            goto EXIT;
        }
    }

    RC = 1;

EXIT:
    if(BindResult != NULL)
    {
        freeaddrinfo(BindResult);
    }

    if(RC < 0)
    {
        if(FD > 0)
        {
            close(FD);
            FD = 0;
        }

        return NuSocketTypeCBError;
    }

    Flag = fcntl(FD, F_GETFL, 0);
    fcntl(FD, F_SETFL, Flag | O_NONBLOCK);

    TypeNode->InFD = TypeNode->OutFD = FD;

    return NuSocketTypeCBPass;
}
static int _InetTemplate(NuSocketTypeNode_t *TypeNode, int SocketDomain, int SocketType)
{
    int                 FD = 0, Flag = 1, RC = -1;
    struct addrinfo     Hint;
    struct addrinfo     *BindResult = NULL, *ConnectResult = NULL, *pAddr = NULL;
    char                Port[16] = "\0";

    if(SocketDomain != AF_INET && SocketDomain != AF_INET6)
    {
        NuSocketTypeLog(TypeNode, "Unsupport socket domain(%d)\n", SocketDomain);
        goto EXIT;
    }

    if(SocketType != SOCK_STREAM && SocketType != SOCK_DGRAM)
    {
        NuSocketTypeLog(TypeNode, "Unsupport socket type(%d)\n", SocketType);
        goto EXIT;
    }

    if((FD = socket(SocketDomain, SocketType, 0)) < 0)
    {
        NuSocketTypeLog(TypeNode, "socket(%s)\n", strerror(errno));
        goto EXIT;
    }

    Flag = 1;
    setsockopt(FD, SOL_SOCKET, SO_REUSEADDR, &Flag, sizeof(Flag));

    if(SocketType == SOCK_STREAM)
    {
        Flag = 1;
        setsockopt(FD, IPPROTO_TCP, TCP_NODELAY, &Flag, sizeof(Flag));

#ifdef __linux__
        Flag = 0;
        setsockopt(FD, IPPROTO_TCP, TCP_CORK, &Flag, sizeof(Flag));
#endif
    }

    if(TypeNode->LocalPort > 0)
    {
        sprintf(Port, "%d", TypeNode->LocalPort);

        memset(&Hint, 0, sizeof(Hint));
        Hint.ai_family = SocketDomain;
        Hint.ai_socktype = SocketType;
        Hint.ai_flags = AI_PASSIVE;

        if((Flag = getaddrinfo(NULL, Port, &Hint, &BindResult)) != 0)
        {
            NuSocketTypeLog(TypeNode, "getaddrinfo for bind(%s)\n", gai_strerror(Flag));

            goto EXIT;
        }

        pAddr = BindResult;
        while(pAddr != NULL)
        {
            if(bind(FD, pAddr->ai_addr, pAddr->ai_addrlen) == 0)
            {
                break;
            }

            pAddr = pAddr->ai_next;
        }

        if(!pAddr)
        {
            NuSocketTypeLog(TypeNode, "bind(%s)\n", strerror(errno));

            goto EXIT;
        }
    }

    if(TypeNode->TargetPort > 0)
    {
        sprintf(Port, "%d", TypeNode->TargetPort);
        memset(&Hint, 0, sizeof(Hint));

        Hint.ai_family = SocketDomain;
        Hint.ai_socktype = SocketType;

        if((Flag = getaddrinfo(NuStrGet(TypeNode->TargetAddr), Port, &Hint, &ConnectResult)) != 0)
        {
            NuSocketTypeLog(TypeNode, "getaddrinfo(%s)\n", gai_strerror(Flag));

            goto EXIT;
        }

        pAddr = ConnectResult;
        while(pAddr != NULL)
        {
            if(connect(FD, pAddr->ai_addr, pAddr->ai_addrlen) != -1)
            {
                break;
            }

            pAddr = pAddr->ai_next;
        }

        if(!pAddr)
        {
            NuSocketTypeLog(TypeNode, "connect(%s)\n", strerror(errno));
            goto EXIT;
        }
    }

    RC = 1;

EXIT:
    if(BindResult != NULL)
    {
        freeaddrinfo(BindResult);
    }

    if(ConnectResult != NULL)
    {
        freeaddrinfo(ConnectResult);
    }

    if(RC < 0)
    {
        if(FD > 0)
        {
            close(FD);
            FD = 0;
        }

        return NuSocketTypeCBError;
    }

    Flag = fcntl(FD, F_GETFL, 0);
    fcntl(FD, F_SETFL, Flag | O_NONBLOCK);

    TypeNode->InFD = TypeNode->OutFD = FD;

    return NuSocketTypeCBPass;
}

/* Denied Operation */
static int _Inet_Denied(NuSocketTypeNode_t *TypeNode, void *Msg, size_t MsgLen)
{
    return NuSocketTypeCBDone;
}

static int _Inet_Denied_V(NuSocketTypeNode_t *TypeNode, struct iovec *iov, int iovCnt)
{
    return NuSocketTypeCBDone;
}

/* DgrmClnt */
static int _DgrmClnt_SetOnline(NuSocketTypeNode_t *TypeNode)
{
    return _InetTemplate(TypeNode, AF_INET, SOCK_DGRAM);
}

NuSocketType_t InetDgrmClnt = {"InetDgrmClnt", NuSocketTypeProperty_TargetAddr|NuSocketTypeProperty_TargetPort, &_DgrmClnt_SetOnline, NULL, NULL, NULL, NULL, NULL, NULL};

/* DgrmSvr */
static int _DgrmSvr_SetOnline(NuSocketTypeNode_t *TypeNode)
{
    return _InetTemplate(TypeNode, AF_INET, SOCK_DGRAM);
}

static int _DgrmSvr_Send(NuSocketTypeNode_t *TypeNode, void *Msg, size_t MsgLen)
{
    NuSocketType_InetDgrmSvrMsg *pMsg = (NuSocketType_InetDgrmSvrMsg *)Msg;
    int                         WriteLen = NuSocketTypeCBError;
    int                         FD = TypeNode->OutFD;

    if(FD > 0)
    {
        do
        {
            if((WriteLen = sendto(FD, pMsg->Msg, MsgLen, 0, (struct sockaddr *)&(pMsg->Addr), sizeof(pMsg->Addr))) <= 0)
            {
                if(errno == EINTR)
                {
                    continue;
                }
                else if(errno == EWOULDBLOCK || errno == EAGAIN)
                {
                    WriteLen = NuSocketTypeCBPass;
                }
                else
                {
                    WriteLen = NuSocketTypeCBError;
                }
            }
        }
        while(0);
    }

    return WriteLen;
}

static int _DgrmSvr_Recv(NuSocketTypeNode_t *TypeNode, void *Msg, size_t MsgLen)
{
    int                         ReadLen = NuSocketTypeCBError;
    int                         FD = TypeNode->InFD;
    NuSocketType_InetDgrmSvrMsg *pMsg = (NuSocketType_InetDgrmSvrMsg *)Msg;
    socklen_t                   SockLen = sizeof(pMsg->Addr);

    if(FD > 0)
    {
        do
        {
            if((ReadLen = recvfrom(FD, pMsg->Msg, MsgLen, 0, (struct sockaddr *)&(pMsg->Addr), &SockLen)) < 0)
            {
                if(errno == EINTR)
                {
                    continue;
                }
                else if(errno == EWOULDBLOCK || errno == EAGAIN)
                {
                    ReadLen = NuSocketTypeCBPass;
                }
                else
                {
                    ReadLen = NuSocketTypeCBError;
                }
            }
            else if(ReadLen == 0)
            {
                ReadLen = NuSocketTypeCBError;
            }
        }
        while(0);
    }

    return ReadLen;
}

NuSocketType_t InetDgrmSvr = {"InetDgrmSvr", NuSocketTypeProperty_LocalPort|NuSocketTypeProperty_NoOnTimeout, &_DgrmSvr_SetOnline, NULL, NULL, &_DgrmSvr_Send, &_DgrmSvr_Recv, NULL, NULL};

/* McstClnt */
static int _McstClnt_SetOnline(NuSocketTypeNode_t *TypeNode)
{
    struct ip_mreq      MReq;
    int                 RC = NuSocketTypeCBError;

    if(!(TypeNode->RecvBuf.ptr))
    {
        NuSocketTypeLog(TypeNode, "RecvBuf.ptr == NULL\n");
        return NuSocketTypeCBError;
    }

    if((RC = _InetMcstClntTemplate(TypeNode, AF_INET, SOCK_DGRAM)) == NuSocketTypeCBError)
    //if((RC = _InetTemplate(TypeNode, AF_INET, SOCK_DGRAM)) == NuSocketTypeCBError)
    {
        return RC;
    }

    memset(&MReq, 0, sizeof(struct ip_mreq));
    MReq.imr_multiaddr.s_addr = inet_addr(NuStrGet(TypeNode->TargetAddr));
    MReq.imr_interface.s_addr = inet_addr(NuStrGet(TypeNode->LocalAddr));

    if(setsockopt(TypeNode->InFD, IPPROTO_IP, IP_ADD_MEMBERSHIP, &MReq, sizeof(struct ip_mreq)) < 0)
    {
        NuSocketTypeLog(TypeNode, "setsockopt(%s)\n", strerror(errno));
        close(TypeNode->InFD);
        TypeNode->InFD = TypeNode->OutFD = 0;
        return NuSocketTypeCBError;
    }

    return NuSocketTypeCBPass;
}

static int _McstClnt_OnEvent(NuSocketTypeNode_t *TypeNode)
{
    int                 Cnt = 0;
    NuSocketRecvBuf_t   *pRecvBuf = &(TypeNode->RecvBuf);

    while((pRecvBuf->RecvLen = read(TypeNode->InFD, pRecvBuf->ptr, pRecvBuf->Len)) <= 0)
    {
        if(pRecvBuf->RecvLen == 0 || errno == EAGAIN || errno == EWOULDBLOCK)
        {
            if((++ Cnt) > 10)
            {
                sleep(0);
                Cnt = 0;
            }
        }
        else
        {
            return NuSocketTypeCBError;
        }
    }

    return NuSocketTypeCBPass;
}

NuSocketType_t InetMcstClnt = {"InetMcstClnt", NuSocketTypeProperty_TargetAddr|NuSocketTypeProperty_LocalAddr|NuSocketTypeProperty_LocalPort, &_McstClnt_SetOnline, NULL, &_McstClnt_OnEvent, &_Inet_Denied, &_Inet_Denied, &_Inet_Denied_V, &_Inet_Denied_V};

/* McstSvr */
static int _McstSvr_SetOnline(NuSocketTypeNode_t *TypeNode)
{
    char            ChFlag = 0;
    struct in_addr  LocalInterface;
    int             RC = _InetTemplate(TypeNode, AF_INET, SOCK_DGRAM);
 
    if(RC == NuSocketTypeCBError)
    {
        return RC;
    }

    setsockopt(TypeNode->InFD, IPPROTO_IP, IP_MULTICAST_LOOP, &ChFlag, sizeof(ChFlag));

    LocalInterface.s_addr = inet_addr(NuStrGet(TypeNode->LocalAddr));

    if(setsockopt(TypeNode->InFD, IPPROTO_IP, IP_MULTICAST_IF, &LocalInterface, sizeof(LocalInterface)) < 0)
    {
        NuSocketTypeLog(TypeNode, "setsockopt(%s)\n", strerror(errno));
        close(TypeNode->InFD);
        TypeNode->InFD = TypeNode->OutFD = 0;
        return NuSocketTypeCBError;
    }

    return NuSocketTypeCBPass;
}

NuSocketType_t InetMcstSvr = {"InetMcstSvr", NuSocketTypeProperty_TargetAddr|NuSocketTypeProperty_TargetPort|NuSocketTypeProperty_LocalAddr, &_McstSvr_SetOnline, NULL, NULL, NULL, &_Inet_Denied, NULL, &_Inet_Denied_V};

/* StrmClnt */
static int _StrmClnt_SetOnline(NuSocketTypeNode_t *TypeNode)
{
    return _InetTemplate(TypeNode, AF_INET, SOCK_STREAM);
}

NuSocketType_t InetStrmClnt = {"InetStrmClnt", NuSocketTypeProperty_TargetAddr|NuSocketTypeProperty_TargetPort, &_StrmClnt_SetOnline, NULL, NULL, NULL, NULL, NULL, NULL};

/* StrmSvr */
static NuSocketType_t InetStrmSvr_Clnt = {"InetStrmSvr_Clnt", 0, NULL, NULL, NULL, NULL, NULL};

static int _StrmSvr_SetOnline(NuSocketTypeNode_t *TypeNode)
{
    int RC = _InetTemplate(TypeNode, AF_INET, SOCK_STREAM);

    if(RC == NuSocketTypeCBError)
    {
        return RC;
    }

    if(listen(TypeNode->InFD, 10) < 0)
    {
        NuSocketTypeLog(TypeNode, "listen(%s)\n", strerror(errno));
        close(TypeNode->InFD);
        TypeNode->InFD = TypeNode->OutFD = 0;
        return NuSocketTypeCBError;
    }

    return NuSocketTypeCBDone;
}

static int _StrmSvr_OnEvent(NuSocketTypeNode_t *TypeNode)
{
    struct sockaddr_in          ClntAddr;
    socklen_t                   SockLen = sizeof(ClntAddr);
    int                         FD = 0, Flag = 1;
    NuSocketTypeNode_t          *pTypeNode = NULL;

    if((FD = accept(TypeNode->InFD, (struct sockaddr *)&ClntAddr, &SockLen)) > 0)
    {
        Flag = 1;
        setsockopt(FD, SOL_SOCKET, SO_REUSEADDR, &Flag, sizeof(Flag));
        setsockopt(FD, IPPROTO_TCP, TCP_NODELAY, &Flag, sizeof(Flag));

#ifdef __linux__
        Flag = 0;
        setsockopt(FD, IPPROTO_TCP, TCP_CORK, &Flag, sizeof(Flag));
#endif

        Flag = fcntl(FD, F_GETFL, 0);
        fcntl(FD, F_SETFL, Flag | O_NONBLOCK);

        pTypeNode = NuSocketTypeNodeFork(TypeNode, &InetStrmSvr_Clnt, FD);

        NuStrCpy(pTypeNode->TargetAddr, inet_ntoa(ClntAddr.sin_addr));
        pTypeNode->TargetPort = ntohs(ClntAddr.sin_port);
        pTypeNode->LocalPort = 0;
        pTypeNode->CBArgu = TypeNode->CBArgu;

        return NuSocketTypeCBDone;
    }
    else
    {
        return NuSocketTypeCBError;
    }
}

NuSocketType_t InetStrmSvr = {"InetStrmSvr", NuSocketTypeProperty_LocalPort|NuSocketTypeProperty_NoOnDisconnect|NuSocketTypeProperty_NoOnTimeout, &_StrmSvr_SetOnline, NULL, &_StrmSvr_OnEvent, &_Inet_Denied, &_Inet_Denied, &_Inet_Denied_V, &_Inet_Denied_V};

