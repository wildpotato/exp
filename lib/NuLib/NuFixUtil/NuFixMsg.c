

#include "NuFixMsg.h"

struct _NuFixMsg_t
{
    NuHash_t        *Hash;
    base_vector_t   *Header;
    base_vector_t   *Body;
    base_vector_t   *AllTag;
    NuBuffer_t      *InfoBuffer;
    NuStr_t         *InMsg;
    NuStr_t         *OutMsg;
};
size_t NuFixMsgSz = sizeof(struct _NuFixMsg_t);


struct _NuFixTag_t
{
	void *Value;
	size_t Len;
	void *Info;
};

typedef struct _NuFixMsgInfo_t
{
    int                     Tag;
	NuFixTag_t              TagVal;
	NuHashItem_t            *Item;
    char                    TagStr[NuFixMsgDefaultTagStrLen + 2];
    int                     Kind;
    struct _NuFixMsgInfo_t  *pBelong;
    NuStr_t                 *Str;
    int                     CurrentRepeatingInstance;
    base_vector_t           *RepeatingGroupMember;
} NuFixMsgInfo_t;

#define NuFixMsgDelimiterStr        "\001"
#define NuFixMsgTagValSepStr        "="
#define NuFixMsgShellEight          "8"NuFixMsgTagValSepStr
#define NuFixMsgShellNine           NuFixMsgDelimiterStr"9=00000"NuFixMsgDelimiterStr
#define NuFixMsgShellTen            "10"NuFixMsgTagValSepStr

/* ====================================================================== */
/* Internal function                                                      */
/* ---------------------------------------------------------------------- */
static unsigned int fixmsg_hash_fn(const void *key, size_t len)
{
    return *((unsigned int *)key);
}

static int fixmsg_cmp_fn(const void *v1, size_t v1_len, const void *v2, size_t v2_len)
{
	return *(unsigned int *)v1 - *(unsigned int *)v2;
}

static int _TagCompare(const void *v1, const void *v2)
{
    return (*(NuFixMsgInfo_t **)v1)->Tag - (*(NuFixMsgInfo_t **)v2)->Tag;
}

static NuFixMsgInfo_t *GetInfoFromBuffer(NuFixMsg_t *pFixMsg, int Tag, size_t ValLen, int Kind)
{
    NuFixMsgInfo_t      *pInfo = (NuFixMsgInfo_t *)NuBufferGet(pFixMsg->InfoBuffer);

    pInfo->Tag = Tag;
    sprintf(pInfo->TagStr, "%d%c", Tag, NuFixMsgTagValSep);
    pInfo->Kind = Kind;
    NuStrNewPreAlloc(&(pInfo->Str), ValLen);
    pInfo->RepeatingGroupMember = NULL;
    pInfo->CurrentRepeatingInstance = 0;
    pInfo->pBelong = NULL;
	pInfo->TagVal.Info = pInfo;
	pInfo->Item = NULL;

    base_vector_push(pFixMsg->AllTag, pInfo);

    return pInfo;
}

static void _FieldAssign(NuFixTag_t *pTag, char *pVal, size_t Len)
{
    pTag->Value = pVal;
    pTag->Len = Len;

    return;
}

static void _FieldSet(NuFixTag_t *pTag, char *pVal, size_t Len)
{
    NuStr_t *pStr = ((NuFixMsgInfo_t *)(pTag->Info))->Str;

    NuStrNCpy(pStr, pVal, Len);
    pTag->Value = (char *)NuStrGet(pStr);
    pTag->Len = NuStrSize(pStr);

    return;
}

static void _FieldSprintf(NuFixTag_t *pTag, char *Format, va_list ArguList)
{
    NuStr_t *pStr = ((NuFixMsgInfo_t *)(pTag->Info))->Str;

    NuStrVPrintf(pStr, 0, Format, ArguList);
    pTag->Value = (char *)NuStrGet(pStr);
    pTag->Len = NuStrSize(pStr);

    return;
}

