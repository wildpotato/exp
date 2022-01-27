
/* This function will not be used, 
 * if you using Kamel library, please call NuProcess.h & KamelSignal.h
 * */
#include "signal.h"

#ifndef _NUDAEMON_H
#define _NUDAEMON_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*NuDaemonOnSignal_Fn)(int Sig, void *Argu);

void NuDaemonStart();
void NuDaemonSetOnSignalCB(NuDaemonOnSignal_Fn CB_OnSignal, void *Argu);
void NuDaemonAllowSignal(int Sig);
void NuDaemonAllowAllSignal();
void NuDaemonDenySignal(int Sig);
void NuDaemonDenyAllSignal();
void NuDaemonStop();
int NuDaemonKeepGoing();
int *NuDaemonVaryKeepGoing();

#ifdef __cplusplus
}
#endif

#endif /* _NUDAEMON_H */

