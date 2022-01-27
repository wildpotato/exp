
#include "NuCommon.h"
#include "NuUtil.h"

#ifndef _NUMEMORY_H
#define _NUMEMORY_H

#ifdef __cplusplus
extern "C" {
#endif

/* 自動成長大小的記憶體
 * note : 函式都不額外判斷傳入物件是否為NULL, 使用者須自行注意
 * */
typedef struct _NuMemory_t {
    char    *data;
    size_t  cap;
    size_t  length;
} NuMemory_t;

#define NuMemoryPtr(M)    (const char *)((M)->data)
#define NuMemoryCap(M)    (M)->cap
#define NuMemoryLength(M) (M)->length

int  NuMemoryNew(NuMemory_t **mem, size_t initCap);
void NuMemoryFree(NuMemory_t *mem);

NU_ATTR_INLINE 
static inline void NuMemoryClear(NuMemory_t *mem) { 
	mem->length = 0; 
}

int NuMemoryAppend(NuMemory_t *mem, void *data, size_t dataLen);

#ifdef __cplusplus
}
#endif

#endif /* _NUMEMORY_H */