static void _FieldRemove(NuFixTag_t *pTag)
{
    pTag->Value = NULL;
    pTag->Len = 0;

    return;
}

static void _GroupAdd(NuFixMsg_t *pFixMsg, NuFixMsgInfo_t *pLeading)
{
    NuHash_t        *pHash = pFixMsg->Hash;
    NuHashItem_t    *pItem = NULL;
    NuFixMsgInfo_t  *pInfo = NULL;
    base_vector_t   *pVec = pLeading->RepeatingGroupMember;
    base_vector_it  VecIt;
    int             Kind = (pLeading->Kind & NuFixMsgKindHeader) ? (NuFixMsgKindHeader|NuFixMsgKindRepeatingGroupMember) 
		                                                         : (NuFixMsgKindBody|NuFixMsgKindRepeatingGroupMember);

    base_vector_it_set(VecIt, pVec);

    while(VecIt != base_vector_it_end(pVec))
    {
        pItem = ((NuFixMsgInfo_t *)(*VecIt))->Item;
        pInfo = GetInfoFromBuffer(pFixMsg, ((NuFixMsgInfo_t *)NuHashItemGetValue(pItem))->Tag, NuFixMsgDefaultValueLen, Kind);
        pInfo->Item = NuMultiHashAdd(pHash, pItem, pInfo);
        pInfo->pBelong = pLeading;
        pInfo->TagVal.Info = pInfo;

        ++ VecIt;
    }

    return;
}

/* ---------------------------------------------------------------------- */
int NuFixMsgInit(NuFixMsg_t *pFixMsg)
{
    int         iRC = NU_OK;

    pFixMsg->Hash = NULL;
    iRC = NuHashNew2(&(pFixMsg->Hash), NuFixMsgDefaultTagNum, &fixmsg_hash_fn, &fixmsg_cmp_fn);
	if (iRC < 0) {
		goto EXIT;
	}

    pFixMsg->Header = NULL;
    iRC = base_vector_new(&(pFixMsg->Header), 30);
    NUCHKRC(iRC, EXIT);

    pFixMsg->Body = NULL;
    iRC = base_vector_new(&(pFixMsg->Body), 70);
    NUCHKRC(iRC, EXIT);

    pFixMsg->AllTag = NULL;
    iRC = base_vector_new(&(pFixMsg->AllTag), NuFixMsgDefaultTagNum);
    NUCHKRC(iRC, EXIT);

    pFixMsg->InfoBuffer = NULL;
    iRC = NuBufferNew(&(pFixMsg->InfoBuffer), sizeof(NuFixMsgInfo_t), NuFixMsgDefaultTagNum);
    NUCHKRC(iRC, EXIT);

    pFixMsg->InMsg = NULL;
    iRC = NuStrNewPreAlloc(&(pFixMsg->InMsg), 512);
    NUCHKRC(iRC, EXIT);

    pFixMsg->OutMsg = NULL;
    iRC = NuStrNewPreAlloc(&(pFixMsg->OutMsg), 512);
    NUCHKRC(iRC, EXIT);

    NuFixMsgTemplateAddTag(pFixMsg, 8, NuFixMsgVerLen, 0, NuFixMsgKindShell);
    NuFixMsgFieldSet(pFixMsg, 8, NuFixMsgDefaultVer, strlen(NuFixMsgDefaultVer));

EXIT:
    if(iRC < 0)
    {
        NuFixMsgDestroy(pFixMsg);
    }

    return iRC;
}

