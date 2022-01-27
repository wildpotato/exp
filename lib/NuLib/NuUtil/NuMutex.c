
#include "NuMutex.h"

int NuMutexInit(NuMutex_t *mtx) {
    return pthread_mutex_init(mtx, NULL);
}

int NuMutexInitWithShared(NuMutex_t *mtx) {

    pthread_mutexattr_t mtx_attr;
    pthread_mutexattr_init(&mtx_attr);
    pthread_mutexattr_setpshared(&mtx_attr, PTHREAD_PROCESS_SHARED);

    return pthread_mutex_init(mtx, &mtx_attr);
}

int NuMutexInit2(NuMutex_t *mtx, int type) {
	pthread_mutexattr_t mtx_attr;

	if (type == NuMutexType_NONE) {
		return pthread_mutex_init(mtx, NULL);
	} else { 
		pthread_mutexattr_init(&mtx_attr);

		if (NuBITCHK(type, NuMutexType_SHARED)) {
			pthread_mutexattr_setpshared(&mtx_attr, PTHREAD_PROCESS_SHARED);
		}

		if (NuBITCHK(type, NuMutexType_RECURSIVE)) {
			pthread_mutexattr_settype(&mtx_attr, PTHREAD_MUTEX_RECURSIVE);
		}

		return pthread_mutex_init(mtx, &mtx_attr);
	}
}
