#include "parser.hpp"

bool lox::parser::empty() const
{
	return peek().type == lox::token_type::EOF_TOK;
}

const lox::token &lox::parser::last()
{
	return tokens[current - 1];
}

const lox::token &lox::parser::next()
{
	if (!empty()) ++current;
	return last();
}

const lox::token &lox::parser::peek() const
{
	return tokens[current];
}

bool lox::parser::check(lox::token_type type) const
{
	if (empty()) return false;
	return peek().type == type;
}

bool lox::parser::match(std::initializer_list<lox::token_type> types)
{
	for (const auto &type : types)
	{
		if (check(type))
		{
			next();
			return true;
		}
	}

	return false;
}

const lox::token *lox::parser::consume(lox::token_type type,
                                       std::u8string_view message_on_err)
{
	if (check(type)) return &next();
	interpreter.error(peek(), message_on_err);
	return nullptr;
}

void lox::parser::sync()
{
	next();

	while (!empty())
	{
		using enum lox::token_type;
		if (last().type == SEMICOLON) return;
		switch (peek().type)
		{
			case CLASS: [[fallthrough]];
			case FUN: [[fallthrough]];
			case VAR: [[fallthrough]];
			case FOR: [[fallthrough]];
			case IF: [[fallthrough]];
			case WHILE: [[fallthrough]];
			case PRINT: [[fallthrough]];
			case RETURN: return;
		}

		next();
	}
}

lox::expr_ptr lox::parser::parse_primary()
{
	using enum lox::token_type;
	if (match({FALSE})) return lox::expr::make_literal({.value = false});
	if (match({TRUE})) return lox::expr::make_literal({.value = true});
	if (match({NIL})) return lox::expr::make_literal({});

	if (match({NUMBER, STRING}))
		return lox::expr::make_literal({.value = last().literal});

	if (match({LEFT_PAREN}))
	{
		lox::expr_ptr e = parse_expression();
		if (!consume(RIGHT_PAREN, u8"Expected ')' after expression."))
			return lox::expr_ptr{};
		return lox::expr::make_grouping({.expression = std::move(e)});
	}

	interpreter.error(peek(), u8"Expected expression.");
	return lox::expr_ptr{};
}

lox::expr_ptr lox::parser::parse_unary()
{
	using enum lox::token_type;
	if (match({BANG, MINUS}))
	{
		lox::token op       = last();
		lox::expr_ptr right = parse_unary();
		if (!right) return lox::expr_ptr{};

		return lox::expr::make_unary({
		  .op    = op,
		  .right = std::move(right),
		});
	}

	return parse_primary();
}

lox::expr_ptr lox::parser::parse_factor()
{
	lox::expr_ptr e = parse_unary();
	if (!e) return lox::expr_ptr{};

	using enum lox::token_type;
	while (match({SLASH, STAR}))
	{
		lox::token op       = last();
		lox::expr_ptr right = parse_unary();
		if (!right) return lox::expr_ptr{};

		e = lox::expr::make_binary({
		  .left  = std::move(e),
		  .op    = op,
		  .right = std::move(right),
		});
	}

	return e;
}

lox::expr_ptr lox::parser::parse_term()
{
	lox::expr_ptr e = parse_factor();
	if (!e) return lox::expr_ptr{};

	using enum lox::token_type;
	while (match({MINUS, PLUS}))
	{
		lox::token op       = last();
		lox::expr_ptr right = parse_factor();
		if (!right) return lox::expr_ptr{};

		e = lox::expr::make_binary({
		  .left  = std::move(e),
		  .op    = op,
		  .right = std::move(right),
		});
	}

	return e;
}

lox::expr_ptr lox::parser::parse_comparison()
{
	lox::expr_ptr e = parse_term();
	if (!e) return lox::expr_ptr{};

	using enum lox::token_type;
	while (match({GREATER, GREATER_EQUAL, LESS, LESS_EQUAL}))
	{
		lox::token op       = last();
		lox::expr_ptr right = parse_term();
		if (!right) return lox::expr_ptr{};

		e = lox::expr::make_binary({
		  .left  = std::move(e),
		  .op    = op,
		  .right = std::move(right),
		});
	}

	return e;
}

lox::expr_ptr lox::parser::parse_equality()
{
	lox::expr_ptr e = parse_comparison();
	if (!e) return lox::expr_ptr{};

	using enum lox::token_type;
	while (match({BANG_EQUAL, EQUAL_EQUAL}))
	{
		lox::token op       = last();
		lox::expr_ptr right = parse_comparison();
		if (!right) return lox::expr_ptr{};

		e = lox::expr::make_binary({
		  .left  = std::move(e),
		  .op    = op,
		  .right = std::move(right),
		});
	}

	return e;
}

lox::expr_ptr lox::parser::parse_expression()
{
	return parse_equality();
}

lox::expr_ptr lox::parser::parse()
{
	return parse_expression();
}
