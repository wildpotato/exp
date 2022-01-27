
#include "NuCStr.h"
#include "NuNum.h"

struct _NuNum_t
{
	char	Sign;
	int		AccuDigit;
	int		IntLen;
	int		TotalLen;
	int		*Alloc;
	int		*Num;
};

#define	NuNumSign_Positive	0
#define	NuNumSign_Negative	1

#define	NuNumIsNegative(pNum)	(((pNum)->Sign) & NuNumSign_Negative)
#define	NuNumChangeSign(pNum)	(((pNum)->Sign) ^= NuNumSign_Negative)

static void _NuNumPutNegative(NuNum_t *pNum)
{
	int	TotalLen = pNum->TotalLen;
	int	*ptr = pNum->Num;

	while(TotalLen --)
	{
		(*ptr) *= (-1);
		++ ptr;
	}

	NuNumChangeSign(pNum);

	return;
}

static void _NuNumRefresh(NuNum_t *pNum)
{
	int	Len = pNum->TotalLen;
	int	*pDigit = pNum->Num;
	int	Cnt = Len, Debt = 0;

	while(*pDigit == 0)
    {
		++ pDigit;
    }

	if(*pDigit < 0)
    {
		_NuNumPutNegative(pNum);
    }

	pDigit = pNum->Num + Len - 1;

	while(Cnt --)
	{
		*pDigit += Debt;
		Debt = 0;

		if(*pDigit > 9)
		{
			Debt = (*pDigit) / 10;
			(*pDigit) %= 10;
		}
		else if(*pDigit < 0)
		{
			(*pDigit) += 10;
			Debt = -1;
		}

		-- pDigit;
	}

	if(Debt < 0)
	{
		Debt *= (-1);
		NuNumChangeSign(pNum);
	}

	while(Debt > 0)
	{
		if(pNum->Num != pNum->Alloc)
        {
			-- pNum->Num;
        }
		else
        {
			memmove(pNum->Num + 1, pNum->Num, (pNum->TotalLen) * sizeof(int));
        }

		*(pNum->Num) = Debt % 10;

		++ (pNum->IntLen);
		++ (pNum->TotalLen);

		Debt /= 10;
	}

	pDigit = pNum->Num;
	while(*pDigit == 0)
    {
		++ pDigit;
    }

	Cnt = pDigit - pNum->Num;
	if(Cnt > pNum->IntLen)
    {
		Cnt = pNum->IntLen;
    }

	if(Cnt > 0)
	{
		pNum->TotalLen -= Cnt;
		pNum->IntLen -= Cnt;
		pNum->Num += Cnt;
	}

	return;
}

static unsigned int _NuNumToInt(NuNum_t *pNum)
{
	unsigned int	Result = 0, Cnt = pNum->TotalLen;
	int				*pDigit = pNum->Num;

	while(Cnt --)
	{
		Result = Result * 10 + (*pDigit);
		++ pDigit;
	}

	return Result;
}

int NuNumNew(NuNum_t **pNum, int Digit)
{
	int RC = NU_OK;

	if(!((*pNum) = (NuNum_t *)malloc(sizeof(NuNum_t))))
    {
		NUGOTO(RC, NU_MALLOC_FAIL, EXIT);
    }

	if(!((*pNum)->Alloc = (int *)calloc(Digit + 1, sizeof(int))))
    {
		NUGOTO(RC, NU_MALLOC_FAIL, EXIT);
    }

	(*pNum)->AccuDigit = Digit + 1;
	NuNumReset(*pNum);

EXIT:
	if(RC < 0)
    {
		NuNumFree(*pNum);
    }

	return RC;
}

void NuNumFree(NuNum_t *pNum)
{
	if(pNum != NULL)
	{
		if(pNum->Alloc != NULL)
        {
			free(pNum->Alloc);
        }

		free(pNum);
	}

	return;
}

void NuNumFromStr(NuNum_t *pNum, char *NumStr)
{
	int		*nptr = NULL;
	char	*ptr = NumStr, *ptr2 = NULL, *pDot = NULL;

	if(*ptr == '-')
	{
		pNum->Sign = NuNumSign_Negative;
		ptr2 = ++ ptr;
	}
	else
	{
		pNum->Sign = NuNumSign_Positive;
		ptr2 = ptr;
	}

	nptr = pNum->Num;

	while(*ptr != '\0')
	{
		if(*ptr == '.')
		{
			pNum->IntLen = ptr - ptr2;
			pDot = ptr ++;
			continue;
		}

		*nptr = (int)(*ptr - '0');

		++ ptr;
		++ nptr;
	}

	if(pDot != NULL)
    {
		pNum->TotalLen = ptr - ptr2 - 1;
    }
	else
    {
		pNum->IntLen = pNum->TotalLen = ptr - ptr2;
    }

	return;
}

