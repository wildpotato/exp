
#include "NuCommon.h"

#ifndef _NUTIMER_H
#define _NUTIMER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _NuTimerEventHdlr_t NuTimerEventHdlr_t;

int NuTimerNew(int Self);
void NuTimerStop(void);
void NuTimerFree(void);
NuTimerEventHdlr_t *NuTimerRegister(int Period, time_t Start, NuEventFn CBFn, void *CBArgu);
void NuTimerUnregister(NuTimerEventHdlr_t *pEventHdlr);
time_t NuTimerGetNow(void);

#ifdef __cplusplus
}
#endif

#endif /* _NUTIMER_H */

