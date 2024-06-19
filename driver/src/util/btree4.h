/******************************************************************************
* Copyright (C) 2024 AMD, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef _BTREE4_H_
#define _BTREE4_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>

/*    [data0, data1, data2]
 *    /      |      |     \
 *   c0     c1     c2     c3
 *
 * c0-c3: child nodes child[0] to child[3].
 */

#define BTREE4_DATA_NUM		3
#define BTREE4_CHILD_NUM	(BTREE4_DATA_NUM + 1)

struct btree4_node {
	void *data[BTREE4_DATA_NUM];
	struct btree4_node *child[BTREE4_CHILD_NUM];
	struct btree4_node *parent;
};

struct btree4 {
	struct btree4_node *root;
	int (*cmp)(void *a, void *b);
};

#define BTREE4_DECLARE(name, cmp)	struct btree4 name = {.root = NULL, .cmp = cmp}
#define BTREE4_INIT(ptr, compare)		{(ptr)->root = NULL; (ptr)->cmp = compare;}

void btree4_inorder_visit(struct btree4 *tree,
			  void (*visit)(void *data, void *priv),
			  void *priv);

int btree4_insert(struct btree4 *tree, void *data);

void *btree4_search(struct btree4 *tree, void *data);
void *btree4_delete(struct btree4 *tree, void *data);

#endif /* _BTREE4_H_ */
