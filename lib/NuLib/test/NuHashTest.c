#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <NuHash.h>

#define DATA "123456789012345678901234567890"
#define DATA2 "12345678901234567890"
#define ITEM_CNT 60

#define errChk(rc) \
do \
{ \
	if (!(rc)) { printf("Check Fail(%d): in line(%d)\n", rc, __LINE__); } \
} while(0)

typedef struct _TestItem_t
{
	int  ID;
	char Data[80];
} TestItem_t;

void _NuHashForEachPrint(NuHashItem_t *Item, void *Argu)
{
	printf("[NuHash]ForEachPrint(%02d)result(%s)\n", *((int *)NuHashItemGetKey(Item)), (char *)NuHashItemGetValue(Item));
}

void _NuHashForEachRemoveMid(NuHashItem_t *Item, void *Argu)
{
	NuHash_t *pHash = (NuHash_t *)Argu;
	unsigned int Idx = 0;
	NuHashSearch(pHash, NuHashItemGetKey(Item), sizeof(int), &Idx);
	if (NuHashRight(Item) && NuHashLeft(Item))
	{
		printf("[NuHash]ForEachRemoveMid(%02d)(%s)\n",*((int *)NuHashItemGetKey(Item)), (char *)NuHashItemGetValue(Item));
		NuHashRemove(pHash, Item, Idx);
	}
}

void _NuHashForEachRemove(NuHashItem_t *Item, void *Argu)
{
	NuHash_t *pHash = (NuHash_t *)Argu;
	unsigned int Idx = 0;
	if (!strchr((char *)NuHashItemGetValue(Item), '-'))
	{
		printf("[NuHash]ForEachRemove(%02d)(%s)\n",*((int *)NuHashItemGetKey(Item)), (char *)NuHashItemGetValue(Item));
		NuHashSearch(pHash, NuHashItemGetKey(Item), sizeof(int), &Idx);
		NuHashRemove(pHash, Item, Idx);
	}
}

int main(int argc, char **argv)
{
	NuHash_t *pHash = NULL;
	NuHashItem_t *pItem = NULL;
	int i = 0;
	unsigned int Idx = 0;
	TestItem_t Items[ITEM_CNT] = {{0}};

	NuHashNew(&pHash, 10);
	for (; i < ITEM_CNT; i++)
	{
		Items[i].ID = i;
		sprintf(Items[i].Data, "%02d-%s", Items[i].ID, DATA);

		if(!NuHashSearch(pHash, &(Items[i].ID), sizeof(int), &Idx))
		{
			pItem = NuHashAdd(pHash, &(Items[i].ID), sizeof(int), DATA, Idx);
			NuMultiHashAdd(pHash, pItem, Items[i].Data);
		}
		else
		{
			printf("[NuHash] Add key(%02d) fail\n", Items[i].ID);
		}
	}

	assert((NuHashGetNum(pHash) == ITEM_CNT * 2));

	for (i = 0; i < ITEM_CNT; i++)
	{
		pItem = NuHashSearch(pHash, &i, sizeof(int), &Idx);

		NuMultiHashAdd(pHash, pItem, DATA2);
		if (pItem)
		{
			do
			{
				printf("[NuHash]Search(%02d)(%ld) result(%s)\n", *((int *)NuHashItemGetKey(pItem)), NuHashItemGetKeyLen(pItem), (char *)NuHashItemGetValue(pItem));
				if (strchr((char *)NuHashItemGetValue(pItem), '-') == NULL)
				{
					NuHashItemSetValue(pItem, Items[i].Data);
				}
				else
				{
					NuHashItemSetValue(pItem, DATA);
				}
			} while((pItem = NuHashRight(pItem)) != NULL);
		}
		else
		{
			printf("[NuHash]Search(%02d) fail\n", Items[i].ID);
		}
	}

	for (i = 0; i < ITEM_CNT; i++)
	{
		pItem = NuHashSearch(pHash, &i, sizeof(int), &Idx);
		assert(NuHashItemGetKeyCollision(pItem) == 3);
	}

	printf("[NuHash] ============================\n");
	NuHashForEach(pHash, &(Items[0].ID), sizeof(int), _NuHashForEachPrint, pHash);

	printf("[NuHash] ============================\n");
	NuHashForEach(pHash, NULL, sizeof(int), _NuHashForEachPrint, pHash);
	NuHashForEach(pHash, NULL, sizeof(int), _NuHashForEachRemoveMid, pHash);
	assert(NuHashGetNum(pHash) == ITEM_CNT * 2);
	printf("[NuHash] ============================\n");
	NuHashForEach(pHash, NULL, sizeof(int), _NuHashForEachPrint, pHash);
	NuHashForEach(pHash, NULL, sizeof(int), _NuHashForEachRemove, pHash);
	assert(NuHashGetNum(pHash) == ITEM_CNT);
	printf("[NuHash] ============================\n");
	NuHashForEach(pHash, NULL, sizeof(int), _NuHashForEachPrint, pHash);

	NuHashClear(pHash);
	assert(NuHashGetNum(pHash) == 0);
	printf("[NuHash] ============================\n");
	NuHashForEach(pHash, NULL, sizeof(int), _NuHashForEachPrint, pHash);

	if(pHash)
	{
		NuHashFree(pHash);
	}
	return EXIT_SUCCESS;
}
