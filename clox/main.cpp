#include "chunk.hpp"
#include "compiler.hpp"
#include "lox.hpp"
#include "virtual_machine.hpp"

#include <lak/string_literals.hpp>
#include <lak/string_ostream.hpp>

#include <iostream>

int main(int argc, char *argv[])
{
	if (argc > 3) return lox::usage();

	lak::optional<std::filesystem::path> file;

	while (argc-- > 1)
	{
		const auto arg{lak::astring_view::from_c_str(argv[argc])};
		if (arg == "--help"_view)
		{
			lox::usage();
			return EXIT_SUCCESS;
		}
		else
		{
			file = lak::astring(arg);
		}
	}

	lox::virtual_machine vm;

	auto result_parser{lak::overloaded{
	  [](lak::monostate) -> int { return EXIT_SUCCESS; },
	  [](lox::interpret_error err) -> int
	  {
		  std::cerr << lox::to_string(err) << "\n";
		  return EXIT_FAILURE;
	  },
	}};

	if (file)
	{
		return vm.run_file(*file).visit(result_parser);
	}
	else
	{
		return vm.run_prompt().visit(result_parser);
	}
}
