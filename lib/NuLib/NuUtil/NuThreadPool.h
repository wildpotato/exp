
#ifndef _NUTHREADPOOL_H
#define _NUTHREADPOOL_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _NuThreadPool_t NuThreadPool_t;
typedef void(*NuThreadPoolFn)(void *Argu);

int NuThreadPoolNew(NuThreadPool_t **pPool, NuThreadPoolFn Fn);
void NuThreadPoolAddThread(NuThreadPool_t *Pool, void *Argu);
void NuThreadPoolAddThreads(NuThreadPool_t *Pool, int ThreadNo, void *Argu);
bool NuThreadPoolPutSelfIntoPool(NuThreadPool_t *Pool, void *Argu);
int NuThreadPoolGetThreadNo(NuThreadPool_t *Pool);
void NuThreadPoolKill(NuThreadPool_t *Pool, int Signal);
void NuThreadPoolStop(NuThreadPool_t *Pool);
void NuThreadPoolFree(NuThreadPool_t *Pool);

#ifdef __cplusplus
}
#endif

#endif /* _NUTHREADPOOL_H */

