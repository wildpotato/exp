
#include "NuCommon.h"

#ifndef _NUVARIABLE_H
#define _NUVARIABLE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _NuVariable_t NuVariable_t;

int NuVariableNew(NuVariable_t **Var, size_t Size);
void NuVariableFree(NuVariable_t *Var);

void NuVariableExtend(NuVariable_t *Var, size_t Size);
void NuVariableCopy(NuVariable_t *Var, const void *Data, size_t DataLen);
void NuVariableSet(NuVariable_t *Var, size_t Offset, const void *Data, size_t DataLen);
void NuVariableSetByte(NuVariable_t *Var, size_t Offset, char Data);
void NuVariableMove(NuVariable_t *Var, size_t Offset, size_t Src, size_t DataLen);
int NuVariablePrintf(NuVariable_t *Var, size_t Offset, const char *Format, ...);
int NuVariableVPrintf(NuVariable_t *Var, size_t Offset, const char *Format, va_list ArguList);

const void *NuVariableGet(NuVariable_t *Var, size_t Offset);
size_t NuVariableSize(NuVariable_t *Var);

#ifdef __cplusplus
}
#endif

#endif /* _NUVARIABLE_H */

