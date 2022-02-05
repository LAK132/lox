#include "compiler.hpp"
#include "scanner.hpp"

#include <lak/debug.hpp>
#include <lak/string_literals.hpp>

lox::compile_result<> lox::compile(lak::u8string_view file)
{
	lox::scanner scanner{file};

	size_t line{0U};

	for (;;)
	{
		using lak::operator<<;
		RES_TRY_ASSIGN(lox::token token =,
		               scanner.scan_token().map_err(
		                 [](lox::scan_error &&err) -> lox::compile_error
		                 { return {lak::move(err)}; }));

		if (line != token.line)
		{
			std::cout << std::setfill('0') << std::setw(4) << token.line << " ";
			line = token.line;
		}
		else
			std::cout << "   | ";

		std::cout << std::setfill('0') << std::setw(4) << unsigned(token.type)
		          << " '" << token.lexeme << "'\n";

		if (token.type == lox::token_type::EOF_TOK) break;
	}

	return lak::ok_t{};
}
