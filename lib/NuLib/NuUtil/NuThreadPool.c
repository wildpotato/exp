
#include <signal.h>
#include <pthread.h>

#include "NuUtil.h"
#include "NuThreadPool.h"


struct _NuThreadPool_t
{
    bool            Working;
    base_vector_t   Threads;
    NuThreadPoolFn  Fn;
};

typedef struct _NuThreadPoolNode_t
{
    pthread_t       ID;
    NuThreadPool_t  *Pool;
    void            *Argu;
} NuThreadPoolNode_t;

static void *WorkThreadFn(void *Argu)
{
    NuThreadPoolNode_t  *Node = (NuThreadPoolNode_t *)Argu;
    NuThreadPool_t      *Pool = Node->Pool;

    while(Pool->Working)
    {
        Pool->Fn(Node->Argu);
    }

//    Node->ID = 0;

    return NULL;
}

static NuThreadPoolNode_t *NewThreadNode(NuThreadPool_t *Pool, void *Argu)
{
    NuThreadPoolNode_t *Node = (NuThreadPoolNode_t *)malloc(sizeof(NuThreadPoolNode_t));
 
    if(Node != NULL)
    {
        Node->ID = 0;
        Node->Pool = Pool;
        Node->Argu = Argu;

        base_vector_push(&(Pool->Threads), Node);
    }

    return Node;
}

int NuThreadPoolNew(NuThreadPool_t **pPool, NuThreadPoolFn Fn)
{
    int RC = NU_OK;

    if(!(*pPool = (NuThreadPool_t *)malloc(sizeof(NuThreadPool_t))))
    {
        NUGOTO(RC, NU_MALLOC_FAIL, EXIT);
    }

    if(base_vector_init(&((*pPool)->Threads), 10))
    {
        NUGOTO(RC, NU_MALLOC_FAIL, EXIT);
    }

    (*pPool)->Fn = Fn;
    (*pPool)->Working = true;

EXIT:
    return RC;
}

void NuThreadPoolAddThread(NuThreadPool_t *Pool, void *Argu)
{
    NuThreadPoolNode_t *Node = NewThreadNode(Pool, Argu);
 
    if(Node != NULL)
    {
        pthread_create(&(Node->ID), NULL, &WorkThreadFn, Node);
    }

    return;
}

void NuThreadPoolAddThreads(NuThreadPool_t *Pool, int ThreadNo, void *Argu)
{
    while(ThreadNo --)
    {
        NuThreadPoolAddThread(Pool, Argu);
    }

    return;
}

bool NuThreadPoolPutSelfIntoPool(NuThreadPool_t *Pool, void *Argu)
{
    pthread_t           Self = pthread_self();
    NuThreadPoolNode_t  *Node = NULL;
    base_vector_t       *Vec = &(Pool->Threads);
    base_vector_it      VecIt;

    base_vector_it_set(VecIt, Vec);
    while(VecIt != base_vector_it_end(Vec))
    {
        Node = (NuThreadPoolNode_t *)(*VecIt);
        if(pthread_equal(Self, Node->ID))
        {
            return false;
        }

        ++ VecIt;
    }

    if((Node = NewThreadNode(Pool, Argu)) != NULL)
    {
        Node->ID = Self;
        WorkThreadFn(Node);
    }

    return true;
}

int NuThreadPoolGetThreadNo(NuThreadPool_t *Pool)
{
    return base_vector_get_cnt(&(Pool->Threads));
}

void NuThreadPoolKill(NuThreadPool_t *Pool, int Signal)
{
    base_vector_t       *Vec = NULL;
    base_vector_it      VecIt;
    NuThreadPoolNode_t  *Node = NULL;

    base_vector_it_set(VecIt, Vec = &(Pool->Threads));
    while(VecIt != base_vector_it_end(Vec))
    {
        Node = (NuThreadPoolNode_t *)(*VecIt);

        if(Node->ID)
        {
            pthread_kill(Node->ID, Signal);
        }

        ++ VecIt;
    }

    return;
}

void NuThreadPoolStop(NuThreadPool_t *Pool)
{
    Pool->Working = false;
    return;
}

void NuThreadPoolFree(NuThreadPool_t *Pool)
{
    base_vector_t       *Vec = NULL;
	NuThreadPoolNode_t *Node = NULL;

    if(Pool != NULL)
    {
		base_vector_it VecIt;
        NuThreadPoolStop(Pool);

        base_vector_it_set(VecIt, Vec = &(Pool->Threads));
        while(VecIt != base_vector_it_end(Vec))
        {
			Node = (NuThreadPoolNode_t *)(*VecIt);
//			printf("join Node->ID %lu\n", Node->ID);
			pthread_join(Node->ID, NULL);
            free(*VecIt);
            ++ VecIt;
        }
        base_vector_destroy(Vec);

        free(Pool);
    }

    return;
}

