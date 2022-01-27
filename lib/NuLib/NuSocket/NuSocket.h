
#include "NuInvoker.h"
#include "NuSocketType.h"

#ifndef _NUSOCKET_H
#define _NUSOCKET_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _NuSocket_t NuSocket_t;
typedef struct _NuSocketNode_t NuSocketNode_t;

typedef void (*NuSocketProtocolEventCB)(NuSocketNode_t *Node, void *Argu);
typedef void (*NuSocketLogCB)(char *Format, va_list ArguList, void *Argu);

typedef struct _NuSocketProtocol_t
{
    NuSocketProtocolEventCB OnConnect;
    NuSocketProtocolEventCB OnDataArrive;
    NuSocketProtocolEventCB OnRemoteTimeout;
    NuSocketProtocolEventCB OnLocalTimeout;
    NuSocketProtocolEventCB OnDisconnect;
} NuSocketProtocol_t;

typedef enum _NuSocketReturnCode
{
    NuSocketReturnOK = 0,
    NuSocketReturnErr = -1,
    NuSocketReturnOfflined = -2
} NuSocketReturnCode;

int NuSocketNew(NuSocket_t **Socket, int ThreadNo, NuSocketLogCB LogFn);
void NuSocketFree(NuSocket_t *Socket);
void NuSocketSetArgu(NuSocketNode_t *Node, void *CBArgu);
void NuSocketSetRecvBuf(NuSocketNode_t *Node, void *Buf, size_t BufLen);
int NuSocketGetRecvBufLen(NuSocketNode_t *Node);
NuSocketNode_t *NuSocketAdd(NuSocket_t *Socket, NuSocketType_t *Type, NuSocketProtocol_t *Protocol, char *TargetAddr, int TargetPort, char *LocalAddr, int LocalPort, void *CBArgu);

int NuSocketSend(NuSocketNode_t *Node, void *Msg, size_t MsgLen);
int NuSocketRecvInLen(NuSocketNode_t *Node, void *Msg, size_t AtLeastLen, size_t AtMostLen);
int NuSocketRecvInTime(NuSocketNode_t *Node, void *Msg, size_t NeedLen, int Sec);
int NuSocketRecvOneTime(NuSocketNode_t *Node, void *Msg, size_t NeedLen);
void NuSocketSetTimeout(NuSocketNode_t *Node, int RemoteTimeout, int LocalTimeout);
void NuSocketSetAutoReconnect(NuSocketNode_t *Node, int AutoReconnect);
void NuSocketReconnect(NuSocketNode_t *Node);
void NuSocketDisconnect(NuSocketNode_t *Node);
void NuSocketInvoke(NuSocket_t *Socket, NuInvokerDelegate_t *Delegate);
char *NuSocketGetAddr(NuSocketNode_t *Node);
int NuSocketGetPort(NuSocketNode_t *Node);

int NuSocketSendV(NuSocketNode_t *Node, struct iovec *iov, int iovCnt);
int NuSocketReadV_InLen(NuSocketNode_t *Node, struct iovec *iov, int iovCnt);
int NuSocketReadV_InTime(NuSocketNode_t *Node, struct iovec *iov, int iovCnt, int Sec);
int NuSocketReadV_OneTime(NuSocketNode_t *Node, struct iovec *iov, int iovCnt);

#ifdef __cplusplus
}
#endif

#endif /* _NUSOCKET_H */

