
#include "NuCommon.h"
#include "NuUtil.h"
#include "NuAlloc.h"


/* defination       */
/* ----------------------------------------------------------- */
struct _NuAlloc_t 
{
	size_t        initSz;
	size_t        extCnt;
	base_vector_t vMem; 
};

typedef struct _NuAllocBuf_t 
{
	size_t Length;
	char   Data[0];
} NuAllocBuf_t;

/* Private function */
/* ----------------------------------------------------------- */
int _alloc_extend(NuAlloc_t *alloc, int cnt, size_t sz)
{
	int i = 0;
	size_t len = sizeof(NuAllocBuf_t) + sz;
	NuAllocBuf_t *buffer = NULL;

	for (i = 0; i < cnt; i++)
	{
		buffer = NULL;
		if ((buffer = (NuAllocBuf_t *)malloc(len)) == NULL)
		{
			return NU_MALLOC_FAIL;
		}

		memset(buffer, 0x0, len);

		buffer->Length = sz;

		base_vector_push(&(alloc->vMem), (void *)buffer);
	}

	return NU_OK;
}

/* Public function  */
/* ----------------------------------------------------------- */
NuAlloc_t *NuAllocOpen(int item_cnt, size_t init_sz)
{
	NuAlloc_t *alloc = NULL;
	
	alloc = (NuAlloc_t *)malloc(sizeof(NuAlloc_t));
	if (alloc != NULL)
	{
		alloc->initSz = init_sz;
		alloc->extCnt = (item_cnt < 16) ? 16 : item_cnt ;
		base_vector_init(&(alloc->vMem), item_cnt * 2);

		if (_alloc_extend(alloc, item_cnt, init_sz) != NU_OK)
		{
			NuAllocClose(alloc);
			alloc = NULL;
		}
	}

	return alloc;
}

void NuAllocClose(NuAlloc_t *alloc)
{
	NuAllocBuf_t *ptr = NULL;

	if (alloc == NULL)
	{
		return;
	}

	while(base_vector_pop(&(alloc->vMem), (void **)&ptr) != NU_EMPTY)
	{
		free(ptr);
	}
	base_vector_destroy(&(alloc->vMem));

	free(alloc);
	return;
}

NuAllocPtr_t *NuAllocGet(NuAlloc_t *alloc, size_t sz)
{
	NuAllocBuf_t *ptr = NULL;

	if (base_vector_get_cnt(&(alloc->vMem)) == 0)
	{
		if (_alloc_extend(alloc, alloc->extCnt, alloc->initSz) != NU_OK)
		{
			return NULL;
		}
	}

	base_vector_pop(&(alloc->vMem), (void **)&ptr);
	
	if (ptr->Length < sz)
	{
		ptr = (NuAllocBuf_t *)realloc(ptr, sz + sizeof(NuAllocBuf_t));
		ptr->Length = sz;
	}

	return (NuAllocPtr_t *)(ptr->Data);
}

void NuAllocPut(NuAlloc_t *alloc, NuAllocPtr_t *alloc_ptr)
{
	NuAllocBuf_t *p = NuContainerOf((void *)alloc_ptr, NuAllocBuf_t, Data);
	base_vector_push(&(alloc->vMem), (void *)p);
	return;
}

size_t NuAllocPtrGetSize(NuAllocPtr_t *alloc_ptr)
{
	NuAllocBuf_t *p = NuContainerOf((void *)alloc_ptr, NuAllocBuf_t, Data);
	return p->Length;
}

NuAllocPtr_t *NuAllocPtrExtend(NuAllocPtr_t *alloc_ptr, size_t len)
{
    NuAllocBuf_t *tmp = NULL;
	NuAllocBuf_t *p = NuContainerOf((void *)alloc_ptr, NuAllocBuf_t, Data);
	
	if (p->Length < len)
	{
		tmp = (NuAllocBuf_t *)realloc(p, (sizeof(size_t) + len));
		if (tmp == NULL)
		{
			return NULL;
		}
	}

	return (NuAllocPtr_t *)(tmp->Data);
}