void NuNumReset(NuNum_t *pNum)
{
	memset(pNum->Alloc, 0, (pNum->AccuDigit) * sizeof(int));
	pNum->Sign = NuNumSign_Positive;
	pNum->IntLen = pNum->TotalLen = 0;
	pNum->Num = pNum->Alloc + 1;

    return;
}

int NuNumToStr(NuNum_t *pNum, char *NumStr)
{
	char	*ptr = NumStr;
	int		Cnt = 0, *nptr = pNum->Num;

	if(NuNumIsNegative(pNum))
	{
		*ptr = '-';
		++ ptr;
	}

	if(!(pNum->IntLen))
	{
		*ptr = '0';
		++ ptr;
	}

	for(Cnt = pNum->IntLen; Cnt --;)
	{
		ptr += NuCStrPrintInt(ptr, *nptr, 0);
		++ nptr;
	}

	if((Cnt = pNum->TotalLen - pNum->IntLen) > 0)
	{
		*ptr = '.';
		++ ptr;

		for(; Cnt --;)
		{
			ptr += NuCStrPrintInt(ptr, *nptr, 0);
			++ nptr;
		}
	}

	*ptr = '\0';

	return ptr - NumStr;
}

void NuNumAdd(NuNum_t *pNum1, NuNum_t *pNum2, NuNum_t *pResult)
{
	int		LenDiff = pNum1->IntLen - pNum2->IntLen;
	int		*ptr2 = NULL, *ptrR = NULL, *pShortEnd = NULL;
	int		IspNum1Negative = 0, IspNum2Negative = 0;

	NuNum_t	*pLong = NULL, *pShort = NULL;

	if(NuNumIsNegative(pNum1))
	{
		IspNum1Negative = 1;
		_NuNumPutNegative(pNum1);
	}

	if(NuNumIsNegative(pNum2))
	{
		IspNum2Negative = 1;
		_NuNumPutNegative(pNum2);
	}

	if(pResult->TotalLen != 0)
    {
		NuNumReset(pResult);
    }

	if(LenDiff < 0)
	{
		LenDiff *= (-1);
		pLong = pNum2;
		pShort = pNum1;
	}
	else
	{
		pLong = pNum1;
		pShort = pNum2;
	}

	pResult->IntLen = pLong->IntLen;
	pResult->Sign = pLong->Sign;
	memcpy(pResult->Num, pLong->Num, (pLong->TotalLen) * sizeof(int));

	ptrR = pResult->Num + LenDiff;
	pShortEnd = (ptr2 = pShort->Num) + pShort->TotalLen;
	while(ptr2 != pShortEnd)
	{
		(*ptrR) += *(ptr2);
		++ ptr2;
		++ ptrR;
	}

	pResult->TotalLen = pLong->IntLen + NuMax(pShort->TotalLen - pShort->IntLen, pLong->TotalLen - pLong->IntLen);
	_NuNumRefresh(pResult);

	if(IspNum1Negative)
    {
		_NuNumPutNegative(pNum1);
    }

	if(IspNum2Negative)
    {
		_NuNumPutNegative(pNum2);
    }

	return;
}

void NuNumSubstract(NuNum_t *pNum1, NuNum_t *pNum2, NuNum_t *pResult)
{
	int		LenDiff = pNum1->IntLen - pNum2->IntLen;
	int		*ptr2 = NULL, *ptrR = NULL, *pShortEnd = NULL;
	int		IspNum1Negative = 0, IspNum2Negative = 0;

	NuNum_t	*pLong = NULL, *pShort = NULL;

	if(NuNumIsNegative(pNum1))
	{
		IspNum1Negative = 1;
		_NuNumPutNegative(pNum1);
	}

	if(NuNumIsNegative(pNum2))
	{
		IspNum2Negative = 1;
		_NuNumPutNegative(pNum2);
	}

	if(pResult->TotalLen != 0)
    {
		NuNumReset(pResult);
    }

	if(LenDiff < 0)
	{
		LenDiff *= (-1);
		pLong = pNum2;
		pShort = pNum1;
	}
	else
	{
		pLong = pNum1;
		pShort = pNum2;
	}

	pResult->IntLen = pLong->IntLen;
	pResult->Sign = pLong->Sign;
	memcpy(pResult->Num, pLong->Num, (pLong->TotalLen) * sizeof(int));

	ptrR = pResult->Num + LenDiff;
	pShortEnd = (ptr2 = pShort->Num) + pShort->TotalLen;
	while(ptr2 != pShortEnd)
	{
		(*ptrR) -= *(ptr2);
		++ ptr2;
		++ ptrR;
	}

	pResult->TotalLen = pLong->IntLen + NuMax(pShort->TotalLen - pShort->IntLen, pLong->TotalLen - pLong->IntLen);
	_NuNumRefresh(pResult);

	if(IspNum1Negative)
    {
		_NuNumPutNegative(pNum1);
    }

	if(IspNum2Negative)
    {
		_NuNumPutNegative(pNum2);
    }

	return;
}

