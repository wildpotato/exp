#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "NuTrie.h"

static void _TEST_BUG(NuTrie_t *trie)
{
	int len = 0;
	char szKey[32] = {0};
	char *pVal = NULL;
	NuTrieItem_t *item = NULL;

	len = sprintf(szKey, "%s", "CAFC7");
	pVal = (char *)malloc(sizeof(char) * 128);
	if (pVal != NULL)
	{
		sprintf(pVal, "%s", "CAFC7");

		item = NuTrieItem_Add(trie, szKey, len, pVal);
		if (item == NULL)
		{
			printf("insert fail %s\n", pVal);
		}
	}
	item = NuTrieItem_Find(trie, szKey, len);
	if (item != NULL)
	{
		NuTrieItem_Remove(item);
	}

	len = sprintf(szKey, "%s", "CAFF7");
	pVal = (char *)malloc(sizeof(char) * 128);
	if (pVal != NULL)
	{
		sprintf(pVal, "%s", "CAFF7");

		item = NuTrieItem_Add(trie, szKey, len, pVal);
		if (item == NULL)
		{
			printf("insert fail %s\n", pVal);
		}
	}
}

static void _TEST_INSERT(NuTrie_t *trie, unsigned int cnt)
{
	int i = 0;
	int len = 0;
	char szKey[32] = {0};
	char *pVal = NULL;
	NuTrieItem_t *item = NULL;

	for (i = 0; i < cnt; i++)
	{
		len = sprintf(szKey, "test_%d", i);

		pVal = (char *)malloc(sizeof(char) * 128);
		if (pVal != NULL)
		{
			sprintf(pVal, "%s", szKey);

			item = NuTrieItem_Add(trie, szKey, len, pVal);
			if (item == NULL)
			{
				printf("insert fail %s\n", pVal);
			}
		}
	}
}

static void _TEST_REMOVE(NuTrie_t *trie, unsigned int idx)
{
	int len = 0;
	char szKey[32] = {0};
	NuTrieItem_t *item = NULL;
	len = sprintf(szKey, "test_%d", idx);

	item = NuTrieItem_Find(trie, szKey, len);
	if (item != NULL)
	{
		NuTrieItem_Remove(item);
	}
}

void ForEachFn(NuTrieItem_t *item, void *argu)
{
	char *str = (char *)NuTrieItem_Get_Value(item);
	printf("Cnt = %d [%s]\n", NuTrieItem_Get_Cnt(item), str);
}

void ForEachClearFn(NuTrieItem_t *item, void *argu)
{
	NuTrieItem_Remove(item);
}

int main(int argc, char **argv)
{
	NuTrie_t *trie;
	NuTrieItem_t *item = NULL;

	NuTrieNew(&trie, 128);

	_TEST_INSERT(trie, 5);
	_TEST_INSERT(trie, 15);

	_TEST_REMOVE(trie, 11);
	_TEST_REMOVE(trie, 3);

	item = NuTrieItem_Get_Root(trie);

	printf("Root : Cnt = %d\n", NuTrieItem_Get_Cnt(item));

	NuTrieItem_ForEach(item, &ForEachFn, NULL);

	printf("Root : Cnt = %d\n", NuTrieItem_Get_Cnt(item));
	NuTrieItem_ForEach(item, &ForEachClearFn, NULL);

	_TEST_BUG(trie);

	printf("-----------------------------\n");
	printf("Root : Cnt = %d\n", NuTrieItem_Get_Cnt(item));
	NuTrieItem_ForEach(item, &ForEachFn, NULL);


	NuTrieFree(trie);

	return 0;
}
