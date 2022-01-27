
#include "NuCondVar.h"

int NuCondVarInit(NuCondVar_t *CondVar) 
{
    CondVar->var = 0;
    CondVar->waitCnt = 0;

    if(pthread_mutex_init(&(CondVar->mtx), NULL)) {
        return NU_FAIL;
    }

    if(pthread_cond_init(&(CondVar->cond), NULL)) {
        return NU_FAIL;
    }

    return NU_OK;
}

int NuCondVarInitWithShared(NuCondVar_t *CondVar) 
{
    pthread_mutexattr_t mtx_attr;
    pthread_condattr_t  cond_attr;

    CondVar->var = 0;
    CondVar->waitCnt = 0;

    pthread_mutexattr_init(&mtx_attr);
    pthread_mutexattr_setpshared(&mtx_attr, PTHREAD_PROCESS_SHARED);
    if(pthread_mutex_init(&(CondVar->mtx), &mtx_attr)) {
        return NU_FAIL;
    }

    pthread_condattr_init(&cond_attr);
    pthread_condattr_setpshared(&cond_attr, PTHREAD_PROCESS_SHARED);
    if(pthread_cond_init(&(CondVar->cond), &cond_attr)) {
        return NU_FAIL;
    }

    return NU_OK;
}

void NuCondVarDestroy(NuCondVar_t *CondVar) 
{
    CondVar->waitCnt = 0;
    CondVar->var = 0;
    pthread_cond_destroy(&(CondVar->cond));
    pthread_mutex_destroy(&(CondVar->mtx));
}

int NuCondVarNew(NuCondVar_t **CondVar)
{
    (*CondVar) = (NuCondVar_t *)malloc(sizeof(NuCondVar_t));

    if(!(*CondVar)) {
        return NU_FAIL;
    }
    
    if (NuCondVarInit((*CondVar)) != NU_OK) {
        if(*CondVar) {
            free(*CondVar);
        }
    }

    return NU_OK;
}

void NuCondVarFree(NuCondVar_t *CondVar)
{
    CondVar->waitCnt = 0;
    CondVar->var = 0;
    pthread_cond_destroy(&(CondVar->cond));
    pthread_mutex_destroy(&(CondVar->mtx));

    free(CondVar);

    return;
}

