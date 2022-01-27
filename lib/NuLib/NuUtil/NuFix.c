
#include "NuCStr.h"
#include "NuHash.h"
#include "NuFix.h"

/* NuFixParser */
struct _NuFixParser_t
{
    NuHash_t    *Hash;
};

struct _NuFixParserNode_t
{
    int             Tag;
    NuFixParserFn   Fn;
    void            *Argu;
};

#define NuFixMsgDelimiter   '\001'
#define NuFixMsgTerminator  '\0'
#define NuFixMsgTagValSep   '='

static unsigned int NuFixParserHash(const void *Key, size_t KeyLen)
{
    return *((unsigned int *)Key);
}

static int NuFixParserCompare(const void *LHS, size_t LHSLen, const void *RHS, size_t RHSLen)
{
    return *(int *)(LHS) - *(int *)(RHS);
}

int NuFixParserNew(NuFixParser_t **pParser)
{
    int RC = NU_OK;

    *pParser = (NuFixParser_t *)malloc(sizeof(NuFixParser_t));
    if(!(*pParser))
    {
        NUGOTO(RC, NU_MALLOC_FAIL, EXIT);
    }

    if(NuHashNew2(&((*pParser)->Hash), 100, &NuFixParserHash, &NuFixParserCompare) != NU_OK)
    {
        NUGOTO(RC, NU_MALLOC_FAIL, EXIT);
    }

EXIT:
    if(RC < 0)
    {
        NuFixParserFree(*pParser);
    }

    return RC;
}

static void _NuFixParserFreeNode(NuHashItem_t *Item, void *Argu)
{
	NuHash_t *Hash = (NuHash_t *)Argu;
    void *Value = NuHashItemGetValue(Item);
	const void *Key = NuHashItemGetKey(Item);
	unsigned int Idx = 0;

	if (NuHashSearch(Hash, Key, strlen((char *)Key), &Idx) != NULL)
	{
		NuHashRemove(Hash, Item, Idx);
		if (Value != NULL)
		{
			free(Value);
		}
	}
	
}

void NuFixParserFree(NuFixParser_t *Parser)
{
    if(Parser != NULL)
    {
        if(Parser->Hash != NULL)
        {
            NuHashForEach(Parser->Hash, NULL, 0, &_NuFixParserFreeNode, Parser->Hash);
            NuHashFree(Parser->Hash);
        }

        free(Parser);
    }

    return;
}

static bool NuFixParserDefaultFn(int Tag, const char *Value, size_t ValueLen, void *Argu, void *Closure)
{
    return true;
}

NuFixParserNode_t *NuFixParserGetNode(NuFixParser_t *Parser, int Tag)
{
    NuHashItem_t        *Item = NULL;
    NuFixParserNode_t   *Node = NULL;
    unsigned int        Idx = 0;

    if(!(Item = NuHashSearch(Parser->Hash, &Tag, sizeof(int), &Idx)))
    {
        Node = (NuFixParserNode_t *)malloc(sizeof(NuFixParserNode_t));

        Node->Tag = Tag;
        NuFixParserRemoveCallback(Node);

        NuHashAdd(Parser->Hash, &(Node->Tag), sizeof(int), Node, Idx);
    }
    else
    {
        Node = NuHashItemGetValue(Item);
    }

    return Node;
}

void NuFixParserSetCallback(NuFixParserNode_t *Node, NuFixParserFn Fn, void *Argu)
{
    //Node->Fn = &NuFixParserDefaultFn;
    Node->Argu = Argu;
    Node->Fn = Fn;

    return;
}

void NuFixParserRemoveCallback(NuFixParserNode_t *Node)
{
    Node->Fn = &NuFixParserDefaultFn;
    Node->Argu = NULL;

    return;
}

static bool NuFixParserToIntFn(int Tag, const char *Value, size_t ValueLen, void *Argu, void *Closure)
{
    *((int *)Argu) = NuCStrToInt(Value, ValueLen);
    return true;
}

