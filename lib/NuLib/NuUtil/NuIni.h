
#include "NuCommon.h"
#include "NuUtil.h"
#include "NuStr.h"
#include "NuFile.h"
#include "NuFileStream.h"
#include "NuHash.h"

#ifndef _NUINI_H
#define _NUINI_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _NuIni_t NuIni_t;
typedef void (*NuIniCBFn)(NuIni_t *Ini, const char *Section, const char *Key, const char *Value, void *Argu);

int NuIniNew(NuIni_t **pIni, const char *FilePath);
void NuIniFree(NuIni_t *Ini);
const char *NuIniFind(NuIni_t *Ini, const char *Section, const char *Key);
bool NuIniSectionExist(NuIni_t *Ini, const char *Section);
void NuIniModify(NuIni_t *Ini, const char *Section, const char *Key, const char *Value);
void NuIniSave(NuIni_t *Ini, const char *FilePath);
void NuIniTraverse(NuIni_t *Ini, const char *Section, NuIniCBFn Fn, void *Argu);
const char *NuIniGetFileName(NuIni_t *Ini);

#ifdef __cplusplus
}
#endif

#endif /* _NUINI_H */

