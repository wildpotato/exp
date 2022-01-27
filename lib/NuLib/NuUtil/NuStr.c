
#include "NuVariable.h"
#include "NuStr.h"

struct _NuStr_t
{
    NuVariable_t    *Var;
    size_t          Size;
};

int NuStrNewPreAlloc(NuStr_t **pStr, size_t Len)
{
    int RC = NU_OK;

    if(!((*pStr) = (NuStr_t *)malloc(sizeof(NuStr_t))))
    {
        NUGOTO(RC, NU_MALLOC_FAIL, EXIT);
    }

    if(NuVariableNew(&((*pStr)->Var), Len) != NU_OK)
    {
        NUGOTO(RC, NU_MALLOC_FAIL, EXIT);
    }

    (*pStr)->Size = 0;

EXIT:
    if(RC < 0)
    {
        NuStrFree(*pStr);
    }

    return RC;
}

int NuStrNew(NuStr_t **pStr, const char *Value)
{
    int     RC = NU_OK;

    if(Value != NULL)
    {
        size_t Len = strlen(Value);
        RC = NuStrNewPreAlloc(pStr, Len + 1);
        (*pStr)->Size = Len;
        NuVariableSet((*pStr)->Var, 0, Value, (*pStr)->Size);
    }
    else
    {
        RC = NuStrNewPreAlloc(pStr, 16);
    }

    if(RC == NU_OK)
    {
        NuVariableSetByte((*pStr)->Var, (*pStr)->Size, '\0');
    }

    return RC;
}

void NuStrFree(NuStr_t *Str)
{
    if(Str != NULL)
    {
        if(Str->Var != NULL)
        {
            NuVariableFree(Str->Var);
        }

        free(Str);
    }

    return;
}

void NuStrClear(NuStr_t *Str)
{
    Str->Size = 0;
    NuVariableSetByte(Str->Var, 0, '\0');

    return;
}

void NuStrNCat(NuStr_t *Str, const void *Data, size_t DataLen)
{
    NuVariableSet(Str->Var, Str->Size, Data, DataLen);
    Str->Size += DataLen;
    NuVariableSetByte(Str->Var, Str->Size, '\0'); 

    return;
}

void NuStrCat(NuStr_t *Str, const char *Src)
{
    NuStrNCat(Str, Src, strlen(Src));
    return;
}

void NuStrCatChr(NuStr_t *Str, char Chr)
{
    NuVariableSetByte(Str->Var, Str->Size ++, Chr); 
    NuVariableSetByte(Str->Var, Str->Size, '\0'); 

    return;
}

void NuStrNCpy(NuStr_t *Str, const void *Data, size_t DataLen)
{
    NuVariableCopy(Str->Var, Data, DataLen);
    NuVariableSetByte(Str->Var, Str->Size = DataLen, '\0'); 

    return;
}

void NuStrCpy(NuStr_t *Str, const char *Src)
{
    NuStrNCpy(Str, Src, strlen(Src));

	return;
}

/* Perform formatted output into an extensible string object. */
void NuStrAppendPrintf(NuStr_t *Str, const char *Format, ...)
{
    va_list ArguList;
    int     Len = 0;

    va_start(ArguList, Format);
    Len = NuVariableVPrintf(Str->Var, Str->Size, Format, ArguList);
    va_end(ArguList);

    Str->Size += Len - 1;

    return;
}

int NuStrPrintf(NuStr_t *Str, int Offset, const char *Format, ...)
{
    int     Len = 0;
    va_list ArguList;

    va_start(ArguList, Format);
    Len = NuStrVPrintf(Str, Offset, Format, ArguList);
    va_end(ArguList);

    return Len;
}

int NuStrVPrintf(NuStr_t *Str, int Offset, const char *Format, va_list ArguList)
{
    int Len = NuVariableVPrintf(Str->Var, Offset, Format, ArguList) - 1;

    Str->Size = Offset + Len;
    return Len;    
}