void NuFixParserSetToInt(NuFixParserNode_t *Node, int *Storage)
{
    NuFixParserSetCallback(Node, &NuFixParserToIntFn, Storage);
    return;
}

static bool NuFixParserToLongFn(int Tag, const char *Value, size_t ValueLen, void *Argu, void *Closure)
{
    *((long *)Argu) = NuCStrToLong(Value, ValueLen);
    return true;
}

void NuFixParserSetToLong(NuFixParserNode_t *Node, long *Storage)
{
    NuFixParserSetCallback(Node, &NuFixParserToLongFn, Storage);
    return;
}

static bool NuFixParserToDoubleFn(int Tag, const char *Value, size_t ValueLen, void *Argu, void *Closure)
{
    *((double *)Argu) = NuCStrToDouble(Value, ValueLen);
    return true;
}

void NuFixParserSetToDouble(NuFixParserNode_t *Node, double *Storage)
{
    NuFixParserSetCallback(Node, &NuFixParserToDoubleFn, Storage);
    return;
}

static bool NuFixParserToFloatFn(int Tag, const char *Value, size_t ValueLen, void *Argu, void *Closure)
{
    *((float *)Argu) = NuCStrToFloat(Value, ValueLen);
    return true;
}

void NuFixParserSetToFloat(NuFixParserNode_t *Node, float *Storage)
{
    NuFixParserSetCallback(Node, &NuFixParserToFloatFn, Storage);
    return;
}

static bool NuFixParserToStrFn(int Tag, const char *Value, size_t ValueLen, void *Argu, void *Closure)
{
    NuStrNCpy(Argu, Value, ValueLen);
    return true;
}

void NuFixParserSetToStr(NuFixParserNode_t *Node, NuStr_t *Storage)
{
    NuFixParserSetCallback(Node, &NuFixParserToStrFn, Storage);
    return;
}

bool NuFixParserForEach(NuFixParser_t *Parser, const char *Msg, void *Closure)
{
    int                 Tag = 0;
    const char          *pValue = NULL;
    NuHashItem_t        *Item = NULL;
    NuFixParserNode_t   *Node = NULL;
    unsigned int        Idx = 0;
    bool                Ret = false;

    if(*Msg == NuFixMsgDelimiter)
    {
        ++ Msg;
    }

    while(*Msg != '\0')
    {
        while(*Msg != NuFixMsgTagValSep)
        {
            Tag = 10 * Tag + *Msg - '0';
            ++ Msg;
        }

        pValue = ++ Msg;
        Msg = strchr(Msg, NuFixMsgDelimiter);

        if((Item = NuHashSearch(Parser->Hash, &Tag, sizeof(int), &Idx)) != NULL)
        {
            Node = NuHashItemGetValue(Item);

            if(!(Ret = Node->Fn(Tag, pValue, Msg - pValue, Node->Argu, Closure)))
            {
                break;
            }
        }

        Tag = 0;
        ++ Msg;
    }

    return Ret;
}

bool NuFixParserForEachByLen(NuFixParser_t *Parser, const char *Msg, int Len, void *Closure)
{
    int                 Tag = 0;
    const char          *pValue = NULL;
    NuHashItem_t        *Item = NULL;
    NuFixParserNode_t   *Node = NULL;
    unsigned int        Idx = 0;
    bool                Ret = false;
	const char          *End = Msg + Len;

    if(*Msg == NuFixMsgDelimiter)
    {
        ++ Msg;
    }

    while(Msg < End)
    {
        while(*Msg != NuFixMsgTagValSep)
        {
            Tag = 10 * Tag + *Msg - '0';
            ++ Msg;
        }

        pValue = ++ Msg;
        Msg = strchr(Msg, NuFixMsgDelimiter);

        if((Item = NuHashSearch(Parser->Hash, &Tag, sizeof(int), &Idx)) != NULL)
        {
            Node = NuHashItemGetValue(Item);

            if(!(Ret = Node->Fn(Tag, pValue, Msg - pValue, Node->Argu, Closure)))
            {
                break;
            }
        }

        Tag = 0;
        ++ Msg;
    }

    return Ret;
}

