#ifndef LOX_PARSER_HPP
#define LOX_PARSER_HPP

#include "expr.hpp"
#include "interpreter.hpp"
#include "token.hpp"

#include <cstdint>
#include <initializer_list>
#include <vector>

namespace lox
{
	struct parser
	{
		lox::interpreter &interpreter;
		std::vector<lox::token> tokens;
		size_t current = 0;

		bool empty() const;

		const lox::token &last();

		const lox::token &next();

		const lox::token &peek() const;

		bool check(lox::token_type type) const;

		bool match(std::initializer_list<lox::token_type> types);

		const lox::token *consume(lox::token_type type,
		                          std::u8string_view message_on_err);

		void sync();

		lox::expr_ptr parse_primary();

		lox::expr_ptr parse_unary();

		lox::expr_ptr parse_factor();

		lox::expr_ptr parse_term();

		lox::expr_ptr parse_comparison();

		lox::expr_ptr parse_equality();

		lox::expr_ptr parse_expression();

		lox::expr_ptr parse();
	};
}

#endif
