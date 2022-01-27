
#include "NuCommon.h"

#ifndef _NULOCK_H
#define _NULOCK_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*NuLockFn)(void *);

typedef struct _NuLockType_t
{
    NuLockFn        Lock;
    NuLockFn        UnLock;
    NuConstructor   Constructor;
    NuDestructor    Destructor;
} NuLockType_t;

typedef struct _NuLock_t
{
    void            *Obj;
    NuLockType_t    *Type;
} NuLock_t;

extern NuLockType_t NuLockType_NULL;
extern NuLockType_t NuLockType_Mutex;
extern NuLockType_t NuLockType_Spin;

int NuLockInit(NuLock_t *Lock, NuLockType_t *Type);
void NuLockDestroy(NuLock_t *Lock);
void NuLockLock(NuLock_t *Lock);
void NuLockUnLock(NuLock_t *Lock);

#ifdef __cplusplus
}
#endif

#endif /* _NULOCK_H */

