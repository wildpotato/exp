
#include <pthread.h>

static int NuLock_MutexNew(void *Obj)
{
    pthread_mutex_t *pmtx = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));

    if(!pmtx)
    {
        return NU_MALLOC_FAIL;
    }

    pthread_mutex_init(pmtx, NULL);

    *((pthread_mutex_t **)Obj) = pmtx;

    return NU_OK;
}

static int NuLock_MutexFree(void *Obj)
{
    pthread_mutex_t *pmtx = (pthread_mutex_t *)Obj;
    pthread_mutex_destroy(pmtx);
    free(pmtx);

    return NU_OK;
}

static void NuLock_MutexLock(void *Obj)
{
    pthread_mutex_lock((pthread_mutex_t *)Obj);
    return;
}

static void NuLock_MutexUnLock(void *Obj)
{
    pthread_mutex_unlock((pthread_mutex_t *)Obj);
    return;
}

NuLockType_t NuLockType_Mutex = {
                                    .Lock = &NuLock_MutexLock,
                                    .UnLock = &NuLock_MutexUnLock,
                                    .Constructor = &NuLock_MutexNew,
                                    .Destructor = &NuLock_MutexFree
                                };

