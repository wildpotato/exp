
#include <omp.h>

#include "NuCommon.h"
#include "NuUtil.h"
#include "NuSpin.h"
#include "NuCBMgr.h"

struct _NuCBMgrHdlr_t
{
    NuCBMgrFn       Fn;
    void            *Argu;
};

struct _NuCBMgr_t
{
    base_vector_t   EventVec;
    NuSpin_t        Lock;
};

static void _NuCBMgrFn_Default(const void *RaiseArgu, void *Argu)
{
    return;
}

NuCBMgr_t *NuCBMgrAdd(void)
{
    NuCBMgr_t   *Mgr = (NuCBMgr_t *)malloc(sizeof(NuCBMgr_t));

    if(!Mgr)
    {
        return NULL;
    }

    if(base_vector_init(&(Mgr->EventVec), 10) < 0)
    {
        free(Mgr);
        return NULL;
    }

    if(NuSpinInit(&(Mgr->Lock)) < 0)
    {
        free(Mgr);
        return NULL;
    }

    return Mgr;
}

void NuCBMgrDel(NuCBMgr_t *Mgr)
{
    if(Mgr != NULL)
    {
        base_vector_foreach(&(Mgr->EventVec), free);
        base_vector_destroy(&(Mgr->EventVec));

        NuSpinDestroy(&(Mgr->Lock));

        free(Mgr);
    }

    return;
}

NuCBMgrHdlr_t *NuCBMgrRegisterEvent(NuCBMgr_t *Mgr, NuCBMgrFn Fn, void *Argu)
{
    NuCBMgrHdlr_t   *Hdlr = NULL;
    base_vector_t   *Vec = &(Mgr->EventVec);
    base_vector_it  VecIt;

    base_vector_it_set(VecIt, Vec);
    while(VecIt != base_vector_it_end(Vec))
    {
        Hdlr = (NuCBMgrHdlr_t *)(*VecIt);

        if(Hdlr->Fn == &_NuCBMgrFn_Default)
        {
            Hdlr->Argu = Argu;
            Hdlr->Fn = Fn;

            return Hdlr;
        }

        ++ VecIt;
    }


    Hdlr = (NuCBMgrHdlr_t *)malloc(sizeof(NuCBMgrHdlr_t));
    Hdlr->Argu = Argu;
    Hdlr->Fn = Fn;

    NuSpinLock(&(Mgr->Lock));
    base_vector_push(Vec, Hdlr);
    NuSpinUnLock(&(Mgr->Lock));

    return Hdlr;
}

void NuCBMgrUnRegisterEvent(NuCBMgrHdlr_t *Hdlr)
{
    Hdlr->Fn = &_NuCBMgrFn_Default;
    Hdlr->Argu = NULL;

    return;
}

void NuCBMgrReset(NuCBMgr_t *Mgr)
{
    base_vector_t   *Vec = &(Mgr->EventVec);
    base_vector_it  VecIt;

    base_vector_it_set(VecIt, Vec);
    while(VecIt != base_vector_it_end(Vec))
    {
        NuCBMgrUnRegisterEvent(*VecIt);
        ++ VecIt;
    }

    return;
}

void NuCBMgrRaiseEvent(NuCBMgr_t *Mgr, const void *RaiseArgu)
{
    base_vector_t   *Vec = &(Mgr->EventVec);
    base_vector_it  VecIt;
    NuCBMgrHdlr_t   *Hdlr = NULL;

    NuSpinLock(&(Mgr->Lock));

    base_vector_it_set(VecIt, Vec);
    while(VecIt != base_vector_it_end(Vec))
    {
        Hdlr = (NuCBMgrHdlr_t *)(*VecIt);
        Hdlr->Fn(RaiseArgu, Hdlr->Argu);

        ++ VecIt;
    }

    NuSpinUnLock(&(Mgr->Lock));

    return;
}

#if defined(_OPENMP)
void NuCBMgrRaiseEventParallel(NuCBMgr_t *Mgr, const void *RaiseArgu)
{
    base_vector_t   *Vec = &(Mgr->EventVec);

    NuSpinLock(&(Mgr->Lock));
	
	#pragma omp parallel
	{
		#pragma omp for
		for(int idx = 0; idx < base_vector_get_cnt(Vec); idx++)
		{
			((NuCBMgrHdlr_t *)base_vector_get_by_index(Vec, idx))->Fn(RaiseArgu, 
			                                                          ((NuCBMgrHdlr_t *)base_vector_get_by_index(Vec, idx))->Argu);
		}
	}

    NuSpinUnLock(&(Mgr->Lock));
}
#endif

