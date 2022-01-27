
#ifndef _NUPROCESS_H
#define _NUPROCESS_H

#ifdef __cplusplus
extern "C" {
#endif

void NuProcessBeDaemon(void);
void NuProcessWritePidFile(const char *Path, const char *procName, const char *Instance);
int  NuProcessBindCore(int CpuID);

int  NuProcessBindCoreWithArray(int *cpuid_ary, int cpuid_sz);
/* CpuID list format : 1,2,3,4,5 */
int  NuProcessBindCoreWithString(const char *sCpuID);

int  NuSchedGetCpu();

#ifdef __cplusplus
}
#endif

#endif /* _NUPROCESS_H */

