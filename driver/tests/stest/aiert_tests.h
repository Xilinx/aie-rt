/******************************************************************************
* Copyright (C) 2023 AMD, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file aiert_tests.h
* @{
*
* Header file for declaring AIE-RT simulator tests (AIEML-Tests)
*
******************************************************************************/

/* List of simulator test APIs
 * Description: Add an external reference to your test-api/s here
 *
 */
extern int test_lock(XAie_DevInst *DevInst);
extern int test_aie_auto_routing(XAie_DevInst *DevInst);
/*
 * Description: Add function pointers to the above test-api/s here
 */
int (*tests_aiert[])(XAie_DevInst *DevInst) =
{
	test_lock
};

/*
 * Description: Add the name of the test-api/s
 */
const char *test_names_aiert[] =
{
	"test_lock",
	"test_aie_auto_routing"
};
/** @} */
