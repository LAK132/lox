#include "callable.hpp"
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
	      .value = lox::object{123.0},
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

	std::vector<lox::expr_ptr> args;
	args.push_back(std::move(expr));
	expr = lox::expr::make_call({
	  .callee =
	    lox::expr::make_variable({.name =
	                                lox::token{
	                                  .type   = lox::token_type::IDENTIFIER,
	                                  .lexeme = u8"f",
	                                }}),
	  .paren =
	    lox::token{
	      .type   = lox::token_type::RIGHT_PAREN,
	      .lexeme = u8")",
	    },
	  .arguments = std::move(args),
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
