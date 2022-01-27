#ifndef _NUMMAP_H
#define _NUMMAP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "NuMMapStream.h"

/* --------------------------------------------------------------------------------------- 
 * NuMMap
 * ---------------------------------------------------------------------------------------*/
#define NuMMapReMapSegment   0
#define NuMMapReMapDefault   1

/** 
 * A structure to memory map file
 */ 
typedef struct _NuMMap_t
{
	NuMPStrm_t    *pMPStrm;
	size_t        len;          /* mapping len   */
	size_t        data_len;     /* data size     */
	int           remap_mode;   /* 0 : segment, 1 : all (root) default */
} NuMMap_t;

/** 
 * mmap create function
 * @param[out] pmmap  : output struct pointer
 * @param[in ] pMFile : mapping file path , include file name
 * @param[in ] len    : expect mapping size, if the file size less, it will be truncate by this parameter
 * @param[in ] prot   : mmap prot parameter 
 * @param[in ] flags  : mmap flags parameter 
 */ 
int NuMMapNew(NuMMap_t **pmmap, const char *pMFile, char *mode, size_t len, int prot, int flags);

void NuMMapSetReMapMode(NuMMap_t *pmmap, int iMode);

/** 
 * mmap free function
 * @param[in ] pmmap  : struct pointer
 */ 
void NuMMapFree(NuMMap_t *pmmap);

#define NuMMapGetAddr(pMMAP)           (pMMAP)->pMPStrm->addr
#define NuMMapGetStartAddr(pMMAP)      (pMMAP)->pMPStrm->start_addr

/**
 * sync mmap data to file
 * @param[in ] pmmap  : struct pointer
 * @param[in ] flags  : MPS_ASYNC specifies that an update be scheduled, but the call returns immediately.  MPS_SYNC asks for an update and waits for it to complete.  MPS_INVALIDATE asks to invalidate other mappings of  the  same file.
 */
#define NuMMapSync(pMMAP, flags)  msync((pMMAP)->pMPStrm->start_addr, (pMMAP)->len, (flags))

/**
 * Write Data to mmap file 
 * @param[in ] pmmap  : struct pointer
 * @param[in ] data   : write data
 * @param[in ] len    : data length   
 */
int NuMMapWriteN(NuMMap_t *pmmap, char *data, size_t len);

/**
 * Write Data to mmap file and append newline char at line end
 * @param[in ] pmmap  : struct pointer
 * @param[in ] data   : write data
 * @param[in ] len    : data length   
 */
int NuMMapWriteLine(NuMMap_t *pmmap, char *data, size_t len);

/**
 * Get memort from mmap file 
 * @param[in ] pmmap  : struct pointer
 * @param[in ] len    : data length   
 * @param[out] pmem   : write data
 */
int NuMMapGet(NuMMap_t *pmmap, size_t len, void **pmem);

#ifdef __cplusplus
}
#endif

#endif /* _NUMMAP_H */

