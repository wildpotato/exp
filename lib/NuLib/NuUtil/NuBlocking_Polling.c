
#include "NuBlocking.h"
#include "pthread.h"

typedef struct _NuBlocking_Polling_t
{
    int             Var;
} NuBlocking_Polling_t;

static int NuBlocking_Polling_Init(void *Argu)
{
    int                     RC = NU_OK;
    NuBlocking_Polling_t    *Polling = (NuBlocking_Polling_t *)malloc(sizeof(NuBlocking_Polling_t));

    if(!Polling)
    {
        NUGOTO(RC, NU_FAIL, EXIT);
    }

    Polling->Var = 0;

EXIT:
    if(RC != NU_OK)
    {
        if(Polling)
        {
            free(Polling);
        }

        *((NuBlocking_Polling_t **)Argu) = NULL;
    }
    else
    {
        *((NuBlocking_Polling_t **)Argu) = Polling;
    }

    return RC;
}

static int NuBlocking_Polling_Destroy(void *Argu)
{
    free(Argu);

    return NU_OK;
}

static int NuBlocking_Polling_Block(void *Hdlr, void *BlockingArgu)
{
    int                     TryCnt = 0;
    NuBlocking_Polling_t    *Polling = (NuBlocking_Polling_t *)Hdlr;

    while(!__sync_bool_compare_and_swap(&(Polling->Var), 1, 0))
    {
        if((++ TryCnt) > 10)
        {
            TryCnt = 0;
            sleep(0);
        }
    }

    return NU_OK;
}

static int NuBlocking_Polling_Wake(void *Hdlr, void *BlockingArgu)
{
    int                     TryCnt = 0;
    NuBlocking_Polling_t    *Polling = (NuBlocking_Polling_t *)Hdlr;

    while(!__sync_bool_compare_and_swap(&(Polling->Var), 0, 1))
    {
        if((++ TryCnt) > 10)
        {
            TryCnt = 0;
            sleep(0);
        }
    }

    return NU_OK;
}

static int NuBlocking_Polling_WakeAll(void *Hdlr, void *BlockingArgu)
{
    NuBlocking_Polling_t    *Polling = (NuBlocking_Polling_t *)Hdlr;

    Polling->Var = 1;

    return NU_OK;
}

NuBlockingType_t NuBlocking_Polling = {&NuBlocking_Polling_Init, &NuBlocking_Polling_Destroy, &NuBlocking_Polling_Block, &NuBlocking_Polling_Wake, &NuBlocking_Polling_WakeAll};