void NuFixMsgDestroy(NuFixMsg_t *pFixMsg)
{
    void            *ptr = NULL;
    NuFixMsgInfo_t  *pInfo = NULL;
    base_vector_t   *pVec = NULL;

    if(pFixMsg != NULL)
    {
        if((pVec = pFixMsg->AllTag) != NULL)
        {
            while(base_vector_pop(pVec, &ptr) >= 0)
            {
                pInfo = (NuFixMsgInfo_t *)(ptr);
                NuStrFree(pInfo->Str);
                if(pInfo->RepeatingGroupMember != NULL)
                {
                    base_vector_free(pInfo->RepeatingGroupMember);
                }
            }

            base_vector_free(pVec);
        }

        base_vector_free(pFixMsg->Header);
        base_vector_free(pFixMsg->Body);

        if(pFixMsg->Hash != NULL)
        {
            NuHashFree(pFixMsg->Hash);
        }

        if(pFixMsg->InMsg != NULL)
        {
            NuStrFree(pFixMsg->InMsg);
        }

        if(pFixMsg->OutMsg != NULL)
        {
            NuStrFree(pFixMsg->OutMsg);
        }

        if(pFixMsg->InfoBuffer != NULL)
        {
            NuBufferFree(pFixMsg->InfoBuffer);
        }
    }

    return;
}

int NuFixMsgNew(NuFixMsg_t **pFixMsg)
{
	int RC = NU_OK;

    (*pFixMsg) = NULL;
    if(!((*pFixMsg) = (NuFixMsg_t *)malloc(sizeof(NuFixMsg_t))))
    {
        return NU_MALLOC_FAIL;
    }

	if((RC = NuFixMsgInit(*pFixMsg)) < 0)
    {
		free(*pFixMsg);
    }

    return RC;
}

void NuFixMsgFree(NuFixMsg_t *pFixMsg)
{
    if(pFixMsg != NULL)
    {
        free(pFixMsg);
    }

    return;
}

void NuFixMsgClear(NuFixMsg_t *pFixMsg)
{
    base_vector_t   *pVec = pFixMsg->AllTag;
    base_vector_it  VecIt;
    NuFixMsgInfo_t  *pInfo = NULL;

    base_vector_it_set(VecIt, pVec);
    while(VecIt != base_vector_it_end(pVec))
    {
        pInfo = (NuFixMsgInfo_t *)(*VecIt);
        _FieldRemove(&(pInfo->TagVal));
        pInfo->CurrentRepeatingInstance = 0;
    
        ++ VecIt;
    }

    NuStrClear(pFixMsg->InMsg);
    NuStrClear(pFixMsg->OutMsg);

    NuFixMsgFieldSet(pFixMsg, 8, NuFixMsgDefaultVer, strlen(NuFixMsgDefaultVer));

    return;
}

int NuFixMsgTemplateAddTag(NuFixMsg_t *pFixMsg, int Tag, size_t ExpectLen, int BelongGroup, int Kind)
{
    unsigned int    idx = 0;
    NuHash_t        *pHash = pFixMsg->Hash;
    NuHashItem_t    *pItem = NuHashSearch(pHash, &Tag, sizeof(int), &idx);
    NuFixMsgInfo_t  *pInfo = NULL;

    if(pItem != NULL)
    {
        return NU_DUPLICATE;
    }

    pInfo = GetInfoFromBuffer(pFixMsg, Tag, ExpectLen, Kind);
	pItem = NuHashAdd(pHash, &(pInfo->Tag), sizeof(int), pInfo, idx);
	pInfo->Item = pItem;

    if(Kind & NuFixMsgKindHeader)
    {
        if(Kind & NuFixMsgKindRepeatingGroupMember)
        {
            if(!(pItem = NuHashSearch(pHash, &BelongGroup, sizeof(int), &idx)))
            {
                NuFixMsgTemplateAddTag(pFixMsg, BelongGroup, NuFixMsgDefaultValueLen, 0, NuFixMsgKindHeader|NuFixMsgKindRepeatingGroupLeader);
                pItem = NuHashSearch(pHash, &BelongGroup, sizeof(int), &idx);
            }

            pInfo->pBelong = (NuFixMsgInfo_t *)(NuHashItemGetValue(pItem));
            base_vector_push(pInfo->pBelong->RepeatingGroupMember, pInfo);
        }
        else
        {
            if(Kind & NuFixMsgKindRepeatingGroupLeader)
            {
                base_vector_new(&(pInfo->RepeatingGroupMember), 10);
            }

            base_vector_push(pFixMsg->Header, pInfo);
        }
    }
    else if(Kind & NuFixMsgKindBody)
    {
        if(Kind & NuFixMsgKindRepeatingGroupMember)
        {
            if(!(pItem = NuHashSearch(pHash, &BelongGroup, sizeof(int), &idx)))
            {
                NuFixMsgTemplateAddTag(pFixMsg, BelongGroup, NuFixMsgDefaultValueLen, 0, NuFixMsgKindBody|NuFixMsgKindRepeatingGroupLeader);
                pItem = NuHashSearch(pHash, &BelongGroup, sizeof(int), &idx);
            }

            pInfo->pBelong = (NuFixMsgInfo_t *)(NuHashItemGetValue(pItem));
            base_vector_push(pInfo->pBelong->RepeatingGroupMember, pInfo);
        }
        else
        {
            if(Kind & NuFixMsgKindRepeatingGroupLeader)
            {
                base_vector_new(&(pInfo->RepeatingGroupMember), 10);
            }

            base_vector_push(pFixMsg->Body, pInfo);
        }
    }
    else
    {
        return NU_FAIL;
    }

    return NU_OK;
}

