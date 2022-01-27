#include "NuUtil.h"

/* single link list */
/* ----------------------------------------------------------- */
void base_single_node_init(base_single_node_t *pnode)
{
    pnode->next = NULL;
    pnode->list = NULL;

    return;
}

void base_single_init(base_single_t *plist)
{
    plist->next = NULL;
    plist->cnt  = 0;

    return;
}

int base_single_insert(base_single_t *plist, base_single_node_t  *pnode)
{
    pnode->next = plist->next;
    plist->next = pnode;

    pnode->list = plist;

    return ++ (plist->cnt);
}

int base_single_remove(base_single_t *plist, base_single_node_t **pnode)
{
    if (!(plist->cnt))
    {
        return NU_EMPTY;
    }

    (*pnode) = plist->next;

    plist->next = plist->next->next;

    return -- (plist->cnt);
}

/* double link list */
/* ----------------------------------------------------------- */
void base_list_node_init(base_list_node_t *pnode)
{
    pnode->next = pnode->prev = NULL;
    pnode->list = NULL;

    return;
}

void base_list_init(base_list_t *plist)
{
    plist->head = plist->tail = NULL;
    plist->cnt = 0;

    return;
}

int base_list_insert_head(base_list_t *plist, base_list_node_t *pnode)
{
    pnode->list = plist;
    pnode->next = plist->head;
    pnode->prev = NULL;

    if(!(plist->cnt))
    {
        plist->tail = pnode;
    }
    else
    {
        plist->head->prev = pnode;
    }

    plist->head = pnode;
    ++ (plist->cnt);

    return plist->cnt;
}

int base_list_remove_head(base_list_t *plist, base_list_node_t **pnode)
{
    if(!(plist->cnt))
    {
        return NU_EMPTY;
    }

    (*pnode) = plist->head;
    plist->head = plist->head->next;

    if(!(plist->head))
    {
        plist->tail = NULL;
    }
    else
    {
        plist->head->prev = NULL;
    }

    -- (plist->cnt);

    return plist->cnt;
}

void *base_list_remove_head2(base_list_t *plist)
{
    //int                 RC = NU_OK;
    base_list_node_t    *pnode = NULL;

    //if((RC = base_list_remove_head(plist, &pnode)) < 0)
    if(base_list_remove_head(plist, &pnode) < 0)
    {
        return NULL;
    }

    return pnode;
}

int base_list_insert_tail(base_list_t *plist, base_list_node_t *pnode)
{
    pnode->list = plist;
    pnode->prev = plist->tail;
    pnode->next = NULL;

    if(!(plist->cnt))
    {
        plist->head = pnode;
    }
    else
    {
        plist->tail->next = pnode;
    }
    
    plist->tail = pnode;

    ++ (plist->cnt);

    return plist->cnt;
}

int base_list_remove_tail(base_list_t *plist, base_list_node_t **pnode)
{
    if(!(plist->cnt))
    {
        return NU_EMPTY;
    }

    (*pnode) = plist->tail;

    plist->tail = plist->tail->prev;

    if(!(plist->tail))
    {
        plist->head = NULL;
    }
    else
    {
        plist->tail->next = NULL;
    }

    -- (plist->cnt);

    return plist->cnt;
}

int base_list_remove_node(base_list_node_t *pnode)
{
    base_list_t *plist = pnode->list;

    if(!plist)
    {
        return NU_FAIL;
    }

    if(!(pnode->prev))
    {
        plist->head = pnode->next;
    }
    else
    {
        pnode->prev->next = pnode->next;
    }

    if(!(pnode->next))
    {
        plist->tail = pnode->prev;
    }
    else
    {
        pnode->next->prev = pnode->prev;
    }

    -- (plist->cnt);

    return plist->cnt;
}

int base_list_transfer_head(base_list_t *psrc, base_list_t *pdes)
{
    if(!(psrc->cnt))
    {
        return NU_EMPTY;
    }

    if(!(pdes->cnt))
    {
        memcpy(pdes, psrc, sizeof(base_list_t));
    }
    else
    {
        pdes->head->prev = psrc->tail;
        psrc->tail->next = pdes->head;

        pdes->head = psrc->head;
        pdes->cnt += psrc->cnt;
    }

    base_list_init(psrc);

    return pdes->cnt;
}

int base_list_transfer_tail(base_list_t *psrc, base_list_t *pdes)
{
    if(!(psrc->cnt))
    {
        return NU_EMPTY;
    }

    if(!(pdes->cnt))
    {
        memcpy(pdes, psrc, sizeof(base_list_t));
    }
    else
    {
        pdes->tail->next = psrc->head;
        psrc->head->prev = pdes->tail;

        pdes->tail = psrc->tail;
        pdes->cnt += psrc->cnt;
    }

    base_list_init(psrc);

    return pdes->cnt;
}

int base_list_items_cnt(base_list_t *plist)
{
    return plist->cnt;  
}

