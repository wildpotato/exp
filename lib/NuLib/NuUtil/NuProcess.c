
#ifdef __linux__
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#endif
#include <sched.h>

#include "NuStr.h"
#include "NuCStr.h"
#include "NuFile.h"

static void Fork(void)
{
    pid_t   Pid = 0;

    if((Pid = fork()) < 0)
    {
        exit(-1);
    }
    else if(Pid != 0)
    {
        exit(0);
    }

    return;
}

void NuProcessBeDaemon(void)
{
    Fork();

    setsid();

    Fork();

    umask(027);

    return;
}

void NuProcessWritePidFile(const char *Path, const char *procName, const char *Instance)
{
    FILE    *File = NULL;
    NuStr_t *Str = NULL;

    NuCreateRecursiveDir(Path);

    NuStrNew(&Str, Path);

    NuPathCombine(Str, Path, procName);

    if(Instance != NULL)
    {
        if(*Instance != '\0')
        {
            NuStrCatChr(Str, '_');
            NuStrCat(Str, Instance);
        }
    }

    NuStrCat(Str, ".pid");

    if((File = fopen(NuStrGet(Str), "w+")) != NULL)
    {
        fprintf(File, "%d\n", getpid());
        fclose(File);
    }

    NuStrFree(Str);

    return;
}

int NuProcessBindCore(int CpuID)
{
    int iRC = 0;
#ifdef __linux__
	cpu_set_t cpuset;

	if(CpuID <= CPU_SETSIZE) {
	    CPU_ZERO(&cpuset);
	    CPU_SET(CpuID, &cpuset);
	    /* set cpu handle this process 
	    * If pid is zero, then the calling process is used
	    * */
	    iRC = sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);
    } else {
        iRC = -1;
    }
#endif
	return iRC;	
}

int NuProcessBindCoreWithArray(int *cpuid_ary, int cpuid_sz)
{
    int iRC = 0;
#ifdef __linux__
    int i = 0;
	cpu_set_t cpuset;

    CPU_ZERO(&cpuset);
    for (i = 0; i < cpuid_sz; i++) {
        if(cpuid_ary[i] <= CPU_SETSIZE) {
            CPU_SET(cpuid_ary[i], &cpuset);
        }
    }

    if (CPU_COUNT(&cpuset) > 0) {
	    iRC = sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);
    } else {
        iRC = -1;
    }

#endif
    return iRC;
}

static void splitCpuID(unsigned int idx, const char *msg, size_t msgSz, void *argu) 
{
    int cpuid = 0;
    cpu_set_t *cst = (cpu_set_t *)argu;
    cpuid = NuCStrToInt(msg, msgSz);

    if(cpuid <= CPU_SETSIZE) {
        CPU_SET(cpuid, cst);
    }

}

int NuProcessBindCoreWithString(const char *sCpuID)
{
    int iRC = 0;
#ifdef __linux__
	cpu_set_t cpuset;

    CPU_ZERO(&cpuset);

    NuCStrSplit(sCpuID, ',', strlen(sCpuID), &splitCpuID, &cpuset);

    if (CPU_COUNT(&cpuset) > 0) {
	    iRC = sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);
    } else {
        iRC = -1;
    }

#endif
    return iRC;
}

int  NuSchedGetCpu() { 
#ifdef __linux__
    return sched_getcpu(); 
#else
    return 0;
#endif
}
