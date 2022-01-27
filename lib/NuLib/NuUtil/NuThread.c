#include "NuThread.h"
#include "NuCStr.h"

/* Thread function */
/* ====================================================================== */
int  NuThdCreate(NuThdFn Proc_fn, void *arg , NuThread_t *ThdHdl)
{
    NuThread_t tid;
    if ( pthread_create( &tid, NULL, (void *(*)(void *))Proc_fn, arg ) != 0 ) 
        return NU_FAIL;

    memcpy(ThdHdl, &tid, sizeof(NuThread_t));

    return NU_OK;
}

int  NuThdCreate2(NuThdFn Proc_fn, void *arg ,unsigned int StackSize, NuThread_t *ThdHdl)
{
    NuThread_t tid;
	pthread_attr_t ThreadAttr;
	if ( pthread_attr_init(&ThreadAttr) != 0)
		return NU_FAIL;

	if (StackSize < 128*1024)
		StackSize = 128*1024;

	if (pthread_attr_setstacksize(&ThreadAttr, StackSize) != 0)
		return NU_FAIL;

    if ( pthread_create( &tid, &ThreadAttr, (void *(*)(void *))Proc_fn, arg ) != 0 ) 
        return NU_FAIL;
    memcpy(ThdHdl, &tid, sizeof(NuThread_t));

    return NU_OK;
}

int  NuThdJoin(NuThread_t ThdHdl)
{
  pthread_join((pthread_t)ThdHdl, 0 );
  return NU_OK;
}

int  NuThdDetach(NuThread_t ThdHdl)
{
  	pthread_detach((pthread_t)ThdHdl);
  return NU_OK;
}

int NuThdKill(NuThread_t ThdHdl, int Sig)
{
	return pthread_kill((pthread_t)ThdHdl, Sig);
}

int NuThdReturn()
{
    pthread_exit(0);
    return NU_OK;
}

NuThread_t NuThdSelf()
{
	NuThread_t tid;
    tid = (NuThread_t)pthread_self();
	return tid;
}

int NuThdYield()
{
	return sched_yield();
}

int NuThdBindCore(int cpuID)
{
    int iRC = 0;
#ifdef __linux__
	cpu_set_t cpuset;
	if(cpuID <= CPU_SETSIZE)
    {
	    CPU_ZERO(&cpuset);
	    CPU_SET(cpuID, &cpuset);
        iRC = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    }
#endif
    return iRC;
}

int NuThdBindCoreWithArray(int *cpuID_ary, int cpuID_sz)
{
    int iRC = 0;
#ifdef __linux__
    int i = 0;
	cpu_set_t cpuset;

    CPU_ZERO(&cpuset);
    for (i = 0; i < cpuID_sz; i++) {
        if(cpuID_ary[i] <= CPU_SETSIZE) {
            CPU_SET(cpuID_ary[i], &cpuset);
        }
    }

    if (CPU_COUNT(&cpuset) > 0) {
        iRC = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    }

#endif
    return iRC;
}


static void splitCpuID_thd(unsigned int idx, const char *msg, size_t msgSz, void *argu) 
{
    int cpuid = 0;
    cpu_set_t *cst = (cpu_set_t *)argu;
    cpuid = NuCStrToInt(msg, msgSz);

    if(cpuid <= CPU_SETSIZE) {
        CPU_SET(cpuid, cst);
    }

}

int NuThdBindCoreWithString(const char *cpuID) 
{
    int iRC = 0;
#ifdef __linux__
	cpu_set_t cpuset;

    CPU_ZERO(&cpuset);

    NuCStrSplit(cpuID, ',', strlen(cpuID), &splitCpuID_thd, &cpuset);

    if (CPU_COUNT(&cpuset) > 0) {
        iRC = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    }
#endif
    return iRC;
}

