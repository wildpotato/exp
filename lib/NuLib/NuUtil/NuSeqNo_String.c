
/* ------------------------------------ */
/* static function                      */
/* ------------------------------------ */
static void _NuSeqNo_Str_Set(NuSeqNo_t *pObj, char *pSeqNo)
{
    char    *p1 = pObj->pSeqFilePos1;
    char    *p2 = pObj->pSeqFilePos2;

    if(pObj->pCurSeqNo == p1)
    {
        *p2 = '\0';
        memcpy(p2, pSeqNo, pObj->Length);
        pObj->pCurSeqNo = p2;
        *p1 = '\0';
    }
    else
    {
        *p1 = '\0';
        memcpy(p1, pSeqNo, pObj->Length);
        pObj->pCurSeqNo = p1;
        *p2 = '\0';
    }

    return;
}

static int NuSeqNo_Str_Init(NuSeqNo_t *pObj, void *pArgu)
{
    memset(pObj->pMaxSeqNo, '9', pObj->Length);
    memset(pObj->pMinSeqNo, '0', pObj->Length);
	return NU_OK;
}

static int NuSeqNo_Str_SetMax(NuSeqNo_t *pObj, void *pSeqNo)
{
    strncpy((char *)(pObj->pMaxSeqNo), (char *)pSeqNo, pObj->Length);
	return NU_OK;
}

static int NuSeqNo_Str_SetMin(NuSeqNo_t *pObj, void *pSeqNo)
{
    strncpy((char *)(pObj->pMinSeqNo), (char *)pSeqNo, pObj->Length);
	return NU_OK;
}

static int NuSeqNo_Str_SetCurent(NuSeqNo_t *pObj, void *pSeqNo)
{
    strncpy((char *)(pObj->pCurSeqNo), (char *)pSeqNo, pObj->Length);
	return NU_OK;
}

static int NuSeqNo_Str_GetCurent(NuSeqNo_t *pObj, void *pSeqNo)
{
    strcpy((char *)pSeqNo, (char *)(pObj->pCurSeqNo));

	return NU_OK;
}

static int NuSeqNo_Str_Pop(NuSeqNo_t *pObj, void *pSeqNo)
{
    if(pObj->Type.Next(pObj->pCurSeqNo, pObj->Length, pSeqNo, pObj->Argu) < 0)
    {
        return NU_FAIL;
    }

    if(pObj->Type.Compare(pObj->pMaxSeqNo, pObj->Length, pSeqNo) < 0 ||
         pObj->Type.Compare(pObj->pMinSeqNo, pObj->Length, pSeqNo) > 0 )
    {
        return NU_FAIL;
    }

    _NuSeqNo_Str_Set(pObj, pSeqNo);

    return NU_OK;
}

static int NuSeqNo_Str_Push(NuSeqNo_t *pObj, void *pSeqNo)
{
    if(pObj->Type.Compare(pObj->pMaxSeqNo, pObj->Length, pSeqNo) <  0 ||
        pObj->Type.Compare(pObj->pMinSeqNo, pObj->Length, pSeqNo) >  0 ||
        pObj->Type.Compare(pObj->pCurSeqNo, pObj->Length, pSeqNo) == 0)
    {
        return NU_FAIL;
    }

    _NuSeqNo_Str_Set(pObj, pSeqNo);

    return NU_OK;
}

static int NuSeqNo_Str_Next(const void *CurSeqNo, int SeqNoLen, void *NewSeqNo, void *arg)
{
    NuCStrPrintLong((char *)NewSeqNo, atol((char *)CurSeqNo) + 1, SeqNoLen);   
    return NU_OK;
}

static int NuSeqNo_Str_Compare(const void *SeqNo1, int SeqNoLen, const void *SeqNo2)
{
    return strcmp((char *)SeqNo1, (char *)SeqNo2);
}

/* ------------------------------------ */
/* global variable                      */
/* ------------------------------------ */
NuSeqNoType_t NuSeqNoType_String = {
                                      .SeqNoInit = &NuSeqNo_Str_Init,
                                      .SetMaxNo  = &NuSeqNo_Str_SetMax,
							          .SetMinNo  = &NuSeqNo_Str_SetMin, 
							          .SetCurNo  = &NuSeqNo_Str_SetCurent,
							          .GetCurNo  = &NuSeqNo_Str_GetCurent,
							          .SeqNoPop  = &NuSeqNo_Str_Pop,
							          .SeqNoPush = &NuSeqNo_Str_Push,
									  .Next      = &NuSeqNo_Str_Next,
									  .Compare   = &NuSeqNo_Str_Compare
                                   };

