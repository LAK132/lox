#include "lox.hpp"

#include <iostream>

int lox::usage()
{
	std::cerr << "Usage: jlox [script] [--dot]\n";
	return EXIT_FAILURE;
}
