
/* @file trie.h
 *
 * @brief Fast string lookups
 *
 * A trie is a data structure for dictionary search with key-value.
 *
 * To create a trie, use @ref NuTrieNew . 
 *
 * To destroy a trie, use @ref NuTrieFree .
 *
 * To insert a value to trie, use @ref NuTrieItem_Add .
 *
 * To find a value from its key, use @ref NuTrieItem_Find and call @ref NuTrieItem_Get_Value to get the value.
 * 
 * To remove a value, use @ref NuTrieItem_Remove .
 *
 * To foreach all trie item, use @ref NuTrieItem_ForEach .
 * 
 * To Clear all trie item, use @ref NuTrieItem_Clear .
 * */
#include "NuCommon.h"
#include "NuUtil.h"
#include "NuBuffer.h"

#ifndef _NUTRIE_H
#define _NUTRIE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A item in a @ref NuTrie_t .
 */
typedef struct _NuTrieItem_t NuTrieItem_t;

/**
 * A trie structure.
 */
typedef struct _NuTrie_t NuTrie_t;

/**
 * Create a new trie.
 *
 * @param trie               The address for pointer of the trie.
 * @param buffer_cnt         The item buffer count for pre-allocate.
 * @return                   Pointer to a new trie structure, or NULL if it
 *                           was not possible to allocate memory.
 */
int NuTrieNew(NuTrie_t **trie, int buffer_cnt);

/**
 * Destroy a trie.
 *
 * @param trie               The trie to destroy.
 */
void NuTrieFree(NuTrie_t *trie);

/**
 * Find a trie item.
 *
 * @param trie               The trie 
 * @param key                The key to access the value.
 * @param key_len            The length of key.
 * @return                   Pointer to a trie item
 */
NuTrieItem_t *NuTrieItem_Find(NuTrie_t *trie, const char *key, unsigned int key_len);
/**
 * Add a value to trie.
 *
 * @param trie               The trie 
 * @param key                The key to access the value.
 * @param key_len            The length of key.
 * @param value              Pointer of the stroe value.
 * @return                   Pointer to a trie item
 */
NuTrieItem_t *NuTrieItem_Add(NuTrie_t *trie, const char *key, unsigned int key_len, void *value);

/**
 * Get the trie root item.
 *
 * @param trie               The trie 
 * @return                   Pointer to a trie root item
 */
NuTrieItem_t *NuTrieItem_Get_Root(NuTrie_t *trie);

/**
 * Get the trie root item.
 *
 * @param trie               The trie 
 */
void NuTrieItem_Clear(NuTrie_t *trie);

/**
 * Replace the item's value.
 *
 * @param item               The item which find from @ref NuTrieItem_Find.
 */
void NuTrieItem_Replace(NuTrieItem_t *item, void *value);

/**
 * Remove the item in trie.
 *
 * @param item               The item which find from @ref NuTrieItem_Find.
 */
void NuTrieItem_Remove(NuTrieItem_t *item);

/**
 * Get the item's valuse. 
 *
 * @param item               The item which find from @ref NuTrieItem_Find.
 * @return                   Pointer to a value from trie item.
 */
void *NuTrieItem_Get_Value(NuTrieItem_t *item);

/**
 * Get the value counts under the item.
 *
 * @param item               The item which find from @ref NuTrieItem_Find.
 * @return                   The count of values under this item.
 */
int  NuTrieItem_Get_Cnt(NuTrieItem_t *item);

/**
 * Get the item's valuse. 
 *
 * @param item               The item which find from @ref NuTrieItem_Find.
 * @param ForEachFn          Callback function for process all sub-items.
 * @param argu               Free argument, it pass to callback function.
 * @return                   The count of values under this item.
 */
void NuTrieItem_ForEach(NuTrieItem_t *item, void (*ForEachFn)(NuTrieItem_t *item, void *argu), void *argu);

#ifdef __cplusplus
}
#endif

#endif /* _NUTRIE_H */

