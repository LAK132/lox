#include "interpreter.hpp"
#include "lox.hpp"
#include "object.hpp"
#include "overloaded.hpp"
#include "scanner.hpp"
#include "string_hacks.hpp"
#include "token.hpp"

#include <iostream>

int main(int argc, char *argv[])
{
	lox::interpreter interpreter;

	if (argc > 2)
		return lox::usage();
	else if (argc == 2)
		return interpreter.run_file(std::filesystem::path(argv[1]));
	else
		return interpreter.run_prompt();
}
