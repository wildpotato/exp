
#ifndef _NUDLIBMGR_H
#define _NUDLIBMGR_H

#ifdef __cplusplus
extern "C" {
#endif

int NuDLibMgrLoad(const char *AliasName, const char *DLibPath, int Flag, char **Err);
int NuDLibMgrLoadToGlobal(const char *AliasName, const char *DLibPath, char **Err);
int NuDLibMgrLoadToLocal(const char *AliasName, const char *DLibPath, char **Err);

void NuDLibMgrUnLoad(const char *AliasName, char **Err);
void NuDLibMgrUnLoadAll(void);

void *NuDLibMgrGetFn(const char *AliasName, const char *FnName, char **Err);

#ifdef __cplusplus
}
#endif

#endif /* _NUDLIBMGR_H */

