
#include <pthread.h>

static int NuLock_SpinNew(void *Obj)
{
    pthread_spinlock_t *pspin = (pthread_spinlock_t *)malloc(sizeof(pthread_spinlock_t));

    if(!pspin)
    {
        return NU_MALLOC_FAIL;
    }

    pthread_spin_init(pspin, 0);

    *((pthread_spinlock_t **)Obj) = pspin;

    return NU_OK;
}

static int NuLock_SpinFree(void *Obj)
{
    pthread_spin_destroy((pthread_spinlock_t *)Obj);
    free(Obj);

    return NU_OK;
}

static void NuLock_SpinLock(void *Obj)
{
    pthread_spin_lock((pthread_spinlock_t *)Obj);
    return;
}

static void NuLock_SpinUnLock(void *Obj)
{
    pthread_spin_unlock((pthread_spinlock_t *)Obj);
    return;
}

NuLockType_t NuLockType_Spin = {
                                    .Lock = &NuLock_SpinLock,
                                    .UnLock = &NuLock_SpinUnLock,
                                    .Constructor = &NuLock_SpinNew,
                                    .Destructor = &NuLock_SpinFree
                               };

