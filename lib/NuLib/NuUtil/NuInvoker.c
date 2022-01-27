
#include "NuUtil.h"
#include "NuThread.h"
#include "NuInvoker.h"
#include "NuTimer.h"

struct _NuInvoker_t
{
    base_vector_t               *Groups;
    NuTimerEventHdlr_t          *Timer;
} *_Invoker = NULL;

struct _NuInvokerGroup_t
{
    int                 Start;
    int                 MaxThreadNo;
    NuBlocking_t        *Blocking;
    base_vector_t       *Threads;
};

typedef struct _NuInvokerThread_t
{
    NuThread_t          ID;
    int                 Busy;
    NuInvokerGroup_t    *Group;
} NuInvokerThread_t;

static NUTHD_FUNC WorkThreadFn(void *Argu)
{
    NuInvokerThread_t       *WorkThread = (NuInvokerThread_t *)Argu;
    NuInvokerGroup_t        *Group = WorkThread->Group;
    NuInvokerDelegate_t     Delegate;

    while(Group->Start)
    {
        if(NuBlockingBlock(Group->Blocking, &Delegate) == NU_OK)
        {
            WorkThread->Busy = 1;

            Delegate.Fn(Delegate.Argu);

            WorkThread->Busy = 0;
        }
    }

    WorkThread->ID = 0;

    NuThdReturn();
    return NULL;
}

static void AddAWorkThread(NuInvokerGroup_t *Group)
{
    NuInvokerThread_t   *pWorkThread = (NuInvokerThread_t *)malloc(sizeof(NuInvokerThread_t));

    if(pWorkThread != NULL)
    {
        pWorkThread->Busy = 0;
        pWorkThread->Group = Group;

        NuThdCreate(&WorkThreadFn, pWorkThread, &(pWorkThread->ID));
        base_vector_push(Group->Threads, pWorkThread);
    }

    return;
}

static void FunctionPerSec(void *Argu)
{
    NuInvokerGroup_t    *Group = NULL;
    NuInvokerThread_t   *WorkThread = NULL;
    base_vector_t       *GroupVec = _Invoker->Groups;
    base_vector_it      GroupVecIt;
    base_vector_t       *ThreadVec = NULL;
    base_vector_it      ThreadVecIt;

    base_vector_it_set(GroupVecIt, GroupVec);
    while(GroupVecIt != base_vector_it_end(GroupVec))
    {
		int ThreadNo = 0;
        Group = (NuInvokerGroup_t *)(*GroupVecIt);

        if(Group->Start && (ThreadNo = base_vector_get_cnt(ThreadVec = Group->Threads)) < Group->MaxThreadNo)
        {
			int BusyWorkThreadCnt = 0;

            base_vector_it_set(ThreadVecIt, ThreadVec);
            while(ThreadVecIt != base_vector_it_end(ThreadVec))
            {
                WorkThread = (NuInvokerThread_t *)(*ThreadVecIt);
                if(WorkThread->Busy)
                {
                    ++ BusyWorkThreadCnt;
                }

                ++ ThreadVecIt;
            }

            if((BusyWorkThreadCnt > 0.8 * ThreadNo))
            {
                AddAWorkThread(Group);
            }
        }

        ++ GroupVecIt;
    }

    return;
}

NuInvokerGroup_t *NuInvokerAddGroup(NuBlockingType_t *BlockingType, void *BlockingHdlr, int ThreadNo, int MaxThreadNo)
{
    int                 New = 0;
    NuInvokerGroup_t    *Group = NULL;
    base_vector_t       *Vec = NULL;
    base_vector_it      VecIt;

    if(!_Invoker)
    {
        _Invoker = (struct _NuInvoker_t *)malloc(sizeof(struct _NuInvoker_t));

        base_vector_new(&(_Invoker->Groups), 3);
        _Invoker->Timer = NuTimerRegister(1, 0, &FunctionPerSec, NULL);
    }

    base_vector_it_set(VecIt, Vec = _Invoker->Groups);
    while(VecIt != base_vector_it_end(Vec))
    {
        Group = (NuInvokerGroup_t *)(*VecIt);
        if(!(Group->Start))
        {
            break;
        }

        ++ VecIt;
    }

    if(VecIt == base_vector_it_end(Vec))
    {
        Group = (NuInvokerGroup_t *)malloc(sizeof(NuInvokerGroup_t));
        base_vector_new(&(Group->Threads), 3);

        New = 1;
    }

    base_vector_clear(Group->Threads);

    if(ThreadNo < 1)
    {
        ThreadNo = 1;
    }

    if(MaxThreadNo < 1 || MaxThreadNo < ThreadNo)
    {
        Group->MaxThreadNo = ThreadNo;
    }
    else
    {
        Group->MaxThreadNo = MaxThreadNo;
    }

    NuBlockingNew(&(Group->Blocking), BlockingType, BlockingHdlr);

    Group->Start = 1;
    while(ThreadNo --)
    {
        AddAWorkThread(Group);
    }

    if(New)
    {
        base_vector_push(_Invoker->Groups, Group);
    }

    return Group;
}

void NuInvokerInvoke(NuInvokerGroup_t *Group, NuInvokerDelegate_t *Delegate)
{
    NuBlockingWake(Group->Blocking, Delegate);
    return;
}

void NuInvokerRemove(NuInvokerGroup_t *Group)
{
    NuInvokerDelegate_t     Delegate = {&NuEventFn_Default, NULL};
    NuInvokerThread_t       *WorkThread = NULL;
    base_vector_t           *Vec = NULL;
    base_vector_it          VecIt;

    Group->Start = 0;

    base_vector_it_set(VecIt, Vec = Group->Threads);
    while(VecIt != base_vector_it_end(Vec))
    {
        WorkThread = (NuInvokerThread_t *)(*VecIt);
        if(WorkThread->ID)
        {
            NuBlockingWake(Group->Blocking, &Delegate);
            sleep(1);
        }
        else
        {
            free(WorkThread);
            ++ VecIt;
        }
    }

    sleep(1);
    base_vector_clear(Vec);
    NuBlockingFree(Group->Blocking, &Delegate);

    return;
}

void NuInvokerStop()
{
    NuInvokerGroup_t    *Group = NULL;
    base_vector_t       *Vec = NULL;

    if(_Invoker != NULL)
    {
		base_vector_it VecIt;
        base_vector_it_set(VecIt, Vec = _Invoker->Groups);
        while(VecIt != base_vector_it_end(Vec))
        {
            Group = (NuInvokerGroup_t *)(*VecIt);

            if(Group->Start)
            {
                NuInvokerRemove(Group);
                base_vector_free(Group->Threads);
            }

            ++ VecIt;
        }

        sleep(1);
        NuTimerUnregister(_Invoker->Timer);
        base_vector_free(Vec);

        free(_Invoker);
        _Invoker = NULL;
    }

    return;
}