bool NuFixParserForEachByCB(NuFixParser_t *Parser, const char *Msg, int Len, NuFixParserFn DefaultFn, void *Closure)
{
    int                 Tag = 0;
    const char          *pValue = NULL;
    NuHashItem_t        *Item = NULL;
    NuFixParserNode_t   *Node = NULL;
    unsigned int        Idx = 0;
    bool                Ret = false;
	const char          *End = Msg + Len;

    if(*Msg == NuFixMsgDelimiter)
    {
        ++ Msg;
    }

    while(Msg < End)
    {
        while(*Msg != NuFixMsgTagValSep)
        {
            Tag = 10 * Tag + *Msg - '0';
            ++ Msg;
        }

        pValue = ++ Msg;
        Msg = strchr(Msg, NuFixMsgDelimiter);

        if((Item = NuHashSearch(Parser->Hash, &Tag, sizeof(int), &Idx)) != NULL)
        {
            Node = NuHashItemGetValue(Item);

            if(!(Ret = Node->Fn(Tag, pValue, Msg - pValue, Node->Argu, Closure)))
            {
                break;
            }
        }
		else
		{
            if(!(Ret = DefaultFn(Tag, pValue, Msg - pValue, NULL, Closure)))
            {
                break;
            }
		}

        Tag = 0;
        ++ Msg;
    }

    return Ret;
}

bool NuFixParserFetchByCB(NuFixParser_t *Parser, const char *Msg, int Len, NuFixParserFn Fn, void *Closure)
{
    int                 Tag = 0;
    const char          *pValue = NULL;
    bool                Ret = false;
	const char          *End = Msg + Len;

    if(*Msg == NuFixMsgDelimiter)
    {
        ++ Msg;
    }

    while(Msg < End)
    {
        while(*Msg != NuFixMsgTagValSep)
        {
            Tag = 10 * Tag + *Msg - '0';
            ++ Msg;
        }

        pValue = ++ Msg;
        Msg = strchr(Msg, NuFixMsgDelimiter);

		if(!(Ret = Fn(Tag, pValue, Msg - pValue, NULL, Closure)))
		{
			break;
		}


        Tag = 0;
        ++ Msg;
    }

    return Ret;
}

/* NuFixComposer */
typedef enum _NuFixComposerKind
{
    NuFixComposerKind_None = 0,
    NuFixComposerKind_Field,
    NuFixComposerKind_RepeatingGroup,
} NuFixComposerKind;

typedef struct _NuFixComposerHdr_t
{
    NuStr_t             *Tag;
    NuFixComposerKind   Kind;
    bool                Use;
} NuFixComposerHdr_t;

struct _NuFixComposerField_t
{
    NuFixComposerHdr_t  Hdr;
    const char          *Value;
    size_t              ValueLen;
};

struct _NuFixComposerRepeatingGroup_t
{
    NuFixComposerHdr_t  Hdr;
    int                 No;
    base_vector_t       Member;
};

struct _NuFixComposer_t
{
    NuStr_t             *Msg;
    base_vector_t       Fields;
};

int NuFixComposerNew(NuFixComposer_t **Composer)
{
    int RC = NU_OK;

    if(!(*Composer = (NuFixComposer_t *)malloc(sizeof(NuFixComposer_t))))
    {
        NUGOTO(RC, NU_MALLOC_FAIL, EXIT);
    }

    if(NuStrNewPreAlloc(&((*Composer)->Msg), 1024) != NU_OK)
    {
        NUGOTO(RC, NU_FAIL, EXIT);
    }

    if(base_vector_init(&((*Composer)->Fields), 100) != NU_OK)
    {
        NUGOTO(RC, NU_FAIL, EXIT);
    }

EXIT:
    if(RC < 0)
    {
        NuFixComposerFree(*Composer);
    }

    return RC;
}

