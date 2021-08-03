#include "interpreter.hpp"
#include "lox.hpp"
#include "object.hpp"
#include "overloaded.hpp"
#include "printer.hpp"
#include "scanner.hpp"
#include "string_hacks.hpp"
#include "token.hpp"

#include <iostream>

int main(int argc, char *argv[])
{
	auto expr = lox::expr::make_binary({
	  .left = lox::expr::make_unary({
	    .op =
	      lox::token{
	        .type   = lox::token_type::MINUS,
	        .lexeme = u8"-",
	      },
	    .right = lox::expr::make_literal({
	      .value = lox::object{.value = 123.0},
	    }),
	  }),
	  .op =
	    lox::token{
	      .type   = lox::token_type::STAR,
	      .lexeme = u8"*",
	    },
	  .right = lox::expr::make_grouping({
	    .expression = lox::expr::make_literal({
	      .value = 45.67,
	    }),
	  }),
	});

	lox::interpreter interpreter;

	if (argc > 2)
		return lox::usage();
	else if (argc == 2)
	{
		if (std::string("--prefix") == argv[1])
			std::cout << lox::as_astring_view(expr->visit(lox::prefix_ast_printer))
			          << "\n";
		else if (std::string("--postfix") == argv[1])
			std::cout << lox::as_astring_view(expr->visit(lox::postfix_ast_printer))
			          << "\n";
		else
			return interpreter.run_file(std::filesystem::path(argv[1]));
	}
	else
		return interpreter.run_prompt();
}
