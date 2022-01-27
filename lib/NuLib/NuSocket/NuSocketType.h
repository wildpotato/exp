
#include <sys/uio.h>
#include "NuStr.h"

#ifndef _NUSOCKETTYPE_H
#define _NUSOCKETTYPE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _NuSocketRecvBuf_t
{
    void    *ptr;
    size_t  Len;
    int     RecvLen;
} NuSocketRecvBuf_t;

typedef struct _NuSocketTypeNode_t
{
    int                         InFD;
    int                         OutFD;
    NuStr_t                     *TargetAddr;
    int                         TargetPort;
    NuStr_t                     *LocalAddr;
    int                         LocalPort;
    int                         ByteLeftForRecv;
    void                        *CBArgu;
    NuSocketRecvBuf_t           RecvBuf;
} NuSocketTypeNode_t;

typedef int (*NuSocketTypeCB)(NuSocketTypeNode_t *TypeNode);
typedef int (*NuSocketTypeOpCB)(NuSocketTypeNode_t *TypeNode, void *Msg, size_t MsgLen);
typedef int (*NuSocketTypeOpVecCB)(NuSocketTypeNode_t *TypeNode, struct iovec *iov, int iovCnt);

typedef enum _NuSocketTypeProperty
{
    NuSocketTypeProperty_TargetAddr = 0x01,
    NuSocketTypeProperty_TargetPort = 0x02,
    NuSocketTypeProperty_LocalAddr = 0x04,
    NuSocketTypeProperty_LocalPort = 0x08,
    NuSocketTypeProperty_NoOnConnect = 0x10,
    NuSocketTypeProperty_NoOnDisconnect = 0x20,
    NuSocketTypeProperty_NoOnTimeout = 0x40
} NuSocketTypeProperty;

typedef struct _NuSocketType_t
{
    char                    *Name;
    NuSocketTypeProperty    Property;
    NuSocketTypeCB          SetOnline;
    NuSocketTypeCB          SetOffline;
    NuSocketTypeCB          OnEvent;
    NuSocketTypeOpCB        Send;
    NuSocketTypeOpCB        Recv;

	NuSocketTypeOpVecCB     SendV;
	NuSocketTypeOpVecCB     ReadV;
} NuSocketType_t;

typedef enum _NuSocketTypeCBReturnCode
{
    NuSocketTypeCBError = -2,
    NuSocketTypeCBDone = -1,
    NuSocketTypeCBPass = 0
} NuSocketTypeCBReturnCode;

NuSocketTypeNode_t *NuSocketTypeNodeFork(NuSocketTypeNode_t *TypeNode, NuSocketType_t *Type, int FD);
void NuSocketTypeLog(NuSocketTypeNode_t *TypeNode, char *Format, ...);

#ifdef __cplusplus
}
#endif

#endif /* _NUSOCKETTYPE_H */