static void NuFixComposerFreeFields(base_vector_t *Vec)
{
    NuFixComposerHdr_t              *Hdr = NULL;
    base_vector_it                  VecIt;

    base_vector_it_set(VecIt, Vec);
    while(VecIt != base_vector_it_end(Vec))
    {
        Hdr = (NuFixComposerHdr_t *)(*VecIt);
        switch(Hdr->Kind)
        {
        case NuFixComposerKind_Field:
            break;
        default:
            NuFixComposerFreeFields(&(((NuFixComposerRepeatingGroup_t *)Hdr)->Member));
            break;
        }

		NuStrFree(Hdr->Tag);
        free(Hdr);
        ++ VecIt;
    }

    base_vector_destroy(Vec);

    return;
}

void NuFixComposerFree(NuFixComposer_t *Composer)
{
    if(Composer != NULL)
    {
        if(Composer->Msg != NULL)
        {
            NuStrFree(Composer->Msg);
        }

        NuFixComposerFreeFields(&(Composer->Fields));
 
        free(Composer);
    }

    return;
}

static void NuFixComposerClearFields(base_vector_t *Vec)
{
    NuFixComposerHdr_t  *Hdr = NULL;
    base_vector_it      VecIt;

    base_vector_it_set(VecIt, Vec);
    while(VecIt != base_vector_it_end(Vec))
    {
        Hdr = (NuFixComposerHdr_t *)(*VecIt);
        switch(Hdr->Kind)
        {
        case NuFixComposerKind_Field:
            break;
        default:
            NuFixComposerRepeatingGroupClear((NuFixComposerRepeatingGroup_t *)Hdr);
            break;
        }

        Hdr->Use = false;
        ++ VecIt;
    }

    return;
}

void NuFixComposerClear(NuFixComposer_t *Composer)
{
    NuFixComposerClearFields(&(Composer->Fields));
    return;
}

static void NuFixComposerHdrInit(base_vector_t *Belong, NuFixComposerHdr_t *Hdr, const char *Tag, int Kind)
{
    NuStrNew(&(Hdr->Tag), Tag);
    Hdr->Kind = Kind;
    Hdr->Use = false;

    base_vector_push(Belong, Hdr);

    return;
}

static NuFixComposerField_t *NuFixComposerGenerateField(base_vector_t *Belong, const char *Tag)
{
    NuFixComposerField_t    *Field = (NuFixComposerField_t *)malloc(sizeof(NuFixComposerField_t));

    if(Field != NULL)
    {
        NuFixComposerHdrInit(Belong, &(Field->Hdr), Tag, NuFixComposerKind_Field);
        Field->Value = NULL;
        Field->ValueLen = 0;
    }

    return Field;
}

NuFixComposerField_t *NuFixComposerFindField(NuFixComposer_t *Composer, const char *Tag)
{
	base_vector_t *vec = &(Composer->Fields);
	int i, cnt;
    NuFixComposerField_t *fld = NULL;
	NuFixComposerHdr_t *hdr = NULL;

	cnt = base_vector_get_cnt(vec);

	for (i = 0; i < cnt; i++)
	{
		fld = (NuFixComposerField_t *)base_vector_get_by_index(vec, i);
		hdr = (NuFixComposerHdr_t *)&(fld->Hdr);

		if (hdr->Kind == NuFixComposerKind_Field)
		{
			if (!strcmp(NuStrGet(hdr->Tag), Tag))
			{
				return fld;
			}
		}
	}
	return NULL;

}

NuFixComposerField_t *NuFixComposerGetField(NuFixComposer_t *Composer, const char *Tag)
{
    return NuFixComposerGenerateField(&(Composer->Fields), Tag);
}

void NuFixComposerFieldSet(NuFixComposerField_t *Field, const char *Value, size_t ValueLen)
{
    Field->Value = Value;
    Field->ValueLen = ValueLen;
    Field->Hdr.Use = true;

    return;
}

