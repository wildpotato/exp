#ifndef _NUMUTEX_H
#define _NUMUTEX_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>

#include "NuCommon.h"
#include "NuUtil.h"

typedef pthread_mutex_t NuMutex_t;

int NuMutexInit(NuMutex_t *mtx);

/* SHARED    : mtx 實體需透過mmap對應到同一個位置, 則可透過SHARED模式跨process進行lock
 * RECURSIVE : mtx 在同一個thread id中可以多次重複lock, 也得做相對應次數的unlock
 * */
#define NuMutexType_NONE       0
#define NuMutexType_SHARED     1 << 0
#define NuMutexType_RECURSIVE  1 << 1
int NuMutexInit2(NuMutex_t *mtx, int type);

int NuMutexInitWithShared(NuMutex_t *mtx);

NU_ATTR_INLINE 
static inline void NuMutexDestroy(NuMutex_t *mtx) {
    pthread_mutex_destroy(mtx);
}

NU_ATTR_INLINE 
static inline void NuMutexLock(NuMutex_t *mtx) {
    pthread_mutex_lock(mtx);
}

NU_ATTR_INLINE 
static inline void NuMutexUnLock(NuMutex_t *mtx) {
    pthread_mutex_unlock(mtx);
}

/* locak guard */
NU_ATTR_INLINE 
static inline void _NuMutexAutoUnLockCBFn_(void *ptr) { \
		NuMutexUnLock( (NuMutex_t *)(*(void **)ptr) );	\
	}

#define NuMutexAutoUnLock   __attribute__((cleanup(_NuMutexAutoUnLockCBFn_)))

/* m : NuMutex_t pointer, it will auto lock and unlock when in/out scope */
#define NuMutexLockGuard(m)  NuMutexAutoUnLock NuMutex_t *_numutex_lock_ptr_ = (m); NuMutexLock(_numutex_lock_ptr_)

#ifdef __cplusplus
}
#endif

#endif /* _NUMUTEX_H */

