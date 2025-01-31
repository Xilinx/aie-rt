/******************************************************************************
* Copyright (C) 2024 AMD, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#ifndef FILES_MISSING

#include <stdio.h>
#include <sys/random.h>
#include "CppUTest/TestHarness.h"

extern "C" {
#include "xaiengine/btree4.h"
}

#define TEST_BTREE4_SIZE	20000

static uint64_t tmp;

int cmp(void *a, void *b)
{
	uint64_t x = (uint64_t)a;
	uint64_t y = (uint64_t)b;

	if (x < y)
		return -1;
	else if (x > y)
		return 1;
	return 0;
}

void visit(void *data, void *priv)
{
	uint64_t v = (uint64_t)data;
	int *order_result = (int *)priv;

	if (*order_result)
		return;
	if (tmp > v) {
		*order_result = 1;
		return;
	}
	tmp = v;
}

int check_order(struct btree4 *tree)
{
	int order_result = 0;
	tmp = 0;
	btree4_inorder_visit(tree, visit, (void *)&order_result);

	return order_result;
}

TEST_GROUP(BTree4)
{
	void setup()
	{
	}
	void teardown()
	{
	}
};

TEST(BTree4, BTree4_test)
{
	uint64_t *data;
	uint64_t *random_index;
	ssize_t size = TEST_BTREE4_SIZE;
	BTREE4_DECLARE(tree, cmp);
	uint64_t i;
	uint64_t res;

	data = (uint64_t *)malloc(size * sizeof(*data));
	CHECK(data != NULL);
	random_index = (uint64_t *)malloc(size * sizeof(*random_index));
	CHECK(random_index != NULL);

	getrandom(data, size * sizeof(*data), GRND_NONBLOCK);
	getrandom(random_index, size * sizeof(*random_index), GRND_NONBLOCK);

	/* Insert */
	for (i = 0; i < size; i++) {
		if (data[i] == 0)
			continue;

		btree4_insert(&tree, (void *)data[i]);

		res = (uint64_t)btree4_search(&tree, (void *)data[i]);
		CHECK_EQUAL(res, data[i]);

		res = check_order(&tree);
		CHECK_EQUAL(res, 0);
	}

	/* Search */
	for (i = 0; i < size; i++) {
		if (data[i] == 0)
			continue;
		res = (uint64_t)btree4_search(&tree, (void *)data[i]);
		CHECK_EQUAL(res, data[i]);
	}

	/* Random Delete */
	for (i = 0; i < size; i++) {
		uint64_t index = random_index[i];

		index %= size;
		if (data[index] == 0)
			continue;

		res = (uint64_t)btree4_delete(&tree, (void *)data[index]);
		CHECK_EQUAL(res, data[index]);

		res = check_order(&tree);
		CHECK_EQUAL(res, 0);

		res = (uint64_t)btree4_search(&tree, (void *)res);
		CHECK_EQUAL(res, 0);

		for (uint64_t j = 0; j < size; j++) {
			if (data[index] == data[j])
				data[j] = 0;
		}
	}
}
#endif  /*#ifndef FILES_MISSING*/
