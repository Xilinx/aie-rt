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
#include <stdint.h>

/*
 * The Arm Certified FuSa C library's <errno.h> only defines the ISO C
 * minimum (EDOM, ERANGE) -- no EINVAL/ENOMEM/EEXIST. These are used below
 * purely as distinct negative-return status codes (never read back via the
 * actual errno object), so a local fallback with the conventional
 * POSIX/glibc values is sufficient and does not conflict with a full
 * hosted <errno.h> that already defines them.
 */
#ifndef EINVAL
#define EINVAL 22
#endif
#ifndef ENOMEM
#define ENOMEM 12
#endif
#ifndef EEXIST
#define EEXIST 17
#endif

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
	int32_t (*cmp)(void *a, void *b);
};

#define BTREE4_DECLARE(name, cmp)	struct btree4 name = {.root = NULL, .cmp = cmp}
#define BTREE4_INIT(ptr, compare)		{(ptr)->root = NULL; (ptr)->cmp = compare;}

void btree4_inorder_visit(struct btree4 *tree,
			  void (*visit)(void *data, void *priv),
			  void *priv);

int32_t btree4_insert(struct btree4 *tree, void *data);

void *btree4_search(const struct btree4 *tree, void *data);
void *btree4_delete(struct btree4 *tree, void *data);

#endif /* _BTREE4_H_ */
