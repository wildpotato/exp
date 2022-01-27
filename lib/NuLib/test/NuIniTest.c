#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <NuIni.h>

void _NuIniTraverseCB(NuIni_t *Ini, const char *Section, const char *Key, const char *Value, void *Argu)
{
	printf("[NuIni] Section(%s)Key(%s)Value(%s)\n", Section, Key, Value);
}

void _PrintHint(const char *Hint)
{
	printf("=>(%s) ", Hint);
}

void _PrintHelp()
{
	printf("Supported function:\n");
	printf("    e: Exist\n");
	printf("    f: Find\n");
	printf("    g: Get file name\n");
	printf("    h: Show this page\n");
	printf("    m: Modify\n");
	printf("    s: Save\n");
	printf("    t: Traverse\n");
	printf("\n    q: Quit\n");
}

int main(int argc, char **argv)
{
	NuIni_t *pIni = NULL;
	const char *FilePath = NULL;
	const char *Result = NULL;
	int  RC = 0;
	char Function[4] = {0};
	char Section[256] = {0};
	char Key[256] = {0};
	char Value[256] = {0};

	if (argc < 2)
	{
		printf("usage: %s <ini_file_name>\n", argv[0]);
		return EXIT_FAILURE;
	}
	FilePath = argv[1];

	if (NuIniNew(&pIni, FilePath) < 0)
	{
		goto EXIT;
	}

	_PrintHelp();
	for (;;)
	{
		memset(Function, 0, sizeof(Function));
		memset(Section, 0, sizeof(Section));
		memset(Key, 0, sizeof(Key));
		memset(Value, 0, sizeof(Value));
		printf("=> ");
		scanf("%s", Function);

		if (Function[0] == 'q' || Function[0] == 'Q')
		{
			break;
		}

		switch(Function[0])
		{
		case 'F':
		case 'f':
			_PrintHint("Section Name");
			scanf("%s", Section);
			_PrintHint("Key Name");
			scanf("%s", Key);
			printf("[NuIni]===== NuIniFind =====\n");
			Result = NuIniFind(pIni, Section, Key);
			printf("[NuIni]Section(%s)Key(%s)Value(%s)\n", Section, Key, Result);
			break;
		case 'E':
		case 'e':
			_PrintHint("Section Name");
			scanf("%s", Section);
			printf("[NuIni] Get(%s)\n", Section);
			printf("[NuIni]===== NuIniExist =====\n");
			RC = NuIniSectionExist(pIni, Section);
			printf("[NuIni]Session %sexist\n", RC? "" : "not ");
			break;
		case 'G':
		case 'g':
			printf("[NuIni]===== NuIniGetFileName =====\n");
			Result = NuIniGetFileName(pIni);
			printf("[NuIni]FileName(%s)\n", Result);
			break;
		case 'H':
		case 'h':
			_PrintHelp();
			break;
		case 'M':
		case 'm':
			_PrintHint("Section Name");
			scanf("%s", Section);
			_PrintHint("Key Name");
			scanf("%s", Key);
			_PrintHint("ValueName");
			scanf("%s", Value);
			printf("[NuIni]===== NuIniModify before =====\n");
			Result = NuIniFind(pIni, Section, Key);
			printf("[NuIni]Section(%s)Key(%s)Value(%s)\n", Section, Key, Result);
			NuIniModify(pIni, Section, Key, Value);
			printf("[NuIni]===== NuIniModify after =====\n");
			Result = NuIniFind(pIni, Section, Key);
			printf("[NuIni]Section(%s)Key(%s)Value(%s)\n", Section, Key, Result);
			break;
		case 'S':
		case 's':
			_PrintHint("File Path");
			scanf("%s", Section);
			NuIniSave(pIni, Section);
			break;
		case 'T':
		case 't':
			_PrintHint("Section Name");
			scanf("%s", Key);
			printf("[NuIni]===== NuIniTraverse =====\n");
			NuIniTraverse(pIni, Key, &_NuIniTraverseCB, NULL);
			break;
		default:
			printf("Unsupport function[%s]\n", Function);
		}
	}

EXIT:
	if (pIni)
	{
		NuIniFree(pIni);
	}
	return EXIT_SUCCESS;
}
