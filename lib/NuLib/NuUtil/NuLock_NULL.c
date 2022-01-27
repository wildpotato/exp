
static int NuLock_NULLFn(void *Obj)
{
    return NU_OK;
}

static void NuLock_NULLOp(void *Obj)
{
    return;
}

NuLockType_t NuLockType_NULL = {
                                    .Lock = &NuLock_NULLOp,
                                    .UnLock = &NuLock_NULLOp,
                                    .Constructor = &NuLock_NULLFn,
                                    .Destructor = &NuLock_NULLFn
                               };

