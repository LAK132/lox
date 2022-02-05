#ifndef LOX_PARSER_HPP
#define LOX_PARSER_HPP

#include "expr.hpp"
#include "interpreter.hpp"
#include "stmt.hpp"
#include "token.hpp"

#include <lak/result.hpp>
#include <lak/stdint.hpp>

#include <initializer_list>
#include <source_location>
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

		const lox::token *consume(
		  lox::token_type type,
		  lak::u8string_view message_on_err,
		  const std::source_location srcloc = std::source_location::current());

		void sync();

		lak::result<lox::expr_ptr> parse_primary();

		lak::result<lox::expr_ptr> parse_call();

		lak::result<lox::expr_ptr> parse_unary();

		lak::result<lox::expr_ptr> parse_factor();

		lak::result<lox::expr_ptr> parse_term();

		lak::result<lox::expr_ptr> parse_comparison();

		lak::result<lox::expr_ptr> parse_equality();

		lak::result<lox::expr_ptr> parse_and();

		lak::result<lox::expr_ptr> parse_or();

		lak::result<lox::expr_ptr> parse_assignment();

		lak::result<lox::expr_ptr> parse_expression();

		lak::result<lox::stmt_ptr> parse_print_statement();

		lak::result<lox::stmt_ptr> parse_return_statement();

		lak::result<lox::stmt_ptr> parse_expression_statement();

		lak::result<lox::stmt::function_ptr> parse_function_ptr(
		  const lak::u8string &kind);

		lak::result<lox::stmt_ptr> parse_function(const lak::u8string &kind);

		lak::result<std::vector<lox::stmt_ptr>> parse_block();

		lak::result<lox::stmt_ptr> parse_branch_statement();

		lak::result<lox::stmt_ptr> parse_while_loop_statement();

		lak::result<lox::stmt_ptr> parse_for_loop_statement();

		lak::result<lox::stmt_ptr> parse_statement();

		lak::result<lox::stmt_ptr> parse_var_declaration();

		lak::result<lox::stmt_ptr> parse_class_declaration();

		lak::result<lox::stmt_ptr> parse_declaration();

		lak::result<std::vector<lox::stmt_ptr>> parse();
	};
}

#endif
