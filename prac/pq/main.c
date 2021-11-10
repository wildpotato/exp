#include <stdio.h>
#include <time.h>    // clock_t, clock()
#include <stdlib.h>  // rand(), free(), malloc()
#include <stdbool.h> // boolean
#include <limits.h>  // INT_MAX

const int SIZE_OF_N = 7;
const int NUM_ELEMENTS[] = {100, 500, 1000, 2000, 3000, 4000, 5000}; // n
const int NUM_OF_OPERATIONS = 5000;                                  // m


#define DELETE_MIN  0
#define INSERT_MIN  1

/************************************************************************
 *    This section contains code snippets for LeftistHeap only.
 ***********************************************************************/

typedef struct LeftistNode {
	int element;
	int dist;
	struct LeftistNode *left;
	struct LeftistNode *right;
} LeftistNode;

struct LeftistNode * New_LeftistNode1(int val)
{
	LeftistNode *node = malloc(sizeof(struct LeftistNode));
	node->element = val;
	node->left = NULL;
	node->right = NULL;
	return node;
}

struct LeftistNode * New_LeftistNode2(int _element, struct LeftistNode *_lt, struct LeftistNode *_rt, int _dist)
{
	struct LeftistNode * node = malloc(sizeof(struct LeftistNode));
	node->element = _element;
	node->left = _lt;
	node->right = _rt;
	node->dist = _dist;
	return node;
}

typedef struct LeftistHeap {
	LeftistNode *root;
} LeftistHeap;

struct LeftistHeap *New_LeftistHeap()
{
	struct LeftistHeap * heap = malloc(sizeof(struct LeftistHeap));
	heap->root = NULL;
	return heap;
}

/*** Forward declaration of member functions for LeftistHeap ***/
int LeftistHeap_findMin(LeftistHeap *);
LeftistNode *LeftistHeap_Merge(LeftistNode *h1, LeftistNode *h2);
LeftistNode *LeftistHeap_Merge1(LeftistNode *h1, LeftistNode *h2);
void LeftistHeap_swapChildren(LeftistNode *);
void LeftistHeap_deleteMin(LeftistHeap *);
void LeftistHeap_insert(LeftistHeap *, int);
bool LeftistHeap_isEmpty(LeftistHeap *);
void LeftistHeap_removeAll(LeftistHeap *);

int LeftistHeap_findMin(LeftistHeap *h) {
	if (LeftistHeap_isEmpty(h)) return INT_MAX;
	return h->root->element;
}


LeftistNode *LeftistHeap_Merge(LeftistNode *h1, LeftistNode *h2) {
	if (h1 == NULL)
		return h2;
	if (h2 == NULL)
		return h1;
	if (h1->element < h2->element)
		return LeftistHeap_Merge1(h1, h2);
	else
		return LeftistHeap_Merge1(h2, h1);
}

LeftistNode *LeftistHeap_Merge1(LeftistNode *h1, LeftistNode *h2) {
	if (h1->left == NULL)
		h1->left = h2;
	else {
		h1->right = LeftistHeap_Merge(h1->right, h2);
		if (h1->left->dist < h1->right->dist)
			LeftistHeap_swapChildren(h1);
		h1->dist = h1->right->dist + 1;
	}
	return h1;
}

void LeftistHeap_swapChildren(LeftistNode *t) {
	LeftistNode *tmp = t->left;
	t->left = t->right;
	t->right = tmp;
}

void LeftistHeap_deleteMin(LeftistHeap *h) {
	if (LeftistHeap_isEmpty(h)) return;
	LeftistNode *oldRoot = h->root;
	h->root = LeftistHeap_Merge(h->root->left, h->root->right);
	free(oldRoot);
}

void LeftistHeap_insert(LeftistHeap *h, int val) {
	h->root = LeftistHeap_Merge(New_LeftistNode1(val), h->root);
}

bool LeftistHeap_isEmpty(LeftistHeap *h) {
	return h->root == NULL;
}

void LeftistHeap_removeAll(LeftistHeap *h) {
	printf("-------------------------------\n");
	while (!LeftistHeap_isEmpty(h)) {
		printf("%d ", LeftistHeap_findMin(h));
		LeftistHeap_deleteMin(h);
	}
	printf("\n-------------------------------\n");
}

/************************************************************************
 *    This section contains code snippets for MinHeap only.
 ************************************************************************/

typedef struct MinHeap {
	int capacity;		// maximum size for min heap
	int size;			// current size for min heap
	int *arr;			// pointer to start of underlying array
} MinHeap;

struct MinHeap *New_MinHeap(int _capacity) {
	struct MinHeap *h = malloc(sizeof(struct MinHeap));
	int * arr = malloc(_capacity *sizeof(int));
	h->capacity = _capacity;
	h->size = 0;
	h->arr = arr;
	return h;
}

void test() {
    int i = 0;
    clock_t clock_start = clock();
    long double time_elapsed = 0.0;
    int OPERATIONS[NUM_OF_OPERATIONS];
    for ( ;i < NUM_OF_OPERATIONS; ++i) {
        OPERATIONS[i] = rand() % 2;
        printf("%d ", NUM_ELEMENTS[rand() % SIZE_OF_N]);
    }
    for ( i = 0; i < NUM_OF_OPERATIONS; ++i) {
        printf("%d", OPERATIONS[i]);
    }
    printf("\n");
    clock_t clock_end = clock();
    time_elapsed = ((long double)(clock_end - clock_start)) / CLOCKS_PER_SEC;
    printf("time elapsed: %Lf\n", time_elapsed);
}

void test1() {
	int arr[] = {6, 2, 4, 8, 1, 5, 3};
	LeftistHeap *h = New_LeftistHeap();
	for (int i = 0; i < 7; ++i) {
		LeftistHeap_insert(h, arr[i]);
	}
	LeftistHeap_removeAll(h);
}

void test2() {
	LeftistHeap *h = New_LeftistHeap();
	LeftistHeap_insert(h, 1);
	LeftistHeap_deleteMin(h);
	LeftistHeap_deleteMin(h);
	LeftistHeap_deleteMin(h);
}

void test3() {
	MinHeap *h = New_MinHeap(10);
}


int main() {
    test3();
    return 0;
}
