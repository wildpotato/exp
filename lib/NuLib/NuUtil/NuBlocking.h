
#include "NuCommon.h"

#ifndef _NUBLOCKING_H
#define _NUBLOCKING_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _NuBlocking_t NuBlocking_t;
typedef int (*NuBlockingFn)(void *Hdlr, void *BlockingArgu);

typedef struct _NuBlockingType_t
{
    NuConstructor   Init;
    NuDestructor    Destroy;
    NuBlockingFn    Block;
    NuBlockingFn    Wake;
    NuBlockingFn    WakeAll;
} NuBlockingType_t;

extern NuBlockingType_t NuBlocking_CondVar;
#ifdef _GNU_SOURCE
extern NuBlockingType_t NuBlocking_Polling;
#endif

int NuBlockingNew(NuBlocking_t **Blocking, NuBlockingType_t *Type, void *Hdlr);
void NuBlockingFree(NuBlocking_t *Blocking, void *BlockingArgu);

void NuBlockingWake(NuBlocking_t *Blocking, void *BlockingArgu);
void NuBlockingWakeAll(NuBlocking_t *Blocking, void *BlockingArgu);
int NuBlockingBlock(NuBlocking_t *Blocking, void *BlockingArgu);

#ifdef __cplusplus
}
#endif

#endif /* _NUBLOCKING_H */

