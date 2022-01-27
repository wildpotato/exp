
#include "NuCommon.h"
#include "pthread.h"

#ifndef _NUCONDVAR_H
#define _NUCONDVAR_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _NuCondVar_t 
{
    pthread_mutex_t mtx;
    pthread_cond_t  cond;
    int             waitCnt;
    int             var;
    
} NuCondVar_t;

int NuCondVarInit(NuCondVar_t *CondVar);
int NuCondVarInitWithShared(NuCondVar_t *CondVar);
void NuCondVarDestroy(NuCondVar_t *CondVar);

/* 宣告 pointer 時使用, 內部會自行init以及destroy
 * */
int NuCondVarNew(NuCondVar_t **CondVar);
void NuCondVarFree(NuCondVar_t *CondVar);

static inline void NuCondVarWake(NuCondVar_t *CondVar) {
    pthread_mutex_lock(&(CondVar->mtx));

    CondVar->var = 1;

    if(CondVar->waitCnt > 0)
    {
        pthread_cond_signal(&(CondVar->cond));
    }

    pthread_mutex_unlock(&(CondVar->mtx));

    return;
}

static inline void NuCondVarWakeAll(NuCondVar_t *CondVar) {
    pthread_mutex_lock(&(CondVar->mtx));

    CondVar->var = 1;

    if(CondVar->waitCnt > 0)
    {
        pthread_cond_broadcast(&(CondVar->cond));
    }

    pthread_mutex_unlock(&(CondVar->mtx));

    return;
}

static inline int NuCondVarBlock(NuCondVar_t *CondVar) {
    pthread_mutex_lock(&(CondVar->mtx));

    ++ (CondVar->waitCnt);

    while(!(CondVar->var))
    {
        pthread_cond_wait(&(CondVar->cond), &(CondVar->mtx));
    }

    -- (CondVar->waitCnt);

    CondVar->var = 0;

    pthread_mutex_unlock(&(CondVar->mtx));

    return NU_OK;
}

#ifdef __cplusplus
}
#endif

#endif /* _NUCONDVAR_H */

