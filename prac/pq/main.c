#include <stdio.h>
#include <time.h>    // clock_t, clock()
#include <stdlib.h>  // rand(), free(), malloc()
#include <stdbool.h> // boolean
#include <limits.h>  // INT_MAX

const int SIZE_OF_N = 7;
const int MAX_RAND_SIZE = 10000;
const int NUM_OF_ELEMENTS[] = {100, 500, 1000, 2000, 3000, 4000, 5000}; // n
const int NUM_OF_OPERATIONS = 5000;                                  // m
enum mode {LEFTIST_HEAP, MIN_HEAP};

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

void LeftistNode_free(LeftistNode *r) {
	if (r == NULL) {
		free(r);
		return;
	}
	LeftistNode_free(r->left);
	LeftistNode_free(r->right);
	free(r);
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
void LeftistHeap_free(LeftistHeap *);

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

void LeftistHeap_free(LeftistHeap *h) {
	LeftistNode_free(h->root);
	free(h);
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

void swap(int *x, int *y) {
	int tmp = *x;
	*x = *y;
	*y = tmp;
}

/*** Forward declaration of member functions for LeftistHeap ***/
void MinHeap_heapify(MinHeap *, int);
int MinHeap_parent(int);
int MinHeap_left(int);
int MinHeap_right(int);
int MinHeap_extractMin(MinHeap *);
void MinHeap_decreaseKey(MinHeap *, int, int);
int MinHeap_findMin(MinHeap *);
void MinHeap_deleteKey(MinHeap *, int);
void MinHeap_insertKey(MinHeap *, int);
void MinHeap_removeAll(MinHeap *);
void MinHeap_free(MinHeap *);

void MinHeap_heapify(MinHeap *h, int i) {
	int l = MinHeap_left(i);
	int r = MinHeap_right(i);
	int smallest = i;
	if (l < h->size && *(h->arr + l) < *(h->arr + i))
		smallest = l;
	if (r < h->size && *(h->arr + r) < *(h->arr + smallest))
		smallest = r;
	if (smallest != i) {
		swap(h->arr + i, h->arr + smallest);
		MinHeap_heapify(h, smallest);
	}
}

int MinHeap_parent(int i) {
	return (i - 1) / 2;
}

int MinHeap_left(int i) {
	return 2 * i + 1;
}

int MinHeap_right(int i) {
	return 2 * i + 2;
}

int MinHeap_extractMin(MinHeap *h) {
	int root;
	if (h->size <= 0)
		return INT_MAX;
	else if (h->size == 1) {
		h->size--;
		return *(h->arr);
	} else {
		root = *(h->arr);
		*(h->arr) = *(h->arr + h->size - 1);
		h->size--;
		MinHeap_heapify(h, 0);
	}
	return root;
}

void MinHeap_decreaseKey(MinHeap *h, int i, int val) {
	*(h->arr + i) = val;
	while (i && *(h->arr + MinHeap_parent(i)) > *(h->arr + i)) {
		swap(h->arr + i, h->arr + MinHeap_parent(i));
		i = MinHeap_parent(i);
	}
}

int minHeap_findMin(MinHeap *h) {
	return *(h->arr);
}

void MinHeap_deleteKey(MinHeap *h, int i) {
	MinHeap_decreaseKey(h, i, INT_MIN);
	MinHeap_extractMin(h);
}

void MinHeap_insertKey(MinHeap *h, int k) {
	int i = h->size;
	if (h->size == h->capacity) {
		printf("Heap is full.\n");
		return;
	}
	h->size++;
	*(h->arr + i) = k;
	while (i && *(h->arr + MinHeap_parent(i)) > *(h->arr + i)) {
		swap(h->arr + i, h->arr + MinHeap_parent(i));
		i = MinHeap_parent(i);
	}
}

void MinHeap_removeAll(MinHeap *h) {
	while (h->size) {
		printf("%d ", MinHeap_extractMin(h));
	}
	printf("\n");
}

void MinHeap_free(MinHeap *h) {
	free(h->arr);
	free(h);
}

/************************************************************************
 *    This section contains code snippets for utility functions.
 ************************************************************************/
int getNextAvailNum(bool *taken, int i) {
	while (*(taken + i) == true) {
		printf("i=%d ", i);
		if (i+1 == MAX_RAND_SIZE) {
			i = 0;
		} else {
			i++;
		}
	}
	*(taken + i) = true;
	return i;
}

void fill10KNumbers(int *arr) {
	for(int i = 0; i < MAX_RAND_SIZE; ++i) {
		*(arr + i) = i;
	}
	// use Fisher Yates shuffling algorithm
	for (int j, i = MAX_RAND_SIZE - 1; i > 0; --i) {
		j = rand() % (i + 1);
		swap(arr + i, arr + j);
	}
}

void fill5KOperations(bool *insert) {
	for (int i = 0; i < NUM_OF_OPERATIONS; ++i) {
		*(insert + i) = rand() % 2 == 0 ? true : false;
	}
}

/************************************************************************
 *    This section contains code snippets for the main program.
 ************************************************************************/

// create a LeftistHeap of n unique elements
LeftistHeap *createLeftistHeap(int n, int *arr) {
	LeftistHeap *h = New_LeftistHeap();
	for (int i = n; i >= 0; --i) {
		LeftistHeap_insert(h, *(arr+i));
	}
	return h;
}

MinHeap *createMinHeap(int n, int *arr) {
	MinHeap *h = New_MinHeap(MAX_RAND_SIZE);
	for (int i = n; i >= 0; --i) {
		MinHeap_insertKey(h, *(arr+i));
	}
	return h;
}

void LeftistHeap_performOperations(LeftistHeap *h, int *arr, bool *insert) {
	for (int i = 0; i < NUM_OF_OPERATIONS; ++i) {
		if (*(insert + i)) {
			LeftistHeap_insert(h, *(arr + NUM_OF_OPERATIONS + i));
		} else {
			LeftistHeap_deleteMin(h);
		}
	}
}

void MinHeap_performOperations(MinHeap *h, int *arr, bool *insert) {
	for (int i = 0; i < NUM_OF_OPERATIONS; ++i) {
		if (*(insert + i)) {
			MinHeap_insertKey(h, *(arr+i));
		} else {
			MinHeap_extractMin(h);
		}
	}
}


void run(enum mode pq) {
    clock_t clock_start, clock_end;;
	bool *insert = malloc(NUM_OF_OPERATIONS * sizeof(bool));
	int *arr = malloc(MAX_RAND_SIZE * sizeof(int));
	fill10KNumbers(arr);
	fill5KOperations(insert);

	printf("-----------------------------------------------------\n");
	for (int i = 0; i < SIZE_OF_N; ++i) {
		int num = NUM_OF_ELEMENTS[i];

		if (pq == LEFTIST_HEAP) {
			LeftistHeap *h = createLeftistHeap(num, arr);
			clock_start = clock();
			LeftistHeap_performOperations(h, arr, insert);
			clock_end = clock();
			//LeftistHeap_removeAll(h);
			LeftistHeap_free(h);
			printf("Leftist heap starting with %d elements, average operation time %Lf/per operation\n",
					num, (long double)(clock_end - clock_start) / CLOCKS_PER_SEC);
		}

		else if (pq == MIN_HEAP) {
			MinHeap *t = createMinHeap(num, arr);
			clock_start = clock();
			MinHeap_performOperations(t, arr, insert);
			clock_end = clock();
			MinHeap_free(t);
			printf("Min heap starting with %d elements, average operation time %Lf/per operation\n",
					num, (long double)(clock_end - clock_start) / CLOCKS_PER_SEC);

		}
	} // for
	printf("-----------------------------------------------------\n");

	free(insert);
	free(arr);
}

void test() {
    int i = 0;
    clock_t clock_start = clock();
    long double time_elapsed = 0.0;
    int OPERATIONS[NUM_OF_OPERATIONS];
    for ( ;i < NUM_OF_OPERATIONS; ++i) {
        OPERATIONS[i] = rand() % 2;
        printf("%d ", NUM_OF_ELEMENTS[rand() % SIZE_OF_N]);
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
	LeftistHeap_free(h);
}

void test2() {
	LeftistHeap *h = New_LeftistHeap();
	LeftistHeap_insert(h, 1);
	LeftistHeap_deleteMin(h);
	LeftistHeap_deleteMin(h);
	LeftistHeap_deleteMin(h);
}

void test3() {
	int arr[] = {7,4,2,8,1,5,6,0,3,9};
	MinHeap *h = New_MinHeap(10);
	for (int i = 0; i < 10; ++i) {
		MinHeap_insertKey(h, arr[i]);
	}
	MinHeap_removeAll(h);
	MinHeap_free(h);
}


int main() {
	enum mode leftist = LEFTIST_HEAP;
	enum mode minheap = MIN_HEAP;
	run(leftist);
	run(minheap);
    return 0;
}
