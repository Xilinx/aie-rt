/******************************************************************************
* Copyright (C) 2024 AMD, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "btree4.h"
#include "xaie_feature_config.h"
#include "xaie_mem_alloc.h"

static int32_t btree4_data_count(const struct btree4_node *node)
{
	int32_t i;

	for (i = 0; i < BTREE4_DATA_NUM; i++) {
		if ((node->data[i] == NULL)) {
			return i;
		}
	}
	return i;
}

static bool btree4_is_leaf_node(const struct btree4_node *node)
{
	int32_t i;

	for (i = 0; i < BTREE4_CHILD_NUM; i++) {
		if (node->child[i] != NULL) {
			return false;
		}
	}
	return true;
}

static struct btree4_node *btree4_find_node_pos(const struct btree4 *tree, void *data)
{
	struct btree4_node *node = tree->root;
	struct btree4_node *parent = node;
	int32_t i;

again:
	if ((node == NULL)) {
		return parent;
	}
	parent = node;
	for (i = 0; i < BTREE4_DATA_NUM; i++) {
		if ((node->data[i] == NULL)) {
			break;
		}
		int32_t res = tree->cmp(data, node->data[i]);

		if (res == 0) {
			return NULL;
		}
		if (res < 0) {
			node = node->child[i];
			goto again;
		}
	}
	node = node->child[i];
	goto again;
}

static int32_t btree4_insert_data_child(const struct btree4 *tree, struct btree4_node *node,
				   void *data, struct btree4_node *child)
{
	int32_t res;
	int32_t pos;

	if ((node == NULL) || (data == NULL)) {
		return 0;
	}
	pos = btree4_data_count(node);

	if (pos == 3) {
#ifdef XAIE_FEATURE_STDIO_LOGGING_ENABLE
		(void)printf("ERROR: no space. Please slit the node\n");
#endif
		return -EINVAL;
	}

	while (pos != 0) {
		res = tree->cmp(data, node->data[pos - 1]);
		if (res > 0) {
			goto out;
		}
		node->data[pos] = node->data[pos - 1];
		node->child[pos + 1] = node->child[pos];
		pos--;
	}
out:
	node->data[pos] = data;
	node->child[pos + 1] = child;
	if (child != NULL) {
		child->parent = node;
	}
	return 0;
}

static int32_t btree4_split(struct btree4 *tree, struct btree4_node *node)
{
	int32_t count;
	struct btree4_node *new_node;
	struct btree4_node *parent;
	void *new_data;

	if ((node == NULL)) {
		return 0;
	}
	count = btree4_data_count(node);

	if (count != BTREE4_DATA_NUM) {
		return 0;
	}
	(void)btree4_split(tree, node->parent);

	new_node = (struct btree4_node *)XAIE_CALLOC(1, sizeof(*new_node));
	if ((new_node == NULL)) {
		return -ENOMEM;
	}

	new_node->data[0] = node->data[2];
	node->data[2] = NULL;

	new_node->child[0] = node->child[2];
	node->child[2] = NULL;
	if (new_node->child[0] != NULL) {
		new_node->child[0]->parent = new_node;
	}

	new_node->child[1] = node->child[3];
	node->child[3] = NULL;
	if (new_node->child[1] != NULL) {
		new_node->child[1]->parent = new_node;
	}

	new_data = node->data[1];
	node->data[1] = NULL;

	if ((node->parent == NULL)) {
		parent = (struct btree4_node *)XAIE_CALLOC(1, sizeof(*parent));
		if ((parent == NULL)) {
			return -ENOMEM;
		}
		parent->data[0] = new_data;

		parent->child[0] = node;
		node->parent = parent;

		parent->child[1] = new_node;
		new_node->parent = parent;

		tree->root = parent;
	} else {
		(void)btree4_insert_data_child(tree, node->parent, new_data, new_node);
	}

	return 0;
}

static void _btree4_inorder_visit(struct btree4 *tree, const struct btree4_node *node,
				  void (*visit)(void *data, void *priv),
				  void *priv)
{
	int32_t i;

	if ((node == NULL)) {
		return;
	}
	for (i = 0; i < BTREE4_DATA_NUM; i++) {
		if ((node->data[i] == NULL)) {
			break;
		}
		_btree4_inorder_visit(tree, node->child[i], visit, priv);
		visit(node->data[i], priv);
	}
	_btree4_inorder_visit(tree, node->child[i], visit, priv);
}


void btree4_inorder_visit(struct btree4 *tree,
			  void (*visit)(void *data, void *priv),
			  void *priv)
{
	if ((visit == NULL)) {
		return;
	}
	_btree4_inorder_visit(tree, tree->root, visit, priv);
}

int32_t btree4_insert(struct btree4 *tree, void *data)
{
	struct btree4_node *node = tree->root;

	if ((node == NULL)) {
		tree->root = (struct btree4_node *)XAIE_CALLOC(1, sizeof(*tree->root));
		if ((tree->root == NULL)) {
			return -ENOMEM;
		}
		node = tree->root;
		node->data[0] = data;
		return 0;
	}
again:
	node = btree4_find_node_pos(tree, data);
	if ((node == NULL)) {
		return -EEXIST;
	}
	if (btree4_data_count(node) == BTREE4_DATA_NUM) {
		(void)btree4_split(tree, node);
		goto again;
	}
	return btree4_insert_data_child(tree, node, data, NULL);
}

static struct btree4_node *btree4_search_node(const struct btree4 *tree, void *data,
					     int32_t *index)
{
	struct btree4_node *node;
	int32_t res;
	int32_t i;

	node = tree->root;

again:
	if ((node == NULL)) {
		return NULL;
	}

	for (i = 0; i < BTREE4_DATA_NUM; i++) {
		if ((node->data[i] == NULL)) {
			break;
		}
		res = tree->cmp(data, node->data[i]);
		if (res < 0) {
			node = node->child[i];
			goto again;
		}
		if (res == 0) {
			*index = i;
			return node;
		}
	}
	node = node->child[i];
	goto again;
}

void *btree4_search(const struct btree4 *tree, void *data)
{
	const struct btree4_node *node;
	int32_t res;
	int32_t i;

	node = tree->root;

again:
	if ((node == NULL)) {
		return NULL;
	}

	for (i = 0; i < BTREE4_DATA_NUM; i++) {
		if ((node->data[i] == NULL)) {
			break;
		}
		res = tree->cmp(data, node->data[i]);
		if (res < 0) {
			node = node->child[i];
			goto again;
		}
		if (res == 0) {
			return node->data[i];
		}
	}
	node = node->child[i];
	goto again;

}

static void btree4_merge_root(struct btree4 *tree, struct btree4_node *node,
			     struct btree4_node *child)
{
	int32_t node_data_count;
	struct btree4_node *other;
	int32_t i;

	node_data_count = btree4_data_count(node);
	if (node_data_count > 1) {
		return;
	} else if (__builtin_expect((node_data_count == 0), 0)) {
		/* node cannot be emmpty and root here. This condition will never happen */
		return;
	} else {
		/* No action required */
	}
	/*	 [node]
	 * 	/      \
	 * child       other
	 */
	if (node->child[0] == child) {
		/*	[n]                        [c, n, o]
		 *     /   \                      /  /  \  \
		 *    [c]  [o]        ->
		 *   /  \  /  \
		 */
		other = node->child[1];
		if (btree4_data_count(other) == 1) {
			child->data[1] = node->data[0];
			child->child[2] = other->child[0];
			child->child[3] = other->child[1];
			child->data[2] = other->data[0];
			tree->root = child;
			child->parent = NULL;
			for (i = 0; i < BTREE4_CHILD_NUM; i++) {
				if (child->child[i] != NULL) {
					child->child[i]->parent = child;
				}
			}
			XAIE_FREE(other);
			other = NULL;
			XAIE_FREE(node);
			node = NULL;
		} else {
			/*	[n]                       [o1]
			 *     /   \             ->      /     \
			 *    [c]   [o1, o2, o3]       [c, n]   [o2, o3]
			 *   /  \   /  /   \   \       /  /  \  /  /   \
			 */
			child->data[1] = node->data[0];
			node->data[0] = other->data[0];
			child->child[2] = other->child[0];
			if (child->child[2] != NULL) {
				child->child[2]->parent = child;
			}
			for (i = 0; i < (BTREE4_DATA_NUM - 1); i++) {
				other->data[i] = other->data[i + 1];
				other->child[i] = other->child[i + 1];
			}
			other->data[i] = NULL;
			other->child[i] = other->child[i + 1];
			other->child[i + 1] = NULL;
		}
	} else {
		/*	 [node]
		 * 	/      \
		 * other       child
		 */
		other = node->child[0];
		if (btree4_data_count(other) == 1) {
			/*	[n]                   [o, n, c]
			 *     /   \         ->       /  /  \  \
			 *    [o]   [c]
			 *   /  \   /  \
			 */
			child->data[2] = child->data[0];
			child->data[1] = node->data[0];
			child->data[0] = other->data[0];
			tree->root = child;
			child->parent = NULL;
			child->child[3] = child->child[1];
			child->child[2] = child->child[0];
			child->child[1] = other->child[1];
			child->child[0] = other->child[0];

			for (i = 0; i < BTREE4_CHILD_NUM; i++) {
				if (child->child[i] != NULL) {
					child->child[i]->parent = child;
				}
			}
			XAIE_FREE(other);
			other = NULL;
			XAIE_FREE(node);
			node = NULL;
		} else {
			/*	       [n]                    [o3]
			 *	      /   \                  /    \
			 * [o1, o2, o3]    [c]    ->   [o1, o2]   [n, c]
			 * /   /   \   \   /  \        /  /   \   / /  \
			 */
			i = (other->data[2] != NULL) ? 2 : 1;
			child->data[1] = child->data[0];
			child->data[0] = node->data[0];
			child->child[2] = child->child[1];
			child->child[1] = child->child[0];
			child->child[0] = other->child[i + 1];
			if (child->child[0] != NULL) {
				child->child[0]->parent = child;
			}
			other->child[i + 1] = NULL;
			node->data[0] = other->data[i];
			other->data[i] = NULL;
			other->child[i + 1] = NULL;
		}
	}
	if ((child->data[0] == NULL)) {
		for (i = 0; i < (BTREE4_DATA_NUM - 1); i++) {
			child->data[i] = child->data[i + 1];
		}
		child->data[i] = NULL;
	}
}

