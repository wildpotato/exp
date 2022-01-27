
#include "NuHash.h"

struct _NuHashItem_t
{
    base_list_node_t    Node;
    void                *Value;
    void                *Hook;
};

typedef struct _NuHashKey_t
{
    const void          *Content;
    size_t              Len;
} NuHashKey_t;

typedef struct _NuHashGroup_t
{
    base_list_t             List;
    NuHashKey_t             Key;
    struct _NuHashGroup_t   *Prev;
    struct _NuHashGroup_t   *Next;
} NuHashGroup_t;

struct _NuHash_t
{
    unsigned int        num;
    unsigned int        anum;

    NuHashFn            HashFn;
    NuHashCompareFn     CompareFn;
    NuBuffer_t          *ItemBuffer;
    NuBuffer_t          *GroupBuffer;
    NuHashGroup_t       *Items[1];
};

/* Internal function */
/* ============================================================================== */

static unsigned int _NuHashGetPrimeNum(int Num)
{
    int Cnt = 0;
    int PrimeNum[] = { 17, 37, 79, 163, 331, 673, 1361, 2729, 5471, 10949, 21911, 43853, 87719, 175447, 350899, 701819, 1403641, 2807303, 5614657, 11229331, 22458671, 44917381, 89834777, 179669557, 359339171, 718678369, 1437356741, 2147483647 };

    while(Cnt < 28)
    {
        if(Num <= PrimeNum[Cnt])
            return PrimeNum[Cnt];

        Cnt ++;
    }

    return PrimeNum[27];
}

static unsigned int _hash_fn(const void *Key, size_t KeyLen)
{
    const char *p = Key;
    const char *e = p + KeyLen;
    unsigned int hash = 5381;

    while(p != e)
    {
        hash = ((hash << 5) + hash) + (*p++);
    }

    return hash;
}

static int _NuHashCompareFn(const void *Key1, size_t KeyLen1, const void *Key2, size_t KeyLen2)
{
    return (KeyLen1 - KeyLen2) ? (KeyLen1 - KeyLen2) : memcmp(Key1, Key2, KeyLen1);
}

static NuHashItem_t *_GetItemFromBuffer(NuHash_t *Hash, void *Value)
{
    NuHashItem_t    *Item = (NuHashItem_t *)NuBufferGet(Hash->ItemBuffer); 

    base_list_node_init(&(Item->Node));

    NuHashItemSetValue(Item, Value);

    Item->Hook = NULL;

    return Item;
}

static NuHashGroup_t *_GetGroupFromBuffer(NuHash_t *Hash, const void *Key, size_t KeyLen)
{
    NuHashGroup_t   *Group = (NuHashGroup_t *)NuBufferGet(Hash->GroupBuffer);

    Group->Next = Group->Prev = NULL;

    base_list_init(&(Group->List));
    Group->Key.Content = Key;
    Group->Key.Len = KeyLen;

    return Group;
}

static int _HashNew(NuHash_t **pHash, int BucketSz, int BufferSz, NuHashFn HashFn, NuHashCompareFn CompareFn) 
{
    int             RC = NU_OK;
    unsigned int    bucket_sz = _NuHashGetPrimeNum(BucketSz);
	unsigned int    buffer_sz = (BufferSz == 0) ? 128 : BufferSz;

    if(!((*pHash) = (NuHash_t *)calloc(sizeof(char), sizeof(NuHash_t) + sizeof(NuHashGroup_t *) * bucket_sz)))
    {
        NUGOTO(RC, NU_MALLOC_FAIL, EXIT);
    }

    if(NuBufferNew(&((*pHash)->ItemBuffer), sizeof(NuHashItem_t), buffer_sz) < 0)
    {
        NUGOTO(RC, NU_MALLOC_FAIL, EXIT);
    }

    if(NuBufferNew(&((*pHash)->GroupBuffer), sizeof(NuHashGroup_t), buffer_sz) < 0)
    {
        NUGOTO(RC, NU_MALLOC_FAIL, EXIT);
    }

    (*pHash)->num = 0;
    (*pHash)->anum = bucket_sz;
    (*pHash)->HashFn = (!HashFn) ? &_hash_fn : HashFn;
    (*pHash)->CompareFn = (!CompareFn) ? &_NuHashCompareFn : CompareFn;

EXIT:
    if(RC < 0)
    {
        NuHashFree(*pHash);
    }

    return RC;
}
/* ----- Constructor/Distructor ----- */
/* ============================================================================== */
int NuHashNew(NuHash_t **pHash, int BucketSz)
{
	return _HashNew(pHash, BucketSz, 0, NULL, NULL);
}

