
#include <dirent.h>
#include <ctype.h>

#include "NuStr.h"
#include "NuCommon.h"

#ifndef _NUFILE_H
#define _NUFILE_H

#ifdef __cplusplus
extern "C" {
#endif

/* macro */
/* -------------------------------------------------------------------- */
#ifndef NUFILE_SEPARATOR
#define NUFILE_SEPARATOR        '/'
#endif
#ifndef NUFILE_SEPARATOR_STR
#define NUFILE_SEPARATOR_STR    "/"
#endif

#ifndef NULINE_END
#define NULINE_END              '\n' 
#endif

#ifndef NULINE_END_STR
#define NULINE_END_STR          "\n"
#endif

bool NuIsDir(const char *Path);
bool NuIsFile(const char *Path);
void NuPathCombine(NuStr_t *Str, const char *DirPath, const char *Name);

void NuCreateDir(const char *pPath);
void NuCreateRecursiveDir(const char *pPath);

size_t NuFileGetSize(const int fd_no);
int NuFileSetSize(int fd_no, unsigned int len);

typedef void (*NuFileFn)(const char *Name, void *Argu);
void NuFileScanDir(const char *Path, NuFileFn Fn, void *Argu);

#ifdef __cplusplus
}
#endif

#endif /* _NUFILE_H */