void NuFixComposerFieldSetUse(NuFixComposerField_t *Field, bool Use)
{
    Field->Hdr.Use = Use;
    return;
}

bool NuFixComposerFieldIsUse(NuFixComposerField_t *Field)
{
    return Field->Hdr.Use;
}

static NuFixComposerRepeatingGroup_t *NuFixComposerGenerateRepeatingGroup(base_vector_t *Belong, const char *Tag)
{
    NuFixComposerRepeatingGroup_t   *Group = (NuFixComposerRepeatingGroup_t *)malloc(sizeof(NuFixComposerRepeatingGroup_t));

    if(Group != NULL)
    {
        NuFixComposerHdrInit(Belong, &(Group->Hdr), Tag, NuFixComposerKind_RepeatingGroup);
        Group->No = 0;
        base_vector_init(&(Group->Member), 20);
    }

    return Group;
}

NuFixComposerRepeatingGroup_t *NuFixComposerGetRepeatingGroup(NuFixComposer_t *Composer, const char *Tag)
{
    return NuFixComposerGenerateRepeatingGroup(&(Composer->Fields), Tag);
}

NuFixComposerField_t *NuFixComposerRepeatingGroupAddField(NuFixComposerRepeatingGroup_t *Group, const char *Tag)
{
    return NuFixComposerGenerateField(&(Group->Member), Tag);
}

NuFixComposerRepeatingGroup_t *NuFixComposerRepeatingGroupAddRepeatingGroup(NuFixComposerRepeatingGroup_t *Group, const char *Tag)
{
    return NuFixComposerGenerateRepeatingGroup(&(Group->Member), Tag);
}

void NuFixComposerRepeatingGroupSetNo(NuFixComposerRepeatingGroup_t *Group, int No)
{
    Group->No = No;
    Group->Hdr.Use = true;
    return;
}

void NuFixComposerRepeatingGroupClear(NuFixComposerRepeatingGroup_t *Group)
{
    Group->No = 0;
    NuFixComposerClearFields(&(Group->Member));

    return;
}

static void NuFixComposerPrint(NuStr_t *Str, base_vector_t *Vec)
{
    NuFixComposerField_t            *Field = NULL;
    NuFixComposerRepeatingGroup_t   *Group = NULL;
    NuFixComposerHdr_t              *Hdr = NULL;
    base_vector_it                  VecIt;

    base_vector_it_set(VecIt, Vec);
    while(VecIt != base_vector_it_end(Vec))
    {
        Hdr = (NuFixComposerHdr_t *)(*VecIt);
        if(Hdr->Use)
        {
            NuStrCat(Str, NuStrGet(Hdr->Tag));
            NuStrCatChr(Str, NuFixMsgTagValSep);
            switch(Hdr->Kind)
            {
            case NuFixComposerKind_Field:
                Field = (NuFixComposerField_t *)Hdr;
                NuStrNCat(Str, Field->Value, Field->ValueLen);
                NuStrCatChr(Str, NuFixMsgDelimiter);
                break;
            default:
                Group = (NuFixComposerRepeatingGroup_t *)Hdr;
                NuStrAppendPrintf(Str, "%d%c", Group->No, NuFixMsgDelimiter);
                NuFixComposerPrint(Str, &(Group->Member));
                break;
            }
        }

        ++ VecIt;
    }

    return;
}

const char *NuFixComposerCompose(NuFixComposer_t *Composer)
{
    NuStrClear(Composer->Msg);
    NuFixComposerPrint(Composer->Msg, &(Composer->Fields));

    return NuStrGet(Composer->Msg);
}

const char *NuFixComposerGetStr(NuFixComposer_t *Composer)
{
    return NuStrGet(Composer->Msg);
}

size_t NuFixComposerGetStrSize(NuFixComposer_t *Composer)
{
    return NuStrSize(Composer->Msg);
}

