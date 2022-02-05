#include "callable.hpp"
#include "interpreter.hpp"
#include "lox.hpp"
#include "object.hpp"
#include "parser.hpp"
#include "printer.hpp"
#include "scanner.hpp"
#include "token.hpp"

#include <lak/string_literals.hpp>
#include <lak/string_ostream.hpp>

#include <iostream>

int main(int argc, char *argv[])
{
	if (argc > 3) return lox::usage();

	lak::optional<std::filesystem::path> file;
	bool print_dot = false;

	while (argc-- > 1)
	{
		const auto arg{lak::astring_view::from_c_str(argv[argc])};
		if (arg == "--help"_view)
		{
			lox::usage();
			return EXIT_SUCCESS;
		}
		else if (arg == "--dot"_view)
		{
			if (print_dot) return lox::usage();
			print_dot = true;
		}
		else
		{
			file = lak::astring(arg);
		}
	}

	lox::interpreter interpreter;

	if (print_dot)
	{
		if (!file)
		{
			std::cerr << "Missing file.\n";
			return lox::usage();
		}

		return interpreter.init_globals().parse_file(*file).visit(lak::overloaded{
		  [](lak::monostate) -> int
		  {
			  std::cerr << "Failed to parse file.\n";
			  return EXIT_FAILURE;
		  },
		  [](const std::vector<lox::stmt_ptr> &stmt) -> int
		  {
			  using lak::operator<<;
			  lak::u8string result;
			  for (const auto &s : stmt)
				  result += s->visit(lox::dot_subgraph_ast_printer);
			  std::cout << lox::dot_digraph_ast_printer.digraph_wrapper(
			                 lak::move(result))
			            << "\n";
			  return EXIT_SUCCESS;
		  }});
	}
	else if (file)
	{
		return interpreter.init_globals().run_file(*file).is_ok() ? EXIT_SUCCESS
		                                                          : EXIT_FAILURE;
	}
	else
	{
		return interpreter.init_globals().run_prompt().is_ok() ? EXIT_SUCCESS
		                                                       : EXIT_FAILURE;
	}
}
