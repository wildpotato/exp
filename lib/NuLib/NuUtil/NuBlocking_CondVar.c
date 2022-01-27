
#include "NuBlocking.h"
#include "pthread.h"

typedef struct _NuBlocking_CondVar_t
{
    pthread_mutex_t Mtx;
    pthread_cond_t  Cond;
    int             WaitCnt;
    int             Var;
} NuBlocking_CondVar_t;

static int NuBlocking_CondVar_Init(void *Argu)
{
    int                     RC = NU_OK;
    NuBlocking_CondVar_t    *CondVar = (NuBlocking_CondVar_t *)malloc(sizeof(NuBlocking_CondVar_t));

    if(!CondVar)
    {
        NUGOTO(RC, NU_FAIL, EXIT);
    }

    CondVar->Var = 0;
    CondVar->WaitCnt = 0;
    if(pthread_mutex_init(&(CondVar->Mtx), 0))
    {
        NUGOTO(RC, NU_FAIL, EXIT);
    }

    if(pthread_cond_init(&(CondVar->Cond), 0))
    {
        NUGOTO(RC, NU_FAIL, EXIT);
    }

EXIT:
    if(RC != NU_OK)
    {
        if(CondVar)
        {
            free(CondVar);
        }

        *((NuBlocking_CondVar_t **)Argu) = NULL;
    }
    else
    {
        *((NuBlocking_CondVar_t **)Argu) = CondVar;
    }

    return RC;
}

static int NuBlocking_CondVar_Destroy(void *Argu)
{
    NuBlocking_CondVar_t    *CondVar = (NuBlocking_CondVar_t *)Argu;

    CondVar->WaitCnt = 0;
    CondVar->Var = 0;
    pthread_cond_destroy(&(CondVar->Cond));
    pthread_mutex_destroy(&(CondVar->Mtx));

    free(Argu);

    return NU_OK;
}

static int NuBlocking_CondVar_Block(void *Hdlr, void *BlockingArgu)
{
    NuBlocking_CondVar_t    *CondVar = (NuBlocking_CondVar_t *)Hdlr;

    pthread_mutex_lock(&(CondVar->Mtx));

    ++ (CondVar->WaitCnt);

    while(!(CondVar->Var))
    {
        pthread_cond_wait(&(CondVar->Cond), &(CondVar->Mtx));
    }

    -- (CondVar->WaitCnt);

    CondVar->Var = 0;

    pthread_mutex_unlock(&(CondVar->Mtx));

    return NU_OK;

}

static int NuBlocking_CondVar_Wake(void *Hdlr, void *BlockingArgu)
{
    NuBlocking_CondVar_t    *CondVar = (NuBlocking_CondVar_t *)Hdlr;

    pthread_mutex_lock(&(CondVar->Mtx));

    CondVar->Var = 1;

    if(CondVar->WaitCnt > 0)
    {
        pthread_cond_signal(&(CondVar->Cond));
    }

    pthread_mutex_unlock(&(CondVar->Mtx));

    return NU_OK;
}

static int NuBlocking_CondVar_WakeAll(void *Hdlr, void *BlockingArgu)
{
    NuBlocking_CondVar_t    *CondVar = (NuBlocking_CondVar_t *)Hdlr;

    pthread_mutex_lock(&(CondVar->Mtx));

    CondVar->Var = 1;

    if(CondVar->WaitCnt > 0)
    {
        pthread_cond_broadcast(&(CondVar->Cond));
    }

    pthread_mutex_unlock(&(CondVar->Mtx));

    return NU_OK;
}

NuBlockingType_t NuBlocking_CondVar = {&NuBlocking_CondVar_Init, &NuBlocking_CondVar_Destroy, &NuBlocking_CondVar_Block, &NuBlocking_CondVar_Wake, &NuBlocking_CondVar_WakeAll};