int NuFixMsgTemplateAddRepeatingGroup(NuFixMsg_t *pFixMsg, int Kind, int LeadingTag, int NodeNo, ...)
{
    va_list         Argu;

    if(NuFixMsgTemplateAddTag(pFixMsg, LeadingTag, NuFixMsgDefaultTagStrLen, 0, Kind|NuFixMsgKindRepeatingGroupLeader) != NU_OK)
    {
        return NU_FAIL;
    }

    va_start(Argu, NodeNo);

    for(; NodeNo > 0; -- NodeNo)
    {
        int NodeTag = va_arg(Argu, int);
        size_t NodeLen = va_arg(Argu, size_t);

        NuFixMsgTemplateAddTag(pFixMsg, NodeTag, NodeLen, LeadingTag, Kind|NuFixMsgKindRepeatingGroupMember);
    }

    return NU_OK;
}

void NuFixMsgTemplateSort(NuFixMsg_t *pFixMsg)
{
    base_vector_sort(pFixMsg->Header, &_TagCompare);
    base_vector_sort(pFixMsg->Body, &_TagCompare);

    return;
}

NuFixTag_t *NuFixMsgFieldGet(NuFixMsg_t *pFixMsg, int Tag)
{
    unsigned int    idx = 0;
	NuHashItem_t    *item = NULL;

	item = NuHashSearch(pFixMsg->Hash, &Tag, sizeof(int), &idx);

	if (item != NULL)
	{
		return (NuFixTag_t *)&(((NuFixMsgInfo_t *)NuHashItemGetValue(item))->TagVal); 
	}
	return NULL;

}

int NuFixMsgFieldAssign(NuFixMsg_t *pFixMsg, int Tag, char *pVal, size_t Len)
{
    NuFixTag_t *pTag = NuFixMsgFieldGet(pFixMsg, Tag);

    if(pTag == NULL)
    {
        return NU_FAIL;
    }

    _FieldAssign(pTag, pVal, Len);

    return NU_OK;
}

int NuFixMsgFieldSet(NuFixMsg_t *pFixMsg, int Tag, char *pVal, size_t Len)
{
    NuFixTag_t *pTag = NuFixMsgFieldGet(pFixMsg, Tag);

    if(pTag == NULL)
    {
        return NU_FAIL;
    }

    _FieldSet(pTag, pVal, Len);

    return NU_OK;
}

int NuFixMsgFieldVPrintf(NuFixMsg_t *pFixMsg, int Tag, char *Format, va_list ArguList)
{
    NuFixTag_t *pTag = NuFixMsgFieldGet(pFixMsg, Tag);

    if(pTag == NULL)
    {
        return NU_FAIL;
    }

    _FieldSprintf(pTag, Format, ArguList);

    return NU_OK;
}

