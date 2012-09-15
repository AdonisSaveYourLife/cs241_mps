#include <stdio.h>
#include "btree.h"

int main()
{
	btree_t tree;
	btree_init(&tree);

	btree_add(&tree, 64);
	btree_add(&tree, 32);
	btree_add(&tree, 123);
	btree_add(&tree, 48);
	btree_add(&tree, -4);
	btree_add(&tree, 42);
	btree_add(&tree, 32);
	btree_add(&tree, 15);
	btree_add(&tree, 64);
	btree_add(&tree, 48);
	btree_add(&tree, -13);
	btree_add(&tree, 394);
	btree_add(&tree, 17);

	btree_print(&tree);

	return 0;
}
