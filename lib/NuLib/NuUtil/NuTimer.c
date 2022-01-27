
#include "NuCommon.h"
#include "NuUtil.h"
#include "NuThread.h"
#include "NuBuffer.h"

typedef struct _NuTimer_t
{
    int                 ThreadWorking;
    struct timespec     Now;
    NuThread_t          ThreadID;
    NuBuffer_t          *EventHdlrBuffer;
    base_vector_t       *EventHdlrVec;
} NuTimer_t;
static NuTimer_t *_Timer = NULL;

typedef struct _NuTimerEventHdlr_t
{
    struct timespec     Next;
    int                 Period;
    NuEventFn           CBFn;
    void                *CBArgu;
} NuTimerEventHdlr_t;

static void _TimerLoop(void)
{
    base_vector_t       *pVec = _Timer->EventHdlrVec;
    NuTimerEventHdlr_t  *pEventHdlr = NULL;
    struct timespec     *pNow = &(_Timer->Now);

    clock_gettime(CLOCK_REALTIME, pNow);

    while(_Timer->ThreadWorking)
    {
		base_vector_it VecIt;
        base_vector_it_set(VecIt, pVec);
        while(VecIt != base_vector_it_end(pVec))
        {
            pEventHdlr = (NuTimerEventHdlr_t *)(*VecIt);

            if(pEventHdlr->Period != 0)
            {
                if(pNow->tv_sec >= pEventHdlr->Next.tv_sec)
                {
                    (pEventHdlr->CBFn)(pEventHdlr->CBArgu);
                    pEventHdlr->Next.tv_sec += pEventHdlr->Period;
                }
            }

            ++ VecIt;
        }

        ++ (pNow->tv_sec);
        clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, pNow, NULL);
    }

    return;
}

static NUTHD_FUNC _TimerThreadLoop(void *Argu)
{
    _TimerLoop();

    NuThdReturn();
    return NULL;
}

int NuTimerNew(int Self)
{
    if(_Timer != NULL)
    {
        if(_Timer->ThreadID > 0 && Self > 0)
        {
            _Timer->ThreadWorking = 0;
            NuThdJoin(_Timer->ThreadID);
            _Timer->ThreadWorking = 1;
            _Timer->ThreadID = 0;
            _TimerLoop();
        }
        else
        {
            return NU_FAIL;
        }
    }
    else
    {
        _Timer = (NuTimer_t *)malloc(sizeof(NuTimer_t));

        if(!_Timer)
        {
            return NU_MALLOC_FAIL;
        }

        if(base_vector_new(&(_Timer->EventHdlrVec), 10) != NU_OK)
        {
            return NU_MALLOC_FAIL;
        }

        if(NuBufferNew(&(_Timer->EventHdlrBuffer), sizeof(NuTimerEventHdlr_t), 10) != NU_OK)
        {
            return NU_MALLOC_FAIL;
        }

        _Timer->ThreadWorking = 1;

        if(!Self)
        {
            if(NuThdCreate2(&_TimerThreadLoop, _Timer, 1024, &(_Timer->ThreadID)) < 0)
            {
                return NU_MALLOC_FAIL;
            }
        }
        else
        {
            _Timer->ThreadID = 0;
            _TimerLoop();
        }
    }

    return NU_OK;
}

void NuTimerStop(void)
{
    if(_Timer != NULL)
    {
        _Timer->ThreadWorking = 0;

        if(_Timer->ThreadID > 0)
        {
            NuThdJoin(_Timer->ThreadID);
        }
    }

    return;
}

void NuTimerFree(void)
{
    NuTimerStop();

    if(_Timer != NULL)
    {
        NuBufferFree(_Timer->EventHdlrBuffer);
        _Timer->EventHdlrBuffer = NULL;

        base_vector_free(_Timer->EventHdlrVec);
        _Timer->EventHdlrVec = NULL;

        free(_Timer);
    }

    return;
}

static void _SetEventHdlr(NuTimerEventHdlr_t *pEventHdlr, int Period, time_t Start, NuEventFn CBFn, void *CBArgu)
{
    if(Start > 0)
    {
        pEventHdlr->Next.tv_sec = Start;
        pEventHdlr->Next.tv_nsec = 0;
    }
    else
    {
        clock_gettime(CLOCK_REALTIME, &(pEventHdlr->Next));
        pEventHdlr->Next.tv_sec += Period;
    }

    pEventHdlr->CBFn = CBFn;
    pEventHdlr->CBArgu = CBArgu;
    pEventHdlr->Period = Period;

    return;
}

NuTimerEventHdlr_t *NuTimerRegister(int Period, time_t Start, NuEventFn CBFn, void *CBArgu)
{
    NuTimerEventHdlr_t  *pEventHdlr = NULL;
    base_vector_t       *pVec = NULL;
    base_vector_it      VecIt;

    NuTimerNew(0);

    base_vector_it_set(VecIt, pVec = _Timer->EventHdlrVec);
    while(VecIt != base_vector_it_end(pVec))
    {
        if(!(((NuTimerEventHdlr_t *)(*VecIt))->Period))
        {
            pEventHdlr = (NuTimerEventHdlr_t *)(*VecIt);
            _SetEventHdlr(pEventHdlr, Period, Start, CBFn, CBArgu);

            break;
        }

        ++ VecIt;
    }

    if(!pEventHdlr)
    {
        pEventHdlr = (NuTimerEventHdlr_t *)NuBufferGet(_Timer->EventHdlrBuffer);
        _SetEventHdlr(pEventHdlr, Period, Start, CBFn, CBArgu);

        base_vector_push(pVec, pEventHdlr);
    }

    return pEventHdlr;
}

void NuTimerUnregister(NuTimerEventHdlr_t *pEventHdlr)
{
    pEventHdlr->CBFn = &NuEventFn_Default;
    pEventHdlr->CBArgu = NULL;
    pEventHdlr->Period = 0;

    return;
}

time_t NuTimerGetNow(void)
{
    NuTimerNew(0);
    return _Timer->Now.tv_sec;
}