int NuFixMsgFieldPrintf(NuFixMsg_t *pFixMsg, int Tag, char *Format, ...)
{
    int     Result = 0;
    va_list ArguList;

    va_start(ArguList, Format);
    Result = NuFixMsgFieldVPrintf(pFixMsg, Tag, Format, ArguList);
    va_end(ArguList);

    return Result;
}

int NuFixMsgFieldRemove(NuFixMsg_t *pFixMsg, int Tag)
{
    NuFixTag_t *pTag = NuFixMsgFieldGet(pFixMsg, Tag);

    if(pTag == NULL)
    {
        return NU_FAIL;
    }

    _FieldRemove(pTag);

    return NU_OK;
}

static NuFixTag_t *NuFixMsgGroupFind(NuFixMsg_t *pFixMsg, int Tag, int Idx)
{
    unsigned int    HashIdx = 0;
    NuHashItem_t    *pItem = NuHashSearch(pFixMsg->Hash, &Tag, sizeof(int), &HashIdx);
    NuFixMsgInfo_t  *pLeading = NULL;

    if(!pItem)
    {
        return NULL;
    }
 
    pLeading = ((NuFixMsgInfo_t *)(NuHashItemGetValue(pItem)))->pBelong;
    pItem = NuHashRightMost(pItem);

    if(Idx + 1 > (pLeading->CurrentRepeatingInstance))
    {
        (pLeading->CurrentRepeatingInstance) = Idx + 1;
    }

    for(; Idx > 0; -- Idx)
    {
        if((pItem = NuHashLeft(pItem)) == NULL)
        {
            break;
        }
    }

    if(Idx > 0)
    {
        for(; Idx > 0; -- Idx)
        {
            _GroupAdd(pFixMsg, pLeading);
        }

        pItem = NuHashSearch(pFixMsg->Hash, &Tag, sizeof(int), &HashIdx);
    }

	if (pItem != NULL)
	{
		return &(((NuFixMsgInfo_t *)NuHashItemGetValue(pItem))->TagVal);
	}
	else
	{
		return NULL;
	}
}

int NuFixMsgGroupAssign(NuFixMsg_t *pFixMsg, int Tag, int Idx, char *pVal, size_t Len)
{
    NuFixTag_t *pTag = NuFixMsgGroupFind(pFixMsg, Tag, Idx);

    if(!pTag)
    {
        return NU_FAIL;
    }

    _FieldAssign(pTag, pVal, Len);

    return NU_OK;
}

int NuFixMsgGroupSet(NuFixMsg_t *pFixMsg, int Tag, int Idx, char *pVal, size_t Len)
{
    NuFixTag_t *pTag = NuFixMsgGroupFind(pFixMsg, Tag, Idx);

    if(!pTag)
    {
        return NU_FAIL;
    }

    _FieldSet(pTag, pVal, Len);

    return NU_OK;
}

int NuFixMsgGroupVPrintf(NuFixMsg_t *pFixMsg, int Tag, int Idx, char *Format, va_list ArguList)
{
    NuFixTag_t *pTag = NuFixMsgGroupFind(pFixMsg, Tag, Idx);

    if(!pTag)
    {
        return NU_FAIL;
    }

    _FieldSprintf(pTag, Format, ArguList);

    return NU_OK;
}

int NuFixMsgGroupPrintf(NuFixMsg_t *pFixMsg, int Tag, int Idx, char *Format, ...)
{
    va_list ArguList;

    va_start(ArguList, Format);
    NuFixMsgGroupVPrintf(pFixMsg, Tag, Idx, Format, ArguList);
    va_end(ArguList);

    return NU_OK;
}

int NuFixMsgGroupRemove(NuFixMsg_t *pFixMsg, int Tag, int Idx)
{
    unsigned int    idx = 0;
    NuHashItem_t    *pItem = NuHashSearch(pFixMsg->Hash, &Tag, sizeof(int), &idx);

    if(pItem == NULL)
    {
        return NU_FAIL;
    }

    pItem = NuHashRightMost(pItem);

    for(; Idx > 0; -- Idx)
    {
        if((pItem = NuHashLeft(pItem)) == NULL)
        {
            return NU_OK;
        }
    }

    _FieldRemove(&(((NuFixMsgInfo_t *)NuHashItemGetValue(pItem))->TagVal));
    return NU_OK;
}

