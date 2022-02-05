#include "lox.hpp"

#include <iostream>

int lox::usage()
{
	std::cerr << "Usage: clox [script]\n";
	return EXIT_FAILURE;
}
