
#include "NuLock.h"
#include "NuLock_NULL.c"
#include "NuLock_Mutex.c"
#include "NuLock_Spin.c"

int NuLockInit(NuLock_t *Lock, NuLockType_t *Type)
{
    Lock->Obj = NULL;
    Lock->Type = Type;

    if(Type->Constructor(&(Lock->Obj)) != NU_OK)
    {
        return NU_FAIL;
    }

    return NU_OK;
}

void NuLockDestroy(NuLock_t *Lock)
{
    Lock->Type->Destructor(Lock->Obj);
    return;
}

void NuLockLock(NuLock_t *Lock)
{
    Lock->Type->Lock(Lock->Obj);
    return;
}

void NuLockUnLock(NuLock_t *Lock)
{
    Lock->Type->UnLock(Lock->Obj);
    return;
}