static void btree4_merge(struct btree4 *tree, struct btree4_node *node,
			struct btree4_node *child)
{
	struct btree4_node *parent;
	int32_t parent_count;
	struct btree4_node *other;
	int32_t i;

	if ((node == NULL)) {
		return;
	}

	if (btree4_data_count(node) > 1) {
		return;
	}
	/* root node */
	if ((node->parent == NULL)) {
		btree4_merge_root(tree, node, child);
		return;
	} else {
		btree4_merge(tree, node->parent, node);
	}
	if (btree4_data_count(node) > 1) {
		return;
	}

	parent = node->parent;
	parent_count = btree4_data_count(parent);
	/*		[parent, a]
	 *	       /   \
	 *	  [node]    [other]
	 */
	for (i = 0; i < BTREE4_CHILD_NUM; i++) {
		if (parent->child[i] == node) {
			break;
		}
	}
	/* This condition should never occur. */
	if (__builtin_expect((i >= BTREE4_CHILD_NUM), 0)) {
		return;
	}
	if (parent_count == i) {
		int32_t ci;

		/* This should never happe. parent data count and child index cannot be 0 */
		if (__builtin_expect(i == 0, 0)) {
			return;
		}
		/*	[parent, a,        b]
		 *     /      /    \         \
		 *               [other]     [node]
		 */
		other = parent->child[i - 1];
		ci = btree4_data_count(other);
		if (ci == 1) {
			/* [p0, p1, p2]          [p0, p1]
			 * /  /   \   \      ->  /  /   \
			 *       [o]  [n]               [o, p2, n]
			 */
			node->data[2] = node->data[0];
			node->data[1] = parent->data[i - 1];
			node->data[0] = other->data[0];
			parent->data[i - 1] = NULL;
			node->child[3] = node->child[1];
			node->child[2] = node->child[0];
			node->child[1] = other->child[1];
			if (node->child[1] != NULL) {
				node->child[1]->parent = node;
			}
			node->child[0] = other->child[0];
			if (node->child[0] != NULL) {
				node->child[0]->parent = node;
			}
			XAIE_FREE(other);
			parent->child[i - 1] = node;
			parent->child[i] = NULL;
		/* ci, which is data count should never be 0 */
		} else if (__builtin_expect(ci != 0, 1)) {
			/*  [p0, p1,             p2]      [p0, p1,     o2]
			 * /   /   \                \  ->/   /   \       \
			 *        [o0, o1, o2]      [n]         [o0, o1] [p2, n]
			 */
			node->data[1] = node->data[0];
			node->data[0] = parent->data[i - 1];
			parent->data[i - 1] = other->data[ci - 1];
			other->data[ci - 1] = NULL;

			node->child[2] = node->child[1];
			node->child[1] = node->child[0];
			node->child[0] = other->child[ci];
			other->child[ci] = NULL;
			if (node->child[0] != NULL) {
				node->child[0]->parent = node;
			}
		}
	} else if (__builtin_expect((i < (BTREE4_CHILD_NUM - 1)), 1)) {
		/*		[parent,     a,    c]
		 *	       /       /      \     \
		 *	  [node]     [other]
		 */
		other = parent->child[i + 1];
		if (btree4_data_count(other) == 1) {
			/*         [p0, p1, p2]            [p0, p1]
			 *        /  /    \    \    ->     /   \   \
			 *                [n]  [o]                  [n, p2, o]
			 */
			node->data[1] = parent->data[i];
			node->data[2] = other->data[0];
			node->child[2] = other->child[0];
			if (node->child[2] != NULL) {
				node->child[2]->parent = node;
			}
			node->child[3] = other->child[1];
			if (node->child[3] != NULL) {
				node->child[3]->parent = node;
			}
			XAIE_FREE(other);
			other = NULL;
			for (; i < (BTREE4_DATA_NUM - 1); i++) {
				parent->data[i] = parent->data[i + 1];
				parent->child[i + 1] = parent->child[i + 2];
			}
			parent->data[i] = NULL;
			parent->child[i + 1] = NULL;
		} else {
			/*   [p0, p1, p2]                [p0, p1,     o0]
			 *   /   /   \   \        ->     /   /   \      \
			 *           [n]  [o0, o1, o2]          [n, p2] [o1, o2]
			 */
			node->data[1] = parent->data[i];
			parent->data[i] = other->data[0];
			node->child[2] = other->child[0];
			if (node->child[2] != NULL) {
				node->child[2]->parent = node;
			}
			for (i = 0; i < (BTREE4_DATA_NUM - 1); i++) {
				other->data[i] = other->data[i + 1];
				other->child[i] = other->child[i + 1];
			}
			other->data[i] = NULL;
			other->child[i] = other->child[i + 1];
			other->child[i + 1] = NULL;
		}
	} else {
		/* No action required */
	}
}

