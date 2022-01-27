
#ifndef _NUCBMGR_H
#define _NUCBMGR_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _NuCBMgr_t NuCBMgr_t;
typedef struct _NuCBMgrHdlr_t NuCBMgrHdlr_t;

typedef void (*NuCBMgrFn)(const void *RaiseArgu, void *Argu);

NuCBMgr_t *NuCBMgrAdd(void);
void NuCBMgrDel(NuCBMgr_t *Mgr);
void NuCBMgrReset(NuCBMgr_t *Mgr);

NuCBMgrHdlr_t *NuCBMgrRegisterEvent(NuCBMgr_t *Mgr, NuCBMgrFn Fn, void *Argu);
void NuCBMgrUnRegisterEvent(NuCBMgrHdlr_t *Hdlr);
void NuCBMgrRaiseEvent(NuCBMgr_t *Mgr, const void *RaiseArgu);

/* Parallel work via OpenMP 
 * if you want to turn on this function, please install libgomp and add -fopenmp to your makefile
 * note that : the basic expenses is 250 us in test machine. 
 *             if your work over 250 us, it helpful.
 * */
void NuCBMgrRaiseEventParallel(NuCBMgr_t *Mgr, const void *RaiseArgu);

#ifdef __cplusplus
}
#endif

#endif /* _NUCBMGR_H */

