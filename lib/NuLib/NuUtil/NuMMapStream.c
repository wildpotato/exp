#include "NuMMapStream.h"

/* functions */
/* ====================================================================== */
int NuMPStrmNew(NuMPStrm_t **pMPStrm, const char *pMFile, char *mode, size_t len, int prot, int flags)
{
	int fd_no = 0;
	int is_new_open = 0;

	len = NU_RALIGN_PAGE(len);

	if(!((*pMPStrm) = (NuMPStrm_t *)malloc(sizeof(NuMPStrm_t))))
    {
		return NU_FAIL;
    }

	if(!pMFile)
	{
		(*pMPStrm)->MFile = NULL;
		fd_no = -1;
		(*pMPStrm)->fd_no = -1;
		(*pMPStrm)->fd = NULL;
	}
	else
	{
		if(NuStrNew(&((*pMPStrm)->MFile), pMFile) < 0)
		{
			free((*pMPStrm));
			return NU_FAIL;
		}

        if (NuStrNewPreAlloc(&((*pMPStrm)->VStrTmp), 1024) , 0)
        {
			NuStrFree((*pMPStrm)->MFile);
			free((*pMPStrm));
			return NU_FAIL;
        }

		/* file exists */
		if(NuIsFile(NuStrGet((*pMPStrm)->MFile)))
        {
			is_new_open = 0;
        }
		else
        {
			is_new_open = 1;
        }

		/* open file for mmap mapping */
		if(!((*pMPStrm)->fd = fopen(NuStrGet((*pMPStrm)->MFile), mode)))
		{
			NuStrFree((*pMPStrm)->MFile);
			NuStrFree((*pMPStrm)->VStrTmp);
			free((*pMPStrm));
			return NU_FAIL;
		}

		fd_no = fileno((*pMPStrm)->fd);
		(*pMPStrm)->fd_no = fd_no;
		if(NuFileGetSize(fd_no) < len)
        {
			NuFileSetSize(fd_no, len);
        }
	}

	(*pMPStrm)->fsz = len;
	//(*pMPStrm)->fd_no = fd_no;
	(*pMPStrm)->prot = prot;
	(*pMPStrm)->flags = flags;
	if(((*pMPStrm)->addr = mmap(NULL, len, prot, flags, fd_no, 0)) == MAP_FAILED)
    {
		goto EXIT;
    }

	(*pMPStrm)->start_addr = (*pMPStrm)->addr;

	if(is_new_open)
    {
		memset((*pMPStrm)->start_addr, 0x00, len);
    }

	return NU_OK;

EXIT:
	if(pMFile != NULL)
	{
		fclose((*pMPStrm)->fd);
		NuStrFree((*pMPStrm)->MFile);
	}

	free((*pMPStrm));
	return NU_FAIL;
}

void NuMPStrmFree(NuMPStrm_t *pMPStrm)
{
	munmap(pMPStrm->start_addr, pMPStrm->fsz);

	if(pMPStrm->MFile != NULL)
	{
		if(pMPStrm->fd != NULL)
        {
			fclose(pMPStrm->fd);
        }

		NuStrFree(pMPStrm->MFile);
        NuStrFree(pMPStrm->VStrTmp);
	}

	free(pMPStrm);

    return;
}

void NuMPStrmFree2(NuMPStrm_t *pMPStrm, size_t len)
{
	munmap(pMPStrm->start_addr, len);

	if(pMPStrm->MFile != NULL)
	{
		if(pMPStrm->fd != NULL)
        {
			fclose(pMPStrm->fd);
        }

		NuStrFree(pMPStrm->MFile);
	}

	free(pMPStrm);

    return;
}

int NuMPStrmWriteN(NuMPStrm_t *pMPStrm, const char *data, size_t len)
{
	memcpy(pMPStrm->addr, data, len);
	//(char *)(pMPStrm->addr) += len;

	pMPStrm->addr = (void *)((char *)(pMPStrm->addr) + len);

	return len;
}

int NuMPStrmVPrintf(NuMPStrm_t *pMPStrm, const char *fmt, va_list ap)
{
    int len = 0;
    NuStr_t *pTmp = pMPStrm->VStrTmp;

    len = NuStrVPrintf(pTmp, 0, fmt, ap);

    if (len > 0) 
    {
        memcpy(pMPStrm->addr, NuStrGet(pTmp), len);
        //(char *)(pMPStrm->addr) += len;
		
		pMPStrm->addr = (void *)((char *)(pMPStrm->addr) + len);
    }

    return len;
}

int NuMPStrmWriteLine(NuMPStrm_t *pMPStrm, char *data, size_t len)
{
	memcpy(pMPStrm->addr, data, len);
	//(char *)(pMPStrm->addr) += len;
	pMPStrm->addr = (void *)((char *)(pMPStrm->addr) + len);
	memcpy(pMPStrm->addr, NULINE_END_STR, 1);
	//(char *)(pMPStrm->addr) += 1;
	pMPStrm->addr = (void *)((char *)(pMPStrm->addr) + 1);

	return (len + 1);
}

int NuMPStrmGet(NuMPStrm_t *pMPStrm, size_t len, void **pmem)
{
	(*pmem) = pMPStrm->addr;
	//(char *)(pMPStrm->addr) += len;
	pMPStrm->addr = (void *)((char *)(pMPStrm->addr) + len);

	return NU_OK;
}

