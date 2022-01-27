
#include "NuTrie.h"
/* ****************************************************************************** */
/* internal declaration                                                           */
/* ****************************************************************************** */
#define TRIE_ITEM_ROOT    257
#define TRIE_ITEM_CNT     256
struct _NuTrieItem_t
{
	NuTrie_t     *TrieObj;
	NuTrieItem_t *Parent;
	
	unsigned int Idx;
	unsigned int Cnt;
	void         *Ptr;
	NuTrieItem_t *Items[TRIE_ITEM_CNT];
};

#define NuTrieItemSz sizeof(NuTrieItem_t)

struct _NuTrie_t
{
	NuTrieItem_t  *Root;
	NuBuffer_t    *Buffer;      /* buffer for item */
};
#define NuTrieSz sizeof(NuTrie_t)

/* ****************************************************************************** */
/* internal function                                                              */
/* ****************************************************************************** */
static void _TrieItem_Cnt_Add(NuTrieItem_t *item)
{
	NuTrieItem_t *node = item;
	do {
		node->Cnt++;
		node = node->Parent;
	} while(node != NULL);
}

static void _TrieItem_Cnt_Sub(NuTrieItem_t *item)
{
	NuTrieItem_t *node = item;
	
	do {
		node->Cnt--;
		node = node->Parent;
	} while(node != NULL);
}

static NuTrieItem_t *_Trie_Find_End(NuTrie_t *trie, const char *key, unsigned int key_len, bool is_create)
{
	NuTrieItem_t *item = trie->Root;
	NuTrieItem_t *node = NULL;
	int i = 0;
	int idx = 0;

	for (i = 0; i < key_len; i++)
	{
		if (item == NULL)
		{
			NUGOTO(item, NULL, EXIT);
		}
		
		idx = (int)key[i];
		if (item->Items[idx] == NULL && 
		    is_create == true)
		{
			/* add an empty node for use */
			node = (NuTrieItem_t *)NuBufferGet(trie->Buffer);

			memset(node, 0x00, NuTrieItemSz);
			node->Parent = item;
			node->Cnt = 0;
			node->Idx = idx;
			node->TrieObj = trie;

			item->Items[idx] = node;

			item = node;
		}
		else
		{
			item = item->Items[idx];
		}
	}

EXIT:
	return item;
}

/* ****************************************************************************** */
/* public function                                                                */
/* ****************************************************************************** */
int NuTrieNew(NuTrie_t **trie, int buffer_cnt)
{
	int iRC = NU_OK;
	(*trie) = (NuTrie_t *)malloc(NuTrieSz);

	if ((*trie) == NULL)
	{
		NUGOTO(iRC, NU_MALLOC_FAIL, EXIT);
	}
	memset((*trie), 0x00, NuTrieSz);

	iRC = NuBufferNew(&((*trie)->Buffer), NuTrieItemSz, buffer_cnt);
	if (iRC == NU_OK)
	{
		(*trie)->Root = (NuTrieItem_t *)NuBufferGet((*trie)->Buffer);

		memset((*trie)->Root, 0x00, NuTrieItemSz);
		(*trie)->Root->Parent = NULL;
		(*trie)->Root->Cnt = 0;
		(*trie)->Root->Idx = TRIE_ITEM_ROOT;
		(*trie)->Root->TrieObj = NULL;
	}

EXIT:
	if (iRC < 0)
	{
		NuTrieFree((*trie));
	}

	return iRC;
}

void NuTrieFree(NuTrie_t *trie)
{
	if (trie != NULL)
	{
		if (trie->Buffer != NULL)
		{
			NuBufferFree(trie->Buffer);
		}
		
		free(trie);
	}
}

NuTrieItem_t *NuTrieItem_Get_Root(NuTrie_t *trie)
{
	return trie->Root;
}

static void _TrieClearFn(NuTrieItem_t *item, void *argu)
{
	NuTrieItem_Remove(item);
}

void NuTrieItem_Clear(NuTrie_t *trie)
{

	NuTrieItem_ForEach(trie->Root, &_TrieClearFn, NULL);
	return;
}

NuTrieItem_t *NuTrieItem_Find(NuTrie_t *trie, const char *key, unsigned int key_len)
{

	NuTrieItem_t *item = _Trie_Find_End(trie, key, key_len, false);

	if (item == NULL)
	{
		return NULL;
	}
	else
	{
		if (item->Ptr == NULL)
		{
			return NULL;
		}
		else
		{
			return item;
		}
	}
	
}

NuTrieItem_t *NuTrieItem_Add(NuTrie_t *trie, const char *key, unsigned int key_len, void *value)
{
	NuTrieItem_t *item = NULL;
	item = _Trie_Find_End(trie, key, key_len, true);

	if (item->Ptr != NULL)
	{
		NUGOTO(item, NULL, EXIT);
	}

	_TrieItem_Cnt_Add(item);
	item->Ptr = value;

EXIT:
	return item;
}

void NuTrieItem_Replace(NuTrieItem_t *item, void *value)
{
	item->Ptr = value;
}

void NuTrieItem_Remove(NuTrieItem_t *item)
{
	NuTrieItem_t *node = NULL;
	NuTrie_t     *trie = item->TrieObj;

	if (item->Idx == TRIE_ITEM_ROOT)
	{
		return;
	}

	if (item->Ptr != NULL)
	{
		item->Ptr = NULL;
		_TrieItem_Cnt_Sub(item);
	}

	if (item->Cnt == 0)
	{
		node = item->Parent;
		node->Items[item->Idx] = NULL;
			

		NuBufferPut(trie->Buffer, (void *)item);
		if (node->Ptr == NULL && node->Cnt == 0)
		{
			NuTrieItem_Remove(node);
		}

		/* 
		item = NULL;
		*/
	}
}

void *NuTrieItem_Get_Value(NuTrieItem_t *item)
{
	return item->Ptr;
}

int  NuTrieItem_Get_Cnt(NuTrieItem_t *item)
{
	return item->Cnt;
}

void NuTrieItem_ForEach(NuTrieItem_t *item, void (*ForEachFn)(NuTrieItem_t *item, void *argu), void *argu)
{
	int i = 0;
	NuTrieItem_t *node = NULL;

	if (item->Idx != TRIE_ITEM_ROOT)
	{
		if (item->Ptr != NULL)
		{
			ForEachFn(item, argu);
		}
	}

	for (i = 0; i < TRIE_ITEM_CNT; i++)
	{
		node = item->Items[i];
		if (node != NULL)
		{
			NuTrieItem_ForEach(node, ForEachFn, argu);
		}
	}
}



