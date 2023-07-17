#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTest/TestHarness.h"
#include <hw_config.h>

int main(int argc, char**argv)
{
	return CommandLineTestRunner::RunAllTests(argc, argv);
}