int NuHashNew2(NuHash_t **pHash, int BucketSz, NuHashFn HashFn, NuHashCompareFn CompareFn) 
{
	return _HashNew(pHash, BucketSz, 0, HashFn, CompareFn);
}

int NuHashNewPreAlloc(NuHash_t **pHash, int BucketSz, int BufferSz)
{
	return _HashNew(pHash, BucketSz, BufferSz, NULL, NULL);
}

int NuHashNewPreAlloc2(NuHash_t **pHash, int BucketSz, int BufferSz, NuHashFn HashFn, NuHashCompareFn CompareFn)
{
	return _HashNew(pHash, BucketSz, BufferSz, HashFn, CompareFn);
}

void NuHashFree(NuHash_t *Hash)
{
    if(Hash != NULL)
    {
        if(Hash->ItemBuffer != NULL)
        {
            NuBufferFree(Hash->ItemBuffer);
        }

        if(Hash->GroupBuffer != NULL)
        {
            NuBufferFree(Hash->GroupBuffer);
        }

        free(Hash);
    }

    return;
}

void NuHashClear(NuHash_t *Hash)
{
    Hash->num = 0;
    memset(Hash->Items, 0, (Hash->anum) * sizeof(NuHashGroup_t *));
    NuBufferClear(Hash->GroupBuffer);
    NuBufferClear(Hash->ItemBuffer);

    return;
}

NuHashItem_t *NuHashSearch(NuHash_t *Hash, const void *Key, size_t KeyLen, unsigned int *Idx)
{
    unsigned int        hash = 0;
    NuHashGroup_t       *Group = NULL;
    base_list_node_t    *Node = NULL;

    hash = Hash->HashFn(Key, KeyLen);
    *Idx = hash % Hash->anum;

    if((Group = Hash->Items[*Idx]) != NULL)
    {
        do
        {
            if(Hash->CompareFn(Group->Key.Content, Group->Key.Len, Key, KeyLen) == 0)
            {
                if((Node = base_list_get_head(&(Group->List))) != NULL)
                {
                    return (NuHashItem_t *)Node;
                }

                return NULL;
            }
        }
        while((Group = Group->Next) != NULL);
    }

    return NULL;
}

NuHashItem_t *NuHashAdd(NuHash_t *Hash, const void *Key, size_t KeyLen, void *Value, unsigned int Idx)
{
    NuHashGroup_t   *Group = _GetGroupFromBuffer(Hash, Key, KeyLen), *GroupFirst = Hash->Items[Idx];
    NuHashItem_t    *AddItem = _GetItemFromBuffer(Hash, Value);

    base_list_insert_head(&(Group->List), &(AddItem->Node));

    if(GroupFirst != NULL)
    {
        GroupFirst->Prev = Group;
        Group->Next = GroupFirst;
    }

    Hash->Items[Idx] = Group;

    ++ (Hash->num);

    return AddItem;
}

NuHashItem_t *NuMultiHashAdd(NuHash_t *Hash, NuHashItem_t *Item, void *Value)
{
    NuHashItem_t    *AddItem = _GetItemFromBuffer(Hash, Value);

    base_list_insert_head(base_list_get_list(&(Item->Node)), &(AddItem->Node));

    ++ (Hash->num);

    return AddItem;
}

void NuHashRemove(NuHash_t *Hash, NuHashItem_t *Item, unsigned int Idx)
{
    NuHashGroup_t   *Group = (NuHashGroup_t *)(base_list_get_list(&(Item->Node)));

    -- (Hash->num);

    if(!base_list_remove_node(&(Item->Node)))
    {
        if(Group->Prev != NULL)
        {
            Group->Prev->Next = Group->Next;
        }
        else
        {
            Hash->Items[Idx] = Group->Next;
        }

        if(Group->Next != NULL)
        {
            Group->Next->Prev = Group->Prev;
        }

        NuBufferPut(Hash->GroupBuffer, Group);
    }

    NuBufferPut(Hash->ItemBuffer, Item);

    return;
}