static struct btree4_node *btree4_largest(struct btree4_node *node, int32_t *index)
{
	int32_t i;

	if ((node == NULL)) {
		*index = 0;
		return NULL;
	}
again:
	for (i = (BTREE4_DATA_NUM - 1); i >= 0; i--) {
		if (node->data[i] != NULL) {
			break;
		}
	}
	if (i < 0) {
		return NULL;
	}
	if ((node->child[i] == NULL)) {
		*index = i;
		return node;
	}
	node = node->child[i + 1];
	goto again;
}

void *btree4_delete(struct btree4 *tree, void *data)
{
	struct btree4_node *node;
	int32_t parent_count;
	int32_t index;
	int32_t i;
	void *value;
	void *insert_value;
	struct btree4_node *merge_node;
	int32_t merge_index;
	struct btree4_node *parent;


	node = btree4_search_node(tree, data, &index);

	if ((node == NULL)) {
		return NULL;
	}
	value = node->data[index];

	/* Node is leaf node */
	if (btree4_is_leaf_node(node)) {
		for (i = index; i < (BTREE4_DATA_NUM - 1); i++) {
			node->data[i] = node->data[i+1];
		}
		node->data[i] = NULL;
		/* Node is not empty */
		if (btree4_data_count(node) != 0) {
			return value;
		}
		/* Node is empty */
		/* Node is root node and empty */
		if (node->parent == NULL) {
			tree->root = NULL;
			XAIE_FREE(node);
			return value;
		}
		/* Node is empty and not root node */
		merge_node = node;
	} else {
		/* Non leaf node
		 * merge_node here is alway leaf node
		 */
		merge_node = btree4_largest(node->child[index], &merge_index);
		node->data[index] = merge_node->data[merge_index];
		merge_node->data[merge_index] = NULL;
		/* merge_node has at least one data */
		if (merge_index != 0) {
			return value;
		}
	}

	/* merge_node is empty */
	/* make sure parent atleast has 2 data */
	btree4_merge(tree, merge_node->parent, merge_node);
	if (btree4_data_count(merge_node) != 0) {
		return value;
	}
	parent = merge_node->parent;
	if ((parent == NULL)) {
		return value;
	}

	parent_count = btree4_data_count(parent);
	for (i = 0; i < (BTREE4_CHILD_NUM - 1); i++) {
		if (parent->child[i] == merge_node) {
			break;
		}
	}
	/* i = 3 parent_count = 3 */
	if (parent_count == i) {
		if (__builtin_expect(i == 0, 0)) {
			/* This condition will never occur */
			return NULL;
		}
		/*    [p0, p1, p2]
		 *   /   /   \   \
		 *               []
		 */
		insert_value = parent->data[i - 1];
		parent->data[i - 1] = NULL;
		parent->child[i] = NULL;
	} else {
		/*    [p0, p1, p2]
		 *   /   /   \   \
		 *      []
		 */

		/* i will never be >= BTREE4_DATA_NUM.
		 * Add unnecessary check to supress the error.
		 */
		if (__builtin_expect((i >= BTREE4_DATA_NUM), 0)) {
			return value;
		}
		insert_value = parent->data[i];
		for (; i < (BTREE4_DATA_NUM - 1); i++) {
			parent->data[i] = parent->data[i + 1];
			parent->child[i] = parent->child[i + 1];
		}
		parent->data[i] = NULL;
		parent->child[i] = parent->child[i + 1];
		parent->child[i + 1] = NULL;
	}
	(void)btree4_insert(tree, insert_value);
	XAIE_FREE(merge_node);

	return value;
}