/* vector (LIFO) */
/* ----------------------------------------------------------- */
int base_vector_init(base_vector_t *pvec, int num)
{
    int RC = NU_OK;
	/* add 2 items for begin and end use */
	int total = num + 2;

    pvec->items = NULL;
    pvec->items = (const void **)malloc(sizeof(const void *) * total);
    if(!(pvec->items))
    {
        NUGOTO(RC, NU_MALLOC_FAIL, EXIT);
    }

    pvec->current = pvec->items + 1;
    pvec->num = 0;
    pvec->anum = num;

EXIT:
    return RC;
}

int base_vector_new(base_vector_t **pvec, int num)
{
    int RC = NU_OK;
	/* add 2 items for begin and end use */

    (*pvec) = NULL;
    (*pvec) = (base_vector_t *)malloc(sizeof(base_vector_t));
    if(!(*pvec))
    {
        NUGOTO(RC, NU_MALLOC_FAIL, EXIT);
    }

    if((RC = base_vector_init(*pvec, num)) < 0)
    {
		goto EXIT;
    }

EXIT:
    if(RC < 0)
    {
        base_vector_free(*pvec);
    }

    return RC;
}

void base_vector_destroy(base_vector_t *pvec)
{
    if(pvec->items != NULL)
    {
        free(pvec->items);
    }

    pvec->current = pvec->items + 1;
    pvec->num = pvec->anum = 0;

    return;
}

void base_vector_free(base_vector_t *pvec)
{
    if(pvec != NULL)
    {
        base_vector_destroy(pvec);

        free(pvec);
    }

    return;
}

void base_vector_clear(base_vector_t *pvec)
{
    pvec->current = pvec->items + 1;
    pvec->num = 0;

    return;
}

int base_vector_push(base_vector_t *pvec, const void *pdata)
{
    if(pvec->num >= pvec->anum)
    { /* expand */
        pvec->items = (const void **)realloc(pvec->items, sizeof(const void *) * ((pvec->anum * 2) + 2));
        if(pvec->items == NULL)
        {
            return NU_MALLOC_FAIL;
        }

        pvec->anum *= 2;
        pvec->current = pvec->items + 1 + pvec->num;
    }

    *(pvec->current) = pdata;
    ++ pvec->current;

    return (pvec->num) ++;
}

int base_vector_pop(base_vector_t *pvec, void **pdata)
{
    if(!(pvec->num))
    {
        return NU_EMPTY;
    }

    *pdata = (void *)(*(--(pvec->current)));
    return -- (pvec->num);
}

const void *base_vector_change_by_index(base_vector_t *pvec, int idx, const void *pdata)
{
	int         index = idx + 1;
    const void  *ptmp = pvec->items[index];

    pvec->items[index] = pdata;

    return ptmp;
}

void base_vector_foreach(base_vector_t *pvec, NuEventFn fn)
{
    base_vector_it  VecIt;

    base_vector_it_set(VecIt, pvec);
    while(VecIt != base_vector_it_end(pvec))
    {
        fn(*VecIt);
        ++ VecIt;
    }

    return;
}

int base_vector_foreach2(base_vector_t *pvec, int (*Fn)(void *item, void *argu), void *argu)
{
    base_vector_it  VecIt;

    base_vector_it_set(VecIt, pvec);
    while(VecIt != base_vector_it_end(pvec))
    {
        if (Fn(*VecIt, argu) == NU_FAIL)
		{
			return NU_FAIL;
		}

        ++ VecIt;
    }

	return NU_OK;
}

/* ring (FIFO) */
/* ----------------------------------------------------------- */
void base_ring_node_init(base_ring_node_t *pnode)
{
    pnode->next = pnode;
    return;
}

void base_ring_init(base_ring_t *pring)
{
    pring->cnt = 0;
    pring->head = pring->tail = NULL;

    return;
}

int base_ring_insert_head(base_ring_t *pring, base_ring_node_t *pnode)
{
    if(!(pring->head))
    {
        pring->head = pring->tail = pnode;
        pnode->next = pnode;
    }
    else
    {
        pnode->next = pring->head;
        pring->tail->next = pnode;
        pring->head = pnode;
    }

    return ++ (pring->cnt);
}

int base_ring_insert_tail(base_ring_t *pring, base_ring_node_t *pnode)
{
    if(!(pring->tail))
    {
        pring->head = pring->tail = pnode;
        pnode->next = pnode;
    }
    else
    {
        pnode->next = pring->head;
        pring->tail->next = pnode;
        pring->tail = pnode;
    }

    return ++ (pring->cnt);
}

int base_ring_remove_head(base_ring_t *pring, base_ring_node_t **pnode)
{
    if(!(pring->head))
    {
        return NU_EMPTY;
    }

    *pnode = pring->head;
    if(pring->head == pring->tail)
    {
        pring->head = pring->tail = NULL;
    }
    else
    {
        pring->head = pring->tail->next = pring->head->next;
    }

    return -- (pring->cnt);
}

