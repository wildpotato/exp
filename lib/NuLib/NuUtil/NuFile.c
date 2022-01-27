
#include "NuFile.h"

/*File & Dir function */
/* ====================================================================== */
bool NuIsDir(const char *Path)
{
    struct stat st;

    if(!stat(Path, &st))
    {
        if(S_ISDIR(st.st_mode))
        {
            return true;
        }
    }

    return false;
}

bool NuIsFile(const char *Path)
{
    struct stat st;

    if(!stat(Path, &st))
    {
        if(S_ISREG(st.st_mode))
        {
            return true;
        }
    }

    return false;
}

void NuPathCombine(NuStr_t *Str, const char *DirPath, const char *Name)
{
    NuStrClear(Str);
    NuStrCpy(Str, DirPath);

    if(NuStrGetChr(Str, (NuStrSize(Str) - 1)) != NUFILE_SEPARATOR)
    {
        NuStrCatChr(Str, NUFILE_SEPARATOR);
    }

    NuStrCat(Str, Name);

    return;
}

void NuCreateDir(const char *Path)
{
    if(!NuIsDir(Path))
    {
        mkdir(Path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }

    return;
}

void NuCreateRecursiveDir(const char *Path)
{
    const char  *pSep = Path + 1;
    NuStr_t     *Str = NULL;

    NuStrNew(&Str, NULL);

    while((pSep = strchr(pSep, NUFILE_SEPARATOR)) != NULL)
    {
        ++ pSep;
        NuStrNCpy(Str, Path, pSep - Path);
        NuCreateDir(NuStrGet(Str));
    }
        
    NuCreateDir(Path);

    NuStrFree(Str);

    return;
}

size_t NuFileGetSize(const int FD)
{
    struct stat     stFileStat;

    if(fstat(FD, &stFileStat) < 0)
    {
        return 0;
    }

    return stFileStat.st_size;
}

int NuFileSetSize(int FD, unsigned int len)
{
    return ftruncate(FD, len);
}

void NuFileScanDir(const char *Path, NuFileFn Fn, void *Argu)
{
	struct dirent   **List = NULL;

    if(NuIsDir(Path))
    {
        int Cnt = scandir(Path, &List, NULL, alphasort);
        if(Cnt > 0)
        {
            while(Cnt --)
            {
                Fn(List[Cnt]->d_name, Argu);
                free(List[Cnt]);
            }

            free(List);
        }
    }

    return;
}

