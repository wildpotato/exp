
/* ------------------------------------ */
/* static function                      */
/* ------------------------------------ */
static void _NuSeqNo_Int_Set(NuSeqNo_t *pObj, int *pSeqNo)
{
    *((int *)(pObj->pCurSeqNo)) = *pSeqNo;
    return;
}

static int NuSeqNo_Int_Init(NuSeqNo_t *pObj, void *pArgu)
{
	*((int *)(pObj->pMaxSeqNo)) = INT_MAX;
	*((int *)(pObj->pMinSeqNo)) = 0;
	return NU_OK;
}

static int NuSeqNo_Int_SetMax(NuSeqNo_t *pObj, void *pSeqNo)
{
	*((int *)(pObj->pMaxSeqNo)) = *((int *)pSeqNo);
	return NU_OK;
}

static int NuSeqNo_Int_SetMin(NuSeqNo_t *pObj, void *pSeqNo)
{
	*((int *)(pObj->pMinSeqNo)) = *((int *)pSeqNo);
	return NU_OK;
}

static int NuSeqNo_Int_SetCurent(NuSeqNo_t *pObj, void *pSeqNo)
{
	*((int *)(pObj->pCurSeqNo)) = *((int *)pSeqNo);
	return NU_OK;
}

static int NuSeqNo_Int_GetCurent(NuSeqNo_t *pObj, void *pSeqNo)
{
	*((int *)pSeqNo) = *((int *)(pObj->pCurSeqNo));
	return NU_OK;
}

static int NuSeqNo_Int_Pop(NuSeqNo_t *pObj, void *pSeqNo)
{
    if(pObj->Type.Next(pObj->pCurSeqNo, pObj->Length, pSeqNo, pObj->Argu) < 0)
    {
        return NU_FAIL;
    }

    if(pObj->Type.Compare(pObj->pMaxSeqNo, pObj->Length, pSeqNo) < 0 ||
         pObj->Type.Compare(pObj->pMinSeqNo, pObj->Length, pSeqNo) > 0)
    {
        return NU_FAIL;
    }

    _NuSeqNo_Int_Set(pObj, pSeqNo);

    return NU_OK;
}

static int NuSeqNo_Int_Push(NuSeqNo_t *pObj, void *pSeqNo)
{
    if (pObj->Type.Compare(pObj->pMaxSeqNo, pObj->Length, pSeqNo) <  0 ||
        pObj->Type.Compare(pObj->pMinSeqNo, pObj->Length, pSeqNo) >  0 ||
        pObj->Type.Compare(pObj->pCurSeqNo, pObj->Length, pSeqNo) == 0)
    {
        return NU_FAIL;
    }

    _NuSeqNo_Int_Set(pObj, pSeqNo);

    return NU_OK;
}

static int NuSeqNo_Int_Next(const void *CurSeqNo, int SeqNoLen, void *NewSeqNo, void *arg)
{
	*((int *)NewSeqNo) = *((int *)CurSeqNo) + 1;

    return NU_OK;
}

static int NuSeqNo_Int_Compare(const void *SeqNo1, int SeqNoLen, const void *SeqNo2)
{
    return *((int *)SeqNo1) - *((int *)SeqNo2);
}

/* ------------------------------------ */
/* global variable                      */
/* ------------------------------------ */
NuSeqNoType_t NuSeqNoType_Int = {
                                   .SeqNoInit = &NuSeqNo_Int_Init,
                                   .SetMaxNo  = &NuSeqNo_Int_SetMax,
                                   .SetMinNo  = &NuSeqNo_Int_SetMin, 
							       .SetCurNo  = &NuSeqNo_Int_SetCurent,
							       .GetCurNo  = &NuSeqNo_Int_GetCurent,
							       .SeqNoPop  = &NuSeqNo_Int_Pop,
							       .SeqNoPush = &NuSeqNo_Int_Push,
								   .Next      = &NuSeqNo_Int_Next,
								   .Compare   = &NuSeqNo_Int_Compare
                                };


