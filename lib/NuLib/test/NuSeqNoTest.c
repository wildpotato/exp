#include <stdio.h>
#include <stdlib.h>

#include <NuSeqNo.h>

#if 0
int main(int argc, char **argv)
{
    NuSeqNo_t *pSeq = NULL;
    char Buf[5] = {0};

    if (NuSeqNoNew(&pSeq, &NuSeqNoType_String, 4, "./", "TEST") < 0)
    {
        return EXIT_FAILURE;
    }
//    NuSeqNoSetMinNo(pSeq, "1111");
    NuSeqNoSetMaxNo(pSeq, "5556");
//    NuSeqNoSetCurNo(pSeq, "1111");

    while(NuSeqNoPop(pSeq, (void *)Buf) == NU_OK)
    {
        printf("[NuSeqNo] Get(%s)\n", Buf);
    }
    
    NuSeqNoFree(pSeq);
    return EXIT_SUCCESS;
}
#else
int main(int argc, char **argv)
{
    NuSeqNo_t *pSeq = NULL;
	int i = 0;
    char Buf[10+1] = {0};

    if (NuSeqNoNew(&pSeq, &NuSeqNoType_String, 10, "./", "TEST") < 0)
    {
        return EXIT_FAILURE;
    }

	//strcpy(Buf, "1234");
	//NuSeqNoSetCurNo(pSeq, Buf);

	for (i = 0; i < 10; i++)
	{
		NuSeqNoPop(pSeq, (void *)Buf);
		printf("[NuSeqNo] Get(%s)\n", Buf);
	}

	NuSeqNoGetCurNo(pSeq, Buf);
	printf("[%s]\n", Buf);
    NuSeqNoFree(pSeq);
    return EXIT_SUCCESS;
}
#endif
