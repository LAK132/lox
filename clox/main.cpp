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

	using lak::operator<<;

	if (file)
	{
		return vm.run_file(*file).visit(lak::overloaded{
		  [](lak::monostate) -> int { return EXIT_SUCCESS; },
		  [](const lox::virtual_machine::run_file_error &err) -> int
		  {
			  return err.visit(lak::overloaded{
			    [](const lak::errno_error &err) -> int
			    {
				    std::cerr << err << "\n";
				    return EXIT_FAILURE;
			    },
			    []<typename T>(const lox::positional_error<T> &err) -> int
			    {
				    std::cerr << lox::to_string(err) << "\n";
				    return EXIT_FAILURE;
			    },
			  });
		  },
		});
	}
	else
	{
		return vm.run_prompt().visit(lak::overloaded{
		  [](lak::monostate) -> int { return EXIT_SUCCESS; },
		  [](const lox::interpret_error &err) -> int
		  {
			  err.visit([]<typename T>(const lox::positional_error<T> &err)
			            { std::cerr << lox::to_string(err) << "\n"; });
			  return EXIT_FAILURE;
		  },
		});
	}
}
