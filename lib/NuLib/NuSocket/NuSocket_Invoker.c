
#include "NuSocketType_Pipe.h"

typedef struct _InternalInvoker_t
{
    NuSocketProtocolEventCB Fn;
    NuSocketNode_t          *Target;
} InternalInvoker_t;

typedef enum _InvokeType
{
    InvokeType_Internal = 0,
    InvokeType_External,
    InvokeTypeNo
} InvokeType;

typedef struct _Invoker_t
{
    InvokeType              Type;
    union TypeData
    {
        InternalInvoker_t   Internal;
        NuInvokerDelegate_t External;
    } Data;
} Invoker_t;

#define InvokerSize         sizeof(Invoker_t)

static void OnDataArrive(NuSocketNode_t *Node, void *CBArgu)
{
    Invoker_t           Invoker;
    InternalInvoker_t   *pInternal = &(Invoker.Data.Internal);
    NuInvokerDelegate_t *pExternal = &(Invoker.Data.External);

    if(NuSocketRecvInTime(Node, &Invoker, InvokerSize, 1) == InvokerSize)
    {
        AddToMonitor(Node);

        if(Invoker.Type == InvokeType_Internal)
        {
            (pInternal->Fn)(pInternal->Target, pInternal->Target->TypeNode.CBArgu);
        }
        else if(Invoker.Type == InvokeType_External)
        {
            (pExternal->Fn)(pExternal->Argu);
        }
    }
    else
    {
        AddToMonitor(Node);
    }

    return;
}

static NuSocketProtocol_t  Invoker = {NULL, &OnDataArrive, NULL, NULL, NULL};

static void Invoke(NuSocketProtocolEventCB Fn, NuSocketNode_t *Target)
{
    Invoker_t   Invoker;

    Invoker.Type = InvokeType_Internal;
    Invoker.Data.Internal.Fn = Fn;
    Invoker.Data.Internal.Target = Target;

    NuSocketSend(Target->Socket->InternalEventNode, &Invoker, InvokerSize);

    return;
}

static int ExternalInvokeFn(void *Hdlr, void *Argu)
{
    NuSocket_t          *Socket = (NuSocket_t *)Hdlr;
    NuInvokerDelegate_t *Delegate = (NuInvokerDelegate_t *)Argu;
    Invoker_t           Invoker;

    Invoker.Type = InvokeType_External;
    memcpy(&(Invoker.Data.External), Delegate, sizeof(NuInvokerDelegate_t));

    NuSocketSend(Socket->InternalEventNode, &Invoker, InvokerSize);

    return NU_OK;
}

