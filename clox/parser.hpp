#ifndef LOX_PARSER_HPP
#define LOX_PARSER_HPP

#include "chunk.hpp"
#include "error.hpp"
#include "scanner.hpp"
#include "token.hpp"

#include <lak/result.hpp>
#include <lak/stdint.hpp>

#include <initializer_list>
#include <source_location>
#include <vector>

namespace lox
{
	struct parse_error_tag;

	using parse_error = lox::positional_error<lox::parse_error_tag>;

	template<typename T = lak::monostate>
	using parse_result =
	  lak::result<T, lox::result_set<lox::scan_error, lox::parse_error>>;

	struct parser
	{
		lox::scanner &scanner;
		lox::token previous, current;
		lox::chunk chunk;

		bool empty() const;

		lox::parse_result<> next();

		bool check(lox::token_type type) const;

		lox::parse_result<const lox::token &> consume(
		  lox::token_type type, lak::u8string_view message_on_err);

		lox::parse_result<> emit_constant(const lox::value &val);

		lox::parse_result<> parse_number();

		lox::parse_result<> parse_grouping();

		lox::parse_result<> parse_unary();

		lox::parse_result<> parse_binary();

		lox::parse_result<> parse_literal();

		lox::parse_result<> parse_expression();

		enum struct precedence
		{
			NONE,
			ASSIGNMENT, // =
			OR,         // or
			AND,        // and
			EQUALITY,   // == !=
			COMPARISON, // < > <= >=
			TERM,       // + -
			FACTOR,     // * /
			UNARY,      // ! -
			CALL,       // . ()
			PRIMARY,
		};

		friend precedence operator+(precedence prec, int offset)
		{
			return static_cast<precedence>(static_cast<uintmax_t>(prec) + offset);
		}

		lox::parse_result<> parse_precedence(precedence prec);

		struct parse_rule
		{
			lox::parse_result<> (lox::parser::*prefix)();
			lox::parse_result<> (lox::parser::*infix)();
			lox::parser::precedence precedence;
		};

		const parse_rule &get_rule(lox::token_type type);
	};
}

#endif
