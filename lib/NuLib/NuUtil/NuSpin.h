#ifndef _NUSPIN_H
#define _NUSPIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>

#include "NuCommon.h"
#include "NuUtil.h"

typedef pthread_spinlock_t NuSpin_t;

int NuSpinInit(NuSpin_t *mtx);

NU_ATTR_INLINE
static inline void NuSpinDestroy(NuSpin_t *mtx) {
    pthread_spin_destroy(mtx);
}

NU_ATTR_INLINE
static inline void NuSpinLock(NuSpin_t *mtx) {
    pthread_spin_lock(mtx);
}

NU_ATTR_INLINE
static inline void NuSpinUnLock(NuSpin_t *mtx) {
    pthread_spin_unlock(mtx);
}

/* locak guard */
NU_ATTR_INLINE 
static inline void _NuSpinAutoUnLockCBFn_(void *ptr) { \
		NuSpinUnLock( (NuSpin_t *)(*(void **)ptr) );	\
	}

#define NuSpinAutoUnLock   __attribute__((cleanup(_NuSpinAutoUnLockCBFn_)))

/* m : NuSpin_t pointer, it will auto lock and unlock when in/out scope */
#define NuSpinLockGuard(m)  NuSpinAutoUnLock NuSpin_t *_nuspin_lock_ptr_ = (m); NuSpinLock(_nuspin_lock_ptr_)

#ifdef __cplusplus
}
#endif

#endif /* _NUSPIN_H */

