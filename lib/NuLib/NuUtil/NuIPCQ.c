
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "NuIPCQ.h"

#define NuIPCQ_READ     0400
#define NuIPCQ_WRITE    0200

#define NuIPCQ_ACCESS   (NuIPCQ_READ | NuIPCQ_WRITE | (NuIPCQ_READ >> 3) | (NuIPCQ_WRITE >> 3))

struct _NuIPCQ_t
{
    int     Id;
    key_t   Key;
};

/* Internal functions. */
static int _QGet(NuIPCQ_t *Q, key_t *pKey, int Flag)
{
    int Ret = NU_OK;

    memcpy(&(Q->Key), pKey, sizeof(key_t));

    if((Ret = msgget(Q->Key, Flag | NuIPCQ_ACCESS)) < 0)
    {
        switch(errno)
        {
        case EACCES:
            Ret = NU_QACCESSFAIL;
            break;
        case ENOMEM:
        case ENOSPC:
            Ret = NU_QRESOURCEDEFICIENT;
            break;
        default:
            Ret = NU_CREATEQFAIL;
            break;
        }
    }
    else
    {
        Q->Id = Ret;
    }

    return Ret;
}

static int _QSend(NuIPCQ_t *Q, NuIPCQMsgHdr_t *Msg, size_t DataLen, int Flag)
{
    int Ret = NU_OK;

    if(msgsnd(Q->Id, Msg, DataLen, Flag) < 0)
    {
        switch(errno)
        {
        case EAGAIN:
            Ret = NU_QRESOURCEDEFICIENT;
            break;
        case EIDRM:
            Ret = NU_QNOTEXIST;
            break;
        case EINTR:
            Ret = NU_ENQINTERRUPT;
            break;
        default:
            Ret = NU_ENQFAIL;
            break;
        }
    }

    return Ret;
}

static int _QRecv(NuIPCQ_t *Q, NuIPCQMsgHdr_t *Msg, size_t DataLen, int Flag)
{
    int Ret = NU_OK;

    if((Ret = msgrcv(Q->Id, Msg, DataLen, Msg->MsgType, Flag)) < 0)
    {
        switch(errno)
        {
        case E2BIG:
            Ret = NU_PARAMERROR;
            break;
        case ENOMSG:
            Ret = 0;
            break;
        case EIDRM:
            Ret = NU_QNOTEXIST;
            break;
        case EINTR:
            Ret = NU_ENQINTERRUPT;
            break;
        default:
            Ret = NU_DEQFAIL;
            break;
        }
    }

    return Ret;
}

/* Create/Attach an IPC Queue with specified key. */
int NuIPCQNew(NuIPCQ_t **Q)
{
    if(!(*Q = (NuIPCQ_t *)malloc(sizeof(NuIPCQ_t))))
    {
        return NU_MALLOC_FAIL;
    }
    memset(*Q, '\0', sizeof(NuIPCQ_t));
    return NU_OK;
}

/* Create/Attach an IPC queue only for forked processes using. */
int NuIPCQCreatePrivate(NuIPCQ_t *Q)
{
	key_t Key = IPC_PRIVATE;
    return _QGet(Q, &Key, IPC_CREAT);
}

int NuIPCQAttach(NuIPCQ_t *Q, key_t *Key)
{
    return _QGet(Q, Key, 0);
}

int NuIPCQDetach(NuIPCQ_t *Q)
{
    return NU_OK;
}

int NuIPCQCreate(NuIPCQ_t *Q, key_t *Key)
{
    return _QGet(Q, Key, IPC_CREAT);
}

int NuIPCQDelete(NuIPCQ_t *Q)
{
    int Ret = NU_OK;

    if(msgctl(Q->Id, IPC_RMID, NULL) < 0)
    {
        switch(errno)
        {
        case EACCES:
        case EPERM:
            Ret = NU_QACCESSFAIL;
            break;
        case EIDRM:
        case EINVAL:
            Ret = NU_QNOTEXIST;
            break;
        default:
            Ret = NU_RMQFAIL;
            break;
        }
    }

    return Ret;
}

/* EnQueue. */
int NuIPCQEnqueue(NuIPCQ_t *Q, NuIPCQMsgHdr_t *Msg, size_t DataLen)
{
    return _QSend(Q, Msg, DataLen, IPC_NOWAIT);
}

/* Blocking verion of enqueue. */
int NuIPCQBlockingEnqueue(NuIPCQ_t *Q, NuIPCQMsgHdr_t *Msg, size_t DataLen)
{
    int RC = NU_OK;

    while((RC = _QSend(Q, Msg, DataLen, false)) < 0)
    {
        if(RC == NU_ENQFAIL)
        {
            return RC;
        }
    }

    return RC;
}

/* DeQueue. */
int NuIPCQDequeue(NuIPCQ_t *Q, NuIPCQMsgHdr_t *Msg, size_t DataLen)
{
    return _QRecv(Q, Msg, DataLen, IPC_NOWAIT);
}

/* Blocking version of dequeue. */
int NuIPCQBlockingDequeue(NuIPCQ_t *Q, NuIPCQMsgHdr_t *Msg, size_t DataLen)
{
    int RC = NU_OK;
    
    while((RC = _QRecv(Q, Msg, DataLen, 0)) < 0)
    {
        if(RC == NU_DEQFAIL)
        {
            return RC;
        }
    }

    return RC;
}

/* Free the IPC Queue instance. */
void NuIPCQFree(NuIPCQ_t *Q)
{
    free(Q);
}
