
#include "NuCommon.h"
#include "NuBlocking.h"

#ifndef _NUINVOKER_H
#define _NUINVOKER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _NuInvokerGroup_t NuInvokerGroup_t;

typedef struct _NuInvokerDelegate_t
{
    NuEventFn   Fn;
    void        *Argu;
} NuInvokerDelegate_t;

NuInvokerGroup_t *NuInvokerAddGroup(NuBlockingType_t *BlockingType, void *BlockingHdlr, int ThreadNo, int MaxThreadNo);
void NuInvokerInvoke(NuInvokerGroup_t *Group, NuInvokerDelegate_t *Delegate);
void NuInvokerRemove(NuInvokerGroup_t *Group);
void NuInvokerStop();

#ifdef __cplusplus
}
#endif

#endif /* _NUINVOKER_H */