void NuStrSetChr(NuStr_t *Str, int Idx, char Chr)
{
    NuVariableSetByte(Str->Var, Idx, Chr);
    return;
}

int NuStrCmp(NuStr_t *Str1, NuStr_t *Str2)
{
    if(Str1->Size != Str2->Size)
    {
        return (Str1->Size - Str2->Size);
    }

    return memcmp(NuVariableGet(Str1->Var, 0), NuVariableGet(Str2->Var, 0), Str1->Size);
}

void NuStrRTrim(NuStr_t *Str)
{
    NuStrRTrimChr(Str, ' ');
    NuStrRTrimChr(Str, '\t');

    return;
}

void NuStrRTrimChr(NuStr_t *Str, char Chr)
{
    size_t      Cnt = Str->Size;
    const char  *p = (const char *)NuVariableGet(Str->Var, Cnt - 1);

    while(Cnt --)
    {
        if(*p != Chr)
        {
			++Cnt;
            break;
        }

        -- p;
    }

	Str->Size = Cnt;
    NuVariableSetByte(Str->Var, Str->Size, '\0');

    return;
}

void NuStrLTrim(NuStr_t *Str)
{
    size_t      Cnt = Str->Size;
    const char  *p = (const char *)NuVariableGet(Str->Var, 0);
    const char  *q = (const char *)NuVariableGet(Str->Var, 0);

    while(Cnt --)
    {
        if(*p != ' ' && *p != '\t')
        {
            break;
        }

        ++ p;
    }

    Str->Size -= (p - q);
    NuVariableMove(Str->Var, 0, (p - q), Str->Size);
    NuVariableSetByte(Str->Var, Str->Size, '\0');

    return;
}

void NuStrReplaceRangeChr(NuStr_t *Str, char Chr, char NewChr, size_t Start, size_t Len)
{
    char    *p = (char *)NuVariableGet(Str->Var, Start);
    char    *d = p + Len;

    while(p != d)
    {
        if(*p == Chr)
        {
            *p = NewChr;
        }

        ++ p;
    }

    return;
}

void NuStrReplaceChr(NuStr_t *Str, char Chr, char NewChr)
{
    NuStrReplaceRangeChr(Str, Chr, NewChr, 0, Str->Size);

    return;
}

const char *NuStrGet(NuStr_t *Str)
{
    return NuVariableGet(Str->Var, 0);
}

char NuStrGetChr(NuStr_t *Str, int Idx)
{
    return *((char *)NuVariableGet(Str->Var, Idx));
}

size_t NuStrSize(NuStr_t *Str)
{
    return Str->Size;
}

void NuStrAppendDump(NuStr_t *Str, void *addr, size_t len)
{
	int i;
	unsigned char buff[17];
	unsigned char *pc = (unsigned char*)addr;

    NuStrClear(Str);

	// Process every byte in the data.
	for (i = 0; i < len; i++) {
		// Multiple of 16 means new line (with line offset).

		if ((i % 16) == 0) {
			// Just don't print ASCII for the zeroth line.
			if (i != 0) {
				NuStrAppendPrintf(Str, "  %s\n", buff);
            }

			// Output the offset.
			NuStrAppendPrintf(Str, "  %04x ", i);
		}

		// Now the hex code for the specific character.
		NuStrAppendPrintf(Str, " %02x", pc[i]);

		// And store a printable ASCII character for later.
		if ((pc[i] < 0x20) || (pc[i] > 0x7e))
        {
			buff[i % 16] = '.';
        }
		else
        {
			buff[i % 16] = pc[i];
        }
		buff[(i % 16) + 1] = '\0';
	}

	// Pad out last line if not exactly 16 characters.
	while ((i % 16) != 0) {
		NuStrAppendPrintf(Str, "   ");
		i++;
	}

	// And print the final ASCII bit.
	NuStrAppendPrintf(Str, "  %s\n", buff);
}