int NuFixMsgInstanceRemove(NuFixMsg_t *pFixMsg, int LeadingTag, int Idx)
{
    unsigned int    idx = 0;
    NuHashItem_t    *pItem = NuHashSearch(pFixMsg->Hash, &LeadingTag, sizeof(int), &idx);
	NuFixMsgInfo_t  *pInfo = NULL;
    base_vector_t   *pVec = NULL;
    base_vector_it  VecIt;

    if(pItem == NULL)
    {
        return NU_FAIL;
    }

	pInfo = NuHashItemGetValue(pItem);
    pVec = pInfo->RepeatingGroupMember;
    base_vector_it_set(VecIt, pVec);

    while(VecIt != base_vector_it_end(pVec))
    {
        pItem = NuHashRightMost(((NuFixMsgInfo_t *)(*VecIt))->Item);
        for(int Cnt = 0; Cnt < Idx; ++ Cnt)
        {
            if((pItem = NuHashLeft(pItem)) == NULL)
            {
                return NU_FAIL;
            }
        }

		pInfo = NuHashItemGetValue(pItem);	
        _FieldRemove(&(pInfo->TagVal));

        ++ VecIt;
    }

    return NU_OK;
}

NuFixTag_t *NuFixMsgGroupGet(NuFixMsg_t *pFixMsg, int Tag, int Idx)
{
    unsigned int    idx = 0;
    NuHashItem_t    *pItem = NuHashSearch(pFixMsg->Hash, &Tag, sizeof(int), &idx);

    if(pItem == NULL)
    {
        return NULL;
    }

    pItem = NuHashRightMost(pItem);

    for(; Idx > 0; -- Idx)
    {
        if((pItem = NuHashLeft(pItem)) == NULL)
        {
            return NULL;
        }
    }

    return &(((NuFixMsgInfo_t *)NuHashItemGetValue(pItem))->TagVal);
}

char *NuFixMsgGetVal(NuFixTag_t *pTag)
{
    if(pTag == NULL)
    {
        return NULL;
    }

    return (char *)(pTag->Value);
}

size_t NuFixMsgGetSize(NuFixTag_t *pTag)
{
    if(pTag == NULL)
    {
        return 0;
    }

    return pTag->Len;
}

int NuFixMsgGetKind(NuFixTag_t *pTag)
{
    if(pTag == NULL)
    {
        return -1;
    }

    return ((NuFixMsgInfo_t *)(pTag->Info))->Kind;
}

