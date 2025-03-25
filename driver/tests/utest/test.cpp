/******************************************************************************
* Copyright (C) 2023-2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTest/TestHarness.h"
#include <hw_config.h>

int main(int argc, char**argv)
{
	return CommandLineTestRunner::RunAllTests(argc, argv);
}
