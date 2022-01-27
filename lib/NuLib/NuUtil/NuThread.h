#ifndef _NUTHREAD_H
#define _NUTHREAD_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#include <process.h>
#else
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <pthread.h>
#include <signal.h>
#include <sched.h>
#endif
#include "NuCommon.h"

typedef void *VOID_PTR;
typedef VOID_PTR (* NuThdFn)(void *); 
#define NUTHD_FUNC VOID_PTR


typedef pthread_t NuThread_t;

/* -----------------------------------------------------------------*/
int	NuThdCreate(NuThdFn Proc_fn, void *arg , NuThread_t *ThdHdl);
int	NuThdCreate2(NuThdFn Proc_fn, void *arg ,unsigned int StackSize, NuThread_t *ThdHdl);
int	NuThdJoin(NuThread_t ThdHdl);
int	NuThdDetach(NuThread_t ThdHdl);
int	NuThdKill(NuThread_t ThdHdl, int Sig);
int NuThdReturn();
NuThread_t NuThdSelf();
int NuThdYield();

int NuThdBindCore(int cpuID);
int NuThdBindCoreWithArray(int *cpuID_ary, int cpuID_sz);
int NuThdBindCoreWithString(const char *cpuID);

#ifdef __cplusplus
}
#endif

#endif /* _NUTHREAD_H */