static void _NuFixMsgParse(NuFixMsg_t *pFixMsg, char *Msg)
{
    int             RepeatingInstance = 0, CurrentInstance = 0, Cnt = 0;
    unsigned int    idx = 0;
    char            *ptr = (char *)NuStrGet(pFixMsg->InMsg);
    char            *pTagStart = NULL, *pVal = NULL;
    NuHashItem_t    *pItem = NULL;
    NuFixMsgInfo_t  *pInfo = NULL, *pLeading = NULL;
    NuHash_t        *pHash = pFixMsg->Hash;

    if(*ptr == NuFixMsgDelimiter)
    {
        pTagStart = ++ ptr;
    }
    else
    {
        pTagStart = ptr;
    }

    while((ptr = strchr(ptr, NuFixMsgTagValSep)) != NULL)
    {
        *ptr = NuFixMsgTerminator;
        int Tag = atoi(pTagStart);

        pItem = NuHashSearch(pHash, &Tag, sizeof(int), &idx);
        if(pItem  == NULL)
        {
            if((ptr = strchr(++ ptr, NuFixMsgDelimiter)) != NULL)
            {
                pTagStart = ++ ptr;
            }
            else
            {
                break;
            }

            continue;
        }
        else if(*(pVal = ++ ptr) == NuFixMsgDelimiter)
        {
            pTagStart = ++ ptr;
            continue;
        }

        if((ptr = strchr(ptr, NuFixMsgDelimiter)) == NULL)
        {
            break;
        }

        *ptr = NuFixMsgTerminator;
        pInfo = (NuFixMsgInfo_t *)(NuHashItemGetValue(pItem));

        if(pInfo->Kind & NuFixMsgKindRepeatingGroupMember)
        {
            if(RepeatingInstance > 0)
            {
                pItem = NuHashRightMost(pItem);
                if(CurrentInstance > 0)
                {
                    for(Cnt = 1; Cnt < CurrentInstance; ++ Cnt)
                    {
                        pItem = NuHashLeft(pItem);
                    }

                    if(((NuFixMsgInfo_t *)NuHashItemGetValue(pItem))->TagVal.Value != NULL)
                    {
                        if(++ CurrentInstance > RepeatingInstance)
                        {
                            -- CurrentInstance;
                        }
                        else
                        {
                            if(CurrentInstance >= NuHashItemGetKeyCollision(pItem))
                            {
                                _GroupAdd(pFixMsg, pLeading);
                            }

                            _FieldAssign(&(((NuFixMsgInfo_t *)NuHashItemGetValue(NuHashLeft(pItem)))->TagVal), pVal, ptr - pVal);
                        }
                    }
                    else
                    {
                        _FieldAssign(&(((NuFixMsgInfo_t *)NuHashItemGetValue(pItem))->TagVal), pVal, ptr - pVal);
                    }
                }
                else
                {
                    ++ CurrentInstance;
                    _FieldAssign(&(((NuFixMsgInfo_t *)NuHashItemGetValue(pItem))->TagVal), pVal, ptr - pVal);
                }
            }
        }
        else
        {
            if(pLeading != NULL)
            {
                pLeading->CurrentRepeatingInstance = CurrentInstance;
                pLeading = NULL;
            }

			_FieldAssign(&(((NuFixMsgInfo_t *)NuHashItemGetValue(pItem))->TagVal), pVal, ptr - pVal);

            if(pInfo->Kind & NuFixMsgKindRepeatingGroupLeader)
            {
                CurrentInstance = 0;
                pLeading = pInfo;
                RepeatingInstance = atoi(pVal);
            }
        }

        pTagStart = ++ ptr;
    }

    if(pLeading != NULL)
    {
        pLeading->CurrentRepeatingInstance = CurrentInstance;
        pLeading = NULL;
    }

    return;
}

void NuFixMsgParse(NuFixMsg_t *pFixMsg, char *Msg)
{
    NuStrCpy(pFixMsg->InMsg, Msg);
    _NuFixMsgParse(pFixMsg, Msg);
}

void NuFixMsgParseByLen(NuFixMsg_t *pFixMsg, char *Msg, size_t MsgLen)
{
    NuStrNCpy(pFixMsg->InMsg, Msg, MsgLen);
    _NuFixMsgParse(pFixMsg, Msg);
}

static void _AppendTag(NuStr_t *pStr, NuFixMsgInfo_t *Tag, char *pVal, size_t ValLen)
{
    NuStrCat(pStr, Tag->TagStr);
    NuStrNCat(pStr, pVal, ValLen);
    NuStrCatChr(pStr, NuFixMsgDelimiter);

    return;
}

