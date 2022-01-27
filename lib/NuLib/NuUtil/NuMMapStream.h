#ifndef _NUMMAPSTREAM_H
#define _NUMMAPSTREAM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <math.h>
#include <fcntl.h>

#include <ctype.h>
#include <sys/mman.h>
#include <unistd.h>                                                                                                          
#include <sys/stat.h>

#include "NuCommon.h"
#include "NuUtil.h"
#include "NuFile.h"
#include "NuStr.h"

#define PAGE_READWRITE         (PROT_READ|PROT_WRITE)
#define FILE_MAP_ALL_ACCESS    (MAP_SHARED)
#define FILE_MAP_PRIVATE       (MAP_PRIVATE|MAP_ANONYMOUS)

#define MMAP_END               EOF
#define MMAP_END_SZ            sizeof(int)

/* --------------------------------------------------------------------------------------- 
 * NuMMapStream
 * ---------------------------------------------------------------------------------------*/
/** 
 * A structure to memory map stream
 */ 
typedef struct _NuMPStrm_t
{
	void         *addr;
	void         *start_addr;
	size_t        fsz;          /* file size     */
	int           prot;
	int           flags;
	FILE         *fd;
	int           fd_no;
	NuStr_t      *MFile;
    NuStr_t      *VStrTmp;
} NuMPStrm_t;

/** 
 * mmap stream create function
 * @param[out] pMPStrm : output struct pointer
 * @param[in ] pMFile : mapping file path , include file name
 * @param[in ] len    : expect mapping size, if the file size less, it will be truncate by this parameter
 * @param[in ] prot   : mmap prot parameter 
 * @param[in ] flags  : mmap flags parameter 
 */ 
int NuMPStrmNew(NuMPStrm_t **pMPStrm, const char *pMFile, char *mode, size_t len, int prot, int flags);

/** 
 * mmap stream free function
 * @param[in ] pMPStrm  : struct pointer
 */ 
void NuMPStrmFree(NuMPStrm_t *pMPStrm);
void NuMPStrmFree2(NuMPStrm_t *pMPStrm, size_t len);

#define NuMPStrmGetSize(pMPStrm)           (pMPStrm)->fsz
#define NuMPStrmGetAddr(pMPStrm)           (pMPStrm)->addr
#define NuMPStrmGetStartAddr(pMPStrm)      (pMPStrm)->start_addr
#define NuMPStrmSeekToTop(pMPStrm)         ((pMPStrm)->addr = (pMPStrm)->start_addr)

/**
 * sync mmap stream data to file
 * @param[in ] pMPStrm : struct pointer
 * @param[in ] flags  : MPS_ASYNC specifies that an update be scheduled, but the call returns immediately.  MPS_SYNC asks for an update and waits for it to complete.  MPS_INVALIDATE asks to invalidate other mappings of  the  same file.
 */
#define NuMPStrmSync(pMPStrm, flags)  msync((pMPStrm)->start_addr, (pMPStrm)->fsz, (flags))

/**
 * Write Data to mmap stream file 
 * @param[in ] pMPStrm : struct pointer
 * @param[in ] data   : write data
 * @param[in ] len    : data length   
 */
int NuMPStrmWriteN(NuMPStrm_t *pMPStrm, const char *data, size_t len);
int NuMPStrmVPrintf(NuMPStrm_t *pMPStrm, const char *fmt, va_list ap);

/**
 * Write Data to mmap stream file and append newline char at line end
 * @param[in ] pMPStrm : struct pointer
 * @param[in ] data   : write data
 * @param[in ] len    : data length   
 */
int NuMPStrmWriteLine(NuMPStrm_t *pMPStrm, char *data, size_t len);

/**
 * Get memort from mmap file 
 * @param[in ] pMPStrm : struct pointer
 * @param[in ] len    : data length   
 * @param[out] pmem   : write data
 */
int NuMPStrmGet(NuMPStrm_t *pMPStrm, size_t len, void **pmem);

#ifdef __cplusplus
}
#endif

#endif /* _NUMMAPSTREAM_H */