NuHashItem_t *NuHashRemove2(NuHash_t *Hash, NuHashItem_t *Item, unsigned int Idx)
{
    NuHashGroup_t   *Group = (NuHashGroup_t *)(base_list_get_list(&(Item->Node)));

    -- (Hash->num);

    if(!base_list_remove_node(&(Item->Node)))
    {
        if(Group->Prev != NULL)
        {
            Group->Prev->Next = Group->Next;
        }
        else
        {
            Hash->Items[Idx] = Group->Next;
        }

        if(Group->Next != NULL)
        {
            Group->Next->Prev = Group->Prev;
        }

        NuBufferPut(Hash->GroupBuffer, Group);
    }

    return Item;
}

void NuHashItemRemoveComplete(NuHash_t *Hash, NuHashItem_t *Item)
{
    NuBufferPut(Hash->ItemBuffer, Item);
}


int NuHashGetNum(NuHash_t *Hash)
{
    return Hash->num;
}

NuHashItem_t *NuHashRight(NuHashItem_t *Item)
{
    base_list_node_t    *Node = base_list_node_next(&(Item->Node));

    return (!Node) ? (NULL) : ((NuHashItem_t *)Node);
}

NuHashItem_t *NuHashLeft(NuHashItem_t *Item)
{
    base_list_node_t    *Node = base_list_node_prev(&(Item->Node));

    return (!Node) ? (NULL) : ((NuHashItem_t *)Node);
}

NuHashItem_t *NuHashRightMost(NuHashItem_t *Item)
{
    base_list_node_t    *Node = base_list_get_tail(base_list_get_list(&(Item->Node)));

    return (!Node) ? (NULL) : ((NuHashItem_t *)Node);
}

NuHashItem_t *NuHashLeftMost(NuHashItem_t *Item)
{
    base_list_node_t    *Node = base_list_get_head(base_list_get_list(&(Item->Node)));

    return (!Node) ? (NULL) : ((NuHashItem_t *)Node);
}

const void *NuHashItemGetKey(NuHashItem_t *Item)
{
    return (((NuHashGroup_t *)(base_list_get_list(&(Item->Node))))->Key).Content;
}

size_t NuHashItemGetKeyLen(NuHashItem_t *Item)
{
    return (((NuHashGroup_t *)(base_list_get_list(&(Item->Node))))->Key).Len;
}

void *NuHashItemGetValue(NuHashItem_t *Item)
{
    return Item->Value;
}

int NuHashItemGetKeyCollision(NuHashItem_t *Item)
{
    return base_list_items_cnt(base_list_get_list(&(Item->Node)));
}

void NuHashItemSetValue(NuHashItem_t *Item, void *Value)
{
    Item->Value = Value;
    return;
}

void NuHashForEach(NuHash_t *Hash, const void *Key, size_t KeyLen, NuHashForEachFn Fn, void *Argu)
{
    NuHashItem_t    *Item = NULL;
	NuHashItem_t    *Next = NULL;
    unsigned int    Idx = 0;
    NuHashGroup_t   *Group = NULL;

    if(Key)
    {
        Item = NuHashSearch(Hash, Key, KeyLen, &Idx);

        while(Item)
        {
            Next = NuHashRight(Item);
            Fn(Item, Argu);
            Item = Next;
        }
    }
    else
    {
        while(Idx < Hash->anum)
        {
            if((Group = Hash->Items[Idx]) != NULL)
            {
				do
				{
					if(base_list_items_cnt(&(Group->List)) > 0)
					{
						Item = (NuHashItem_t *)(base_list_get_head(&(Group->List)));

						while(Item)
						{
							Next = NuHashRight(Item);
							Fn(Item, Argu);
							Item = Next;
						}
					}
				} while ((Group = Group->Next) != NULL);
            }

            ++ Idx;
        }
    }

    return;
}

