#include <stdio.h>
#include <stdlib.h>
#include "btree.h"

/**
 * Initializes the binary tree data structure.
 *
 * You may assume that:
 * - This is the first function called for each instance of btree_t.
 * - The btree_t pointer is a valid, non-NULL pointer.
 *
 * @param b
 *   A pointer to the binary tree data structure.
 */
void btree_init(btree_t *b)
{

}


/**
 * Adds a value into the binary tree.
 *
 * You may assume that:
 * - The btree_t pointer points to an initalized btree_t data structure.
 *
 * @param b
 *   A pointer to the binary tree data structure.
 * @param value
 *   The value to be added to the binary tree.
 */
void btree_add(btree_t *b, int value)
{

}


/**
 * Helper function.  Recursively prints the binary tree in-order.
 *
 * Remember, in-order means you should:
 * - move to the left child,
 * - print your own value, and then
 * - move to the right child
 * 
 * @param entry
 *   The current binary tree entry.  May be NULL.
 */
static void btree_print_helper(btree_entry_t *entry)
{

}


/**
 * Prints the binary tree in-order.
 *
 * (Note: This function is complete and does not need additional code.)
 *
 * @param b
 *   A pointer to the binary tree data structure.
 */
void btree_print(btree_t *b)
{
	btree_print_helper(b->root);
}


