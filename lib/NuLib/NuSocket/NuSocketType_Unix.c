
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <sys/un.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>

#include "NuSocketType_UnixDgrmClnt.h"
#include "NuSocketType_UnixDgrmSvr.h"
#include "NuSocketType_UnixStrmClnt.h"
#include "NuSocketType_UnixStrmSvr.h"

static int _UnixTemplate(NuSocketTypeNode_t *TypeNode, int SocketType)
{
    int                 FD = 0, Flag = 1;
    struct sockaddr_un  AddrUn;
    struct sockaddr     *pAddr = (struct sockaddr *)(&AddrUn);

    if(SocketType != SOCK_STREAM && SocketType != SOCK_DGRAM)
    {
        NuSocketTypeLog(TypeNode, "Unsupport socket type(%d)\n", SocketType);
        return NuSocketTypeCBError;
    }

    if((FD = socket(AF_UNIX, SocketType, 0)) < 0)
    {
        NuSocketTypeLog(TypeNode, "socket(%s)\n", strerror(errno));
        return NuSocketTypeCBError;
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

    if(NuStrSize(TypeNode->LocalAddr) > 0)
    {
        unlink(NuStrGet(TypeNode->LocalAddr));

        AddrUn.sun_family = AF_UNIX;
        strcpy(AddrUn.sun_path, NuStrGet(TypeNode->LocalAddr));

        if(bind(FD, pAddr, sizeof(AddrUn)) < 0)
        {
            NuSocketTypeLog(TypeNode, "bind(%s)\n", strerror(errno));
            close(FD);
            TypeNode->InFD = TypeNode->OutFD = 0;
            return NuSocketTypeCBError;
        }
    }

    if(NuStrSize(TypeNode->TargetAddr) > 0)
    {
        AddrUn.sun_family = AF_UNIX;
        strcpy(AddrUn.sun_path, NuStrGet(TypeNode->TargetAddr));

	    if(connect(FD, pAddr, sizeof(AddrUn)) < 0)
        {
            NuSocketTypeLog(TypeNode, "connect(%s)\n", strerror(errno));
            close(FD);
            TypeNode->InFD = TypeNode->OutFD = 0;
            return NuSocketTypeCBError;
        }
    }

    Flag = fcntl(FD, F_GETFL, 0);
    fcntl(FD, F_SETFL, Flag | O_NONBLOCK);

    TypeNode->InFD = TypeNode->OutFD = FD;

    return NuSocketTypeCBPass;
}

static int _Unix_SetOffline(NuSocketTypeNode_t *TypeNode)
{
    unlink(NuStrGet(TypeNode->TargetAddr));
    return NuSocketTypeCBDone;
}

/* DgrmClnt */
static int _DgrmClnt_SetOnline(NuSocketTypeNode_t *TypeNode)
{
    return _UnixTemplate(TypeNode, SOCK_DGRAM);
}

NuSocketType_t UnixDgrmClnt = {"UnixDgrmClnt", NuSocketTypeProperty_TargetAddr, &_DgrmClnt_SetOnline, NULL, NULL, NULL, NULL, NULL, NULL};

/*DgrmSvr */
static int _DgrmSvr_SetOnline(NuSocketTypeNode_t *TypeNode)
{
    return _UnixTemplate(TypeNode, SOCK_DGRAM);
}

static int _DgrmSvr_Send(NuSocketTypeNode_t *TypeNode, void *Msg, size_t MsgLen)
{
    NuSocketType_UnixDgrmSvrMsg *pMsg = (NuSocketType_UnixDgrmSvrMsg *)Msg;
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
    NuSocketType_UnixDgrmSvrMsg *pMsg = (NuSocketType_UnixDgrmSvrMsg *)Msg;
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

NuSocketType_t UnixDgrmSvr = {"UnixDgrmSvr", NuSocketTypeProperty_LocalAddr|NuSocketTypeProperty_NoOnTimeout, &_DgrmSvr_SetOnline, &_Unix_SetOffline, NULL, &_DgrmSvr_Send, &_DgrmSvr_Recv, NULL, NULL};

/* StrmClnt */
static int _StrmClnt_SetOnline(NuSocketTypeNode_t *TypeNode)
{
    return _UnixTemplate(TypeNode, SOCK_STREAM);
}

NuSocketType_t UnixStrmClnt = {"UnixStrmClnt", NuSocketTypeProperty_TargetAddr, &_StrmClnt_SetOnline, NULL, NULL, NULL, NULL};

/* StrmSvr */
static NuSocketType_t UnixStrmSvr_Clnt = {"UnixStrmSvr_Clnt", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

static int _StrmSvr_SetOnline(NuSocketTypeNode_t *TypeNode)
{
    int RC = _UnixTemplate(TypeNode, SOCK_STREAM);

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
    struct sockaddr_un          ClntAddr;
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

        pTypeNode = NuSocketTypeNodeFork(TypeNode, &UnixStrmSvr_Clnt, FD);

        NuStrCpy(pTypeNode->TargetAddr, ClntAddr.sun_path);
        pTypeNode->TargetPort = 0;
        pTypeNode->LocalPort = 0;
        pTypeNode->CBArgu = TypeNode->CBArgu;

        return NuSocketTypeCBDone;
    }
    else
    {
        return NuSocketTypeCBError;
    }
}

NuSocketType_t UnixStrmSvr = {"UnixStrmSvr", NuSocketTypeProperty_LocalAddr|NuSocketTypeProperty_NoOnTimeout, &_StrmSvr_SetOnline, &_Unix_SetOffline, &_StrmSvr_OnEvent, NULL, NULL, NULL, NULL};