static void _GenTag(NuStr_t *pStr, NuFixMsgInfo_t *Tag)
{
    NuHashItem_t    *pItem = Tag->Item;
    base_vector_t   *pVec = Tag->RepeatingGroupMember;
    base_vector_it  VecIt;
    NuFixMsgInfo_t  *pInfo = NULL;
	NuFixTag_t      *pTag = NULL;


	pInfo = (NuFixMsgInfo_t *)NuHashItemGetValue(pItem);
	pTag = &(pInfo->TagVal);

    if(pTag->Value != NULL)
    {
        _AppendTag(pStr, Tag, pTag->Value, pTag->Len);

        if(Tag->Kind & NuFixMsgKindRepeatingGroupLeader)
        {
            int Cnt = Tag->CurrentRepeatingInstance;

            for(int i = 0; i < Cnt; ++ i)
            {
                base_vector_it_set(VecIt, pVec);
                while(VecIt != base_vector_it_end(pVec))
                {
                    pItem = ((NuFixMsgInfo_t *)(*VecIt))->Item;
                    for(int i2 = 0; i2 < i; ++ i2)
                    {
                        pItem = NuHashLeft(pItem);
                    }

                    pInfo = (NuFixMsgInfo_t *)NuHashItemGetValue(pItem);
					pTag = &(pInfo->TagVal);
                    if(pTag->Value != NULL)
                    {
                        _AppendTag(pStr, pInfo, pTag->Value, pTag->Len);
                    }

                    ++ VecIt;
                }
            }
        }
    }

    return;
}

NuStr_t *NuFixMsgGen(NuFixMsg_t *pFixMsg, int Gen)
{
    int             Len = 0;
    char            *ptr = NULL;
    base_vector_t   *pVec = NULL;
    base_vector_it  VecIt;
    NuStr_t         *pStr = pFixMsg->OutMsg;

    NuStrClear(pStr);

    if(Gen & NuFixMsgKindShell)
    {
        NuStrCat(pStr, NuFixMsgShellEight);

		NuFixTag_t *pTag = NuFixMsgFieldGet(pFixMsg, 8);
        NuStrCat(pStr, pTag->Value);
        NuStrCat(pStr, NuFixMsgShellNine);

        Len = NuStrSize(pStr);
    }

    if(Gen & NuFixMsgKindHeader)
    {
        base_vector_it_set(VecIt, pVec = pFixMsg->Header);
        while(VecIt != base_vector_it_end(pVec))
        {
            _GenTag(pStr, *VecIt);
            ++ VecIt;
        }
    }

    if(Gen & NuFixMsgKindBody)
    {
        base_vector_it_set(VecIt, pVec = pFixMsg->Body);
        while(VecIt != base_vector_it_end(pVec))
        {
            _GenTag(pStr, *VecIt);
            ++ VecIt;
        }
    }

    if(Gen & NuFixMsgKindShell)
    {
        ptr = strchr(NuStrGet(pStr), NuFixMsgDelimiter) + 3;

        ptr += NuCStrPrintInt(ptr, NuStrSize(pStr) - Len, 5);
        *ptr = NuFixMsgDelimiter;
		
        NuStrAppendPrintf(pStr, "%s%03u%c", NuFixMsgShellTen, NuFixMsgGenCheckSum(NuStrGet(pStr), NuStrSize(pStr)), NuFixMsgDelimiter);
    }

    return pStr;
}

char *NuFixMsgTakeOutMsg(NuFixMsg_t *pFixMsg)
{
    return (char *)NuStrGet(pFixMsg->OutMsg);
}

size_t NuFixMsgGetOutMsgSize(NuFixMsg_t *pFixMsg)
{
    return NuStrSize(pFixMsg->OutMsg);
}

unsigned int NuFixMsgGenCheckSum(const char *Msg, size_t Len)
{
    unsigned int CheckSum = 0;

    while(Len --)
    {
        CheckSum += (unsigned int)(*Msg ++);
    }

    return (CheckSum & 255);
}

void NuFixMsgTagExchange(NuFixMsg_t *pFixMsg, int Tag1, int Tag2)
{
    char            *TmpValue = NULL;
    NuFixTag_t      *pTag1 = NuFixMsgFieldGet(pFixMsg, Tag1);
    NuFixTag_t      *pTag2 = NuFixMsgFieldGet(pFixMsg, Tag2);

    if(pTag1 != NULL && pTag2 != NULL)
    {
        TmpValue = pTag1->Value;
        size_t TmpLen = pTag1->Len;
    
        pTag1->Value = pTag2->Value;
        pTag1->Len = pTag2->Len;

        pTag2->Value = TmpValue;
        pTag2->Len = TmpLen;
    }

    return;
}

