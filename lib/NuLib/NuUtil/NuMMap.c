#include "NuMMap.h"

/* internal functions */
/* ====================================================================== */

static int _ReMMap(NuMMap_t *pmmap)
{
	size_t nfsz = NU_RALIGN_PAGE(pmmap->pMPStrm->fsz * 2);
	size_t lalign = NU_LALIGN_PAGE((pmmap->pMPStrm->fsz - pmmap->data_len));

	if (NuFileSetSize(pmmap->pMPStrm->fd_no, nfsz) != 0) {
		return NU_FAIL;
	}
	munmap(pmmap->pMPStrm->start_addr, pmmap->len);

	pmmap->len = nfsz - lalign;

	pmmap->pMPStrm->addr = pmmap->pMPStrm->start_addr = (void *)mmap(NULL, pmmap->len, pmmap->pMPStrm->prot, 
	                                                               pmmap->pMPStrm->flags, pmmap->pMPStrm->fd_no, lalign);
	if (pmmap->pMPStrm->addr == MAP_FAILED) {
		return NU_MMAPERR;
	}

	//(char *)(pmmap->pMPStrm->addr) += (pmmap->data_len - lalign);
	pmmap->pMPStrm->addr = (void *)((char *)(pmmap->pMPStrm->addr) + (pmmap->data_len - lalign));
	pmmap->pMPStrm->fsz = nfsz;
	return NU_OK;
}

static int _ReMMap2(NuMMap_t *pmmap)
{
	size_t nfsz = 0;

	munmap(pmmap->pMPStrm->start_addr, pmmap->pMPStrm->fsz);

	nfsz = NU_RALIGN_PAGE(pmmap->pMPStrm->fsz * 2);

	if (NuFileSetSize(pmmap->pMPStrm->fd_no, nfsz) != 0)
		return NU_FAIL;

	pmmap->len = nfsz;

	pmmap->pMPStrm->addr = pmmap->pMPStrm->start_addr = (void *)mmap(NULL, nfsz, pmmap->pMPStrm->prot, 
	                                                               pmmap->pMPStrm->flags, pmmap->pMPStrm->fd_no, 0);
	if (pmmap->pMPStrm->addr == MAP_FAILED)
		return NU_MMAPERR;

	//(char *)(pmmap->pMPStrm->addr) += pmmap->data_len;
	pmmap->pMPStrm->addr = (void *)((char *)(pmmap->pMPStrm->addr) + pmmap->data_len);
	pmmap->pMPStrm->fsz = nfsz;

	return NU_OK;
}

static int _MMapRevise(NuMMap_t *pmmap)
{
	if (pmmap->remap_mode == NuMMapReMapSegment)
		return _ReMMap(pmmap);
	else
		return _ReMMap2(pmmap);
}

/* functions */
/* ====================================================================== */
int NuMMapNew(NuMMap_t **pmmap, const char *pMFile, char *mode, size_t len, int prot, int flags)
{
	len = NU_RALIGN_PAGE(len);

	(*pmmap) = (NuMMap_t *)malloc(sizeof(NuMMap_t));
	if ((*pmmap) == NULL)
		return NU_FAIL;

	if (NuMPStrmNew(&((*pmmap)->pMPStrm), pMFile, mode, len, prot, flags) < 0)
		return NU_FAIL;

	(*pmmap)->len = len;
	(*pmmap)->data_len = 0;
	(*pmmap)->remap_mode = NuMMapReMapDefault;

	return NU_OK;
}

void NuMMapSetReMapMode(NuMMap_t *pmmap, int iMode)
{
	pmmap->remap_mode = iMode;
}

void NuMMapFree(NuMMap_t *pmmap)
{
	if (pmmap->remap_mode == NuMMapReMapSegment) 
		NuMPStrmFree2(pmmap->pMPStrm, pmmap->len);
	else
		NuMPStrmFree(pmmap->pMPStrm);

	free(pmmap);
}

int NuMMapWriteN(NuMMap_t *pmmap, char *data, size_t len)
{
	if ( (pmmap->data_len + len) > pmmap->pMPStrm->fsz )
	{
		if (_MMapRevise(pmmap) < 0)
			return NU_FAIL;
	}

	pmmap->data_len += NuMPStrmWriteN(pmmap->pMPStrm, data, len);

	return len;
}

int NuMMapWriteLine(NuMMap_t *pmmap, char *data, size_t len)
{
	if ( (pmmap->data_len + len + 1) > pmmap->pMPStrm->fsz )
	{
		if (_MMapRevise(pmmap) < 0)
			return NU_FAIL;
	}

	pmmap->data_len += NuMPStrmWriteLine(pmmap->pMPStrm, data, len);

	return len;
}

int NuMMapGet(NuMMap_t *pmmap, size_t len, void **pmem)
{
	if ( (pmmap->data_len + len + 1) > pmmap->pMPStrm->fsz )
	{
		if (_MMapRevise(pmmap) < 0)
			return NU_FAIL;
	}

	NuMPStrmGet(pmmap->pMPStrm, len, pmem);
	pmmap->data_len += len;

	return NU_OK;
}



