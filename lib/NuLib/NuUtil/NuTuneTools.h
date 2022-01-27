
#include "NuCommon.h"

#ifndef _NUTUNETOOLS_H
#define _NUTUNETOOLS_H

#ifdef __cplusplus
extern "C" {
#endif

void NuTuneSetStart();
void NuTuneSetStop();
time_t NuTuneGetSecElapse();
long NuTuneGetNanoSecElapse();

#ifdef __cplusplus
}
#endif

#endif /* _NUTUNETOOLS_H */
