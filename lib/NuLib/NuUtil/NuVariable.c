
#include "NuUtil.h"
#include "NuVariable.h"

struct _NuVariable_t
{
    void    *Alloc;
    size_t  Size;
};

int NuVariableNew(NuVariable_t **Var, size_t Size)
{
    int RC = NU_OK;

    if(!(*Var = malloc(sizeof(NuVariable_t))))
    {
        NUGOTO(RC, NU_MALLOC_FAIL, EXIT);
    }

    if(!((*Var)->Alloc = malloc(NU_ALIGN8(Size))))
    {
        NUGOTO(RC, NU_MALLOC_FAIL, EXIT);
    }

    (*Var)->Size = Size;

EXIT:

    if(RC < 0)
    {
        NuVariableFree(*Var);
        *Var = NULL;
    }

    return RC;
}

void NuVariableFree(NuVariable_t *Var)
{
    if(Var)
    {
        if(Var->Size)
        {
            free(Var->Alloc);
        }

        free(Var);
    }

    return;
}

void NuVariableExtend(NuVariable_t *Var, size_t Size)
{
    if(Size > Var->Size)
    {
		size_t RealSize = 0;
        if((Var->Alloc = realloc(Var->Alloc, RealSize = NU_ALIGN8(Size))) != NULL)
        {
            Var->Size = RealSize;
        }
    }

    return;
}

void NuVariableCopy(NuVariable_t *Var, const void *Data, size_t DataLen)
{
    NuVariableSet(Var, 0, Data, DataLen);
	return;
}

void NuVariableSet(NuVariable_t *Var, size_t Offset, const void *Data, size_t DataLen)
{
    NuVariableExtend(Var, DataLen + Offset);
    memcpy((char *)(Var->Alloc) + Offset, Data, DataLen);

    return;
}

void NuVariableSetByte(NuVariable_t *Var, size_t Offset, char Data)
{
    NuVariableExtend(Var, Offset + 1);
    *(((char *)(Var->Alloc)) + Offset) = Data;

    return;
}

void NuVariableMove(NuVariable_t *Var, size_t Offset, size_t Src, size_t DataLen)
{
    memmove((char *)(Var->Alloc) + Offset, (char *)(Var->Alloc) + Src, DataLen);

    return;
}

int NuVariablePrintf(NuVariable_t *Var, size_t Offset, const char *Format, ...)
{
    int     Len = 0;
    va_list ArguList;

    va_start(ArguList, Format);
    Len = NuVariableVPrintf(Var, Offset, Format, ArguList);
    va_end(ArguList);

    return Len;

}

int NuVariableVPrintf(NuVariable_t *Var, size_t Offset, const char *Format, va_list ArguList)
{
    int     Len = 0;
    va_list Argu;

    va_copy(Argu, ArguList);

    NuVariableExtend(Var, (Len = vsnprintf((char *)(Var->Alloc) + Offset, Var->Size - Offset, Format, Argu) + 1) + Offset);

    vsprintf((char *)(Var->Alloc) + Offset, Format, ArguList);

	va_end(Argu);

    return Len;
}

const void *NuVariableGet(NuVariable_t *Var, size_t Offset)
{
    NuVariableExtend(Var, Offset + 1);
    return (char *)(Var->Alloc) + Offset;
}

size_t NuVariableSize(NuVariable_t *Var)
{
    return Var->Size;
}

