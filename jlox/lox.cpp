#include "lox.hpp"

#include <iostream>

int lox::usage()
{
	std::cerr << "Usage: lox [script] [--dot]\n";
	return EXIT_FAILURE;
}
