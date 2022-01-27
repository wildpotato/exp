
#include "NuCStr.h"
#include "NuTrie.h"
#include "NuFixReader.h"

/* NuFixReader */
struct _NuFixReader_t
{
    NuTrie_t    *Trie;
};

struct _NuFixReaderNode_t
{
    char            Tag[10];
	int             TagLen;
    NuFixReaderFn   Fn;
    void            *Argu;
};

#define NuFixMsgDelimiter   '\001'
#define NuFixMsgTerminator  '\0'
#define NuFixMsgTagValSep   '='

/* ---------------------------------------------------------- */
/* internal function                                          */
/* ---------------------------------------------------------- */
static void _ClearReaderNode(NuTrieItem_t *item, void *argu)
{
	NuFixReaderNode_t *node = (NuFixReaderNode_t *)NuTrieItem_Get_Value(item);
	NuTrieItem_Remove(item);
	free(node);
}

static bool _NuFixReaderDefaultFn(const char *Tag, size_t TagLen, const char *Value, size_t ValueLen, void *Argu, void *Closure)
{
    return true;
}

/* ---------------------------------------------------------- */
/* public function                                            */
/* ---------------------------------------------------------- */
int NuFixReaderNew(NuFixReader_t **pReader)
{
    int iRC = NU_OK;

    *pReader = (NuFixReader_t *)malloc(sizeof(NuFixReader_t));
    if(!(*pReader))
    {
        NUGOTO(iRC, NU_MALLOC_FAIL, EXIT);
    }

	iRC = NuTrieNew(&((*pReader)->Trie), 128);
    if(iRC < 0)
    {
        NUGOTO(iRC, NU_MALLOC_FAIL, EXIT);
    }

EXIT:
    if(iRC < 0)
    {
        NuFixReaderFree((*pReader));
    }

    return iRC;
}

void NuFixReaderFree(NuFixReader_t *pReader)
{
    if(pReader != NULL)
    {
        if(pReader->Trie != NULL)
        {
			NuTrieItem_t *item;
			item = NuTrieItem_Get_Root(pReader->Trie);
			NuTrieItem_ForEach(item, &_ClearReaderNode, NULL);
			NuTrieFree(pReader->Trie);
        }
        free(pReader);
    }

    return;
}

NuFixReaderNode_t *NuFixReaderGetNode(NuFixReader_t *pReader, char *key, unsigned int key_len)
{
    NuTrieItem_t        *item = NULL;
    NuFixReaderNode_t   *node = NULL;

	item = NuTrieItem_Find(pReader->Trie, key, key_len);
	if (item == NULL)
	{
		node = (NuFixReaderNode_t *)malloc(sizeof(NuFixReaderNode_t));
		strncpy(node->Tag, key, key_len);
		node->TagLen = key_len;

		NuTrieItem_Add(pReader->Trie, key, key_len, node);
	}
	else
	{
		node = (NuFixReaderNode_t *)NuTrieItem_Get_Value(item);
	}

	return node;
}

void NuFixReaderSetCallback(NuFixReaderNode_t *Node, NuFixReaderFn Fn, void *Argu)
{
    Node->Fn = (Fn == NULL) ?  &_NuFixReaderDefaultFn : Fn;
    Node->Argu = Argu;

    return;
}

void NuFixReaderRemoveCallback(NuFixReaderNode_t *Node)
{
    Node->Fn = &_NuFixReaderDefaultFn;
    Node->Argu = NULL;

    return;
}

bool NuFixReaderForEachWithCB(NuFixReader_t *pReader, const char *Msg, int Len, NuFixReaderFn DefaultFn, void *Closure)
{
	const char          *pTag  = NULL;
    const char          *pVal  = NULL;
    NuFixReaderNode_t   *node  = NULL;
    bool                Ret    = false;
	const char          *End   = Msg + Len;
	NuTrieItem_t        *item  = NULL;

    if(*Msg == NuFixMsgDelimiter)
    {
        ++ Msg;
    }

    while(Msg < End)
    {
		unsigned int TagLen = 0;
		pTag = Msg;

        while(*Msg != NuFixMsgTagValSep)
        {
            ++ Msg;
        }

		TagLen = Msg - pTag;

        pVal = ++ Msg;
        Msg = strchr(Msg, NuFixMsgDelimiter);

		item = NuTrieItem_Find(pReader->Trie, pTag, TagLen);
		if( item != NULL)
        {
            node = NuTrieItem_Get_Value(item);
			if (node == NULL)
			{
				break;
			}

            Ret = node->Fn(pTag, TagLen,  pVal, Msg - pVal, node->Argu, Closure);
            if(Ret == false)
            {
                break;
            }
        }
		else
		{
            Ret = DefaultFn(pTag, TagLen,  pVal, Msg - pVal, NULL, Closure);
            if(Ret == false)
            {
                break;
            }
		}

        ++ Msg;
    }

    return Ret;

}

bool NuFixReaderForEach(NuFixReader_t *pReader, const char *Msg, int Len, void *Closure)
{
	return NuFixReaderForEachWithCB(pReader, Msg, Len, &_NuFixReaderDefaultFn, Closure);
}