void NuNumMultiply(NuNum_t *pNum1, NuNum_t *pNum2, NuNum_t *pResult)
{
	int	Cnt1 = pNum1->TotalLen - 1, Cnt2 = pNum2->TotalLen;
	int	*p1 = pNum1->Num + Cnt1, *p2 = NULL, *pR = NULL, *pEnd = NULL;

	if(pResult->TotalLen != 0)
    {
		NuNumReset(pResult);
    }

	pResult->Sign = (pNum1->Sign) ^ (pNum2->Sign);
	pResult->IntLen = pNum1->IntLen + pNum2->IntLen - 1;
	pResult->TotalLen = Cnt1 + Cnt2;

	for(; Cnt1 >= 0; -- Cnt1, -- p1)
	{
		if(*p1 == 0)
        {
			continue;
        }

		p2 = pNum2->Num + Cnt2 - 1;
		pEnd = pResult->Num + Cnt1 - 1;
		pR = pEnd + Cnt2;

		while(pR >= pEnd)
		{
			*pR += ((*p1) * (*p2));
			-- pR;
			-- p2;
		}
	}

	_NuNumRefresh(pResult);

	return;
}

void NuNumDivide(NuNum_t *pNum1, NuNum_t *pNum2, NuNum_t *pResult, int AccuDigit)
{
	int				FracLen = (pNum1->TotalLen - pNum1->IntLen) - (pNum2->TotalLen - pNum2->IntLen);
	unsigned int	Dividend = 0, Divisor = _NuNumToInt(pNum2), Cnt = 0;
	int				*pDigit = pNum1->Num, *pQuotient = NULL;

	if(pResult->TotalLen != 0)
    {
		NuNumReset(pResult);
    }

	if(!Divisor)
    {
		return;
    }

	pQuotient = pResult->Num;

	pResult->Sign = (pNum1->Sign) ^ (pNum2->Sign);

	for(Cnt = pNum1->TotalLen; Cnt --;)
	{
		Dividend = Dividend * 10 + (*pDigit);
		++ pDigit;

		if(Dividend >= Divisor)
		{
			*pQuotient = Dividend / Divisor;
			Dividend -= Divisor * (*pQuotient);
		}
		else
        {
			*pQuotient = 0;
        }

		++ pQuotient;
	}

	if(!Dividend)
    {
		pQuotient += (AccuDigit - FracLen);
    }
    else
	{
		while(AccuDigit >= FracLen)
		{
			Dividend *= 10;

			if(Dividend >= Divisor)
			{
				*pQuotient = Dividend / Divisor;
				Dividend -= Divisor * (*pQuotient);
			}
			else
            {
				*pQuotient = 0;
            }

			++ pQuotient;
			++ FracLen;
		}

		-- pQuotient;

		if((*pQuotient) > 4)
        {
			*(pQuotient - 1) += 1;
        }

		*pQuotient = 0;
	}

	pResult->IntLen = (pResult->TotalLen = pQuotient - (pResult->Num)) - AccuDigit;

	_NuNumRefresh(pResult);
	return;
}

int NuNumCompare(NuNum_t *pNum1, NuNum_t *pNum2)
{
	int	Diff = 0;
	int	*pInt1 = pNum1->Num, *pInt2 = pNum2->Num;

	if((Diff = (int)(pNum1->Sign) - (int)(pNum2->Sign)) != 0)
    {
		return (-1) * Diff;
    }

	if((Diff = pNum1->IntLen - pNum2->IntLen) != 0)
    {
		return ((int)(pNum1->Sign)) ? (-Diff) : (Diff);
    }

	for(Diff = pNum1->TotalLen; Diff > 0; -- Diff)
	{
		if(*pInt1 != *pInt2)
        {
			return *pInt1 - *pInt2;
        }

		++ pInt1;
		++ pInt2;
	}

	return 0;
}

void NuNumLeftShift(NuNum_t *pNum, int Digit)
{
	int	TotalLen = pNum->TotalLen + Digit;

	if(TotalLen > pNum->AccuDigit)
    {
		TotalLen = pNum->AccuDigit;
    }

	pNum->IntLen += Digit;
	pNum->TotalLen = TotalLen;

	return;
}

void NuNumRightShift(NuNum_t *pNum, int Digit)
{
	int	Len = pNum->IntLen;
	int	Len2 = pNum->AccuDigit;

	if(Len <= Digit)
	{
		pNum->IntLen = 0;
		if((pNum->TotalLen + (Len = Digit - Len)) > Len2)
		{
			Len = (Len2 - pNum->TotalLen);
			pNum->TotalLen = Len2;
		}
		else
        {
			pNum->TotalLen += Len;
        }

		memmove(pNum->Num + Len, pNum->Num, (pNum->TotalLen) * sizeof(int));
		
        do
		{
			pNum->Num[-- Len] = 0;
		}
		while(Len > 0);
	}
	else
    {
		pNum->IntLen -= Digit;
    }

	return;
}

