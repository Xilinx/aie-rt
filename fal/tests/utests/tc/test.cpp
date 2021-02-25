#include "xaiefal/xaiefal.hpp"

#include "CppUTest/CommandLineTestRunner.h"

using namespace xaiefal;

int main(int ac, char** av)
{

	Logger::log(LogLevel::INFO) << "xaiefal cpputests." << std::endl;
	return CommandLineTestRunner::RunAllTests(ac, av);
}
