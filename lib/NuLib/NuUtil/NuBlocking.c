
#include "NuBlocking.h"
#include "NuBlocking_CondVar.c"
#ifdef _GNU_SOURCE
# include "NuBlocking_Polling.c"
#endif
#include "NuCommon.h"

struct _NuBlocking_t
{
    void                *Hdlr;
    NuBlockingType_t    Type;
};

int NuBlockingNew(NuBlocking_t **Blocking, NuBlockingType_t *Type, void *Hdlr)
{
    if(!(*Blocking = (NuBlocking_t *)malloc(sizeof(NuBlocking_t))))
    {
        return NU_MALLOC_FAIL;
    }

    memcpy(&((*Blocking)->Type), Type, sizeof(NuBlockingType_t));

    if(!(Type->Init))
    {
        (*Blocking)->Type.Init = &NuInitailFn_Default;
    }

    if(!(Type->Destroy))
    {
        (*Blocking)->Type.Destroy = &NuDestroyFn_Default;
    }

    (*Blocking)->Hdlr = Hdlr;
    if((*Blocking)->Type.Init(&((*Blocking)->Hdlr)) != NU_OK)
    {
        free(*Blocking);
        *Blocking = NULL;
        return NU_FAIL;
    }

    return NU_OK;
}

void NuBlockingFree(NuBlocking_t *Blocking, void *BlockingArgu)
{
    NuBlockingWakeAll(Blocking, BlockingArgu);

    Blocking->Type.Destroy(Blocking->Hdlr);
    free(Blocking);

    return;
}

void NuBlockingWake(NuBlocking_t *Blocking, void *BlockingArgu)
{
    Blocking->Type.Wake(Blocking->Hdlr, BlockingArgu);

    return;
}

void NuBlockingWakeAll(NuBlocking_t *Blocking, void *BlockingArgu)
{
    Blocking->Type.WakeAll(Blocking->Hdlr, BlockingArgu);

    return;
}

int NuBlockingBlock(NuBlocking_t *Blocking, void *BlockingArgu)
{
    return Blocking->Type.Block(Blocking->Hdlr, BlockingArgu);
}

