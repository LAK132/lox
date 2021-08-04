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

	if (match({IDENTIFIER})) return lox::expr::make_variable({.name = last()});

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

lox::expr_ptr lox::parser::parse_and()
{
	lox::expr_ptr expr = parse_equality();
	if (!expr) return lox::expr_ptr{};

	while (match({lox::token_type::AND}))
	{
		lox::token op       = last();
		lox::expr_ptr right = parse_equality();
		if (!right) return lox::expr_ptr{};

		expr = lox::expr::make_logical({
		  .left  = std::move(expr),
		  .op    = op,
		  .right = std::move(right),
		});
	}

	return expr;
}

lox::expr_ptr lox::parser::parse_or()
{
	lox::expr_ptr expr = parse_and();
	if (!expr) return lox::expr_ptr{};

	while (match({lox::token_type::OR}))
	{
		lox::token op       = last();
		lox::expr_ptr right = parse_and();
		if (!right) return lox::expr_ptr{};

		expr = lox::expr::make_logical({
		  .left  = std::move(expr),
		  .op    = op,
		  .right = std::move(right),
		});
	}

	return expr;
}

lox::expr_ptr lox::parser::parse_assignment()
{
	lox::expr_ptr expr = parse_or();
	if (!expr) return lox::expr_ptr{};

	if (match({lox::token_type::EQUAL}))
	{
		lox::token equals   = last();
		lox::expr_ptr value = parse_assignment();
		if (!value) return lox::expr_ptr{};

		if (std::holds_alternative<lox::expr::variable>(expr->value))
		{
			return lox::expr::make_assign({
			  .name  = std::get<lox::expr::variable>(expr->value).name,
			  .value = std::move(value),
			});
		}

		interpreter.error(equals, u8"Invalid assignment target.");
		return lox::expr_ptr{};
	}

	return expr;
}

lox::expr_ptr lox::parser::parse_expression()
{
	return parse_assignment();
}

lox::stmt_ptr lox::parser::parse_print_statement()
{
	lox::expr_ptr value = parse_expression();
	if (!consume(lox::token_type::SEMICOLON, u8"Expected ';' after value."))
		return lox::stmt_ptr{};
	return lox::stmt::make_print({.expression = std::move(value)});
}

lox::stmt_ptr lox::parser::parse_expression_statement()
{
	lox::expr_ptr expr = parse_expression();
	if (!consume(lox::token_type::SEMICOLON, u8"Expected ';' after expression."))
		return lox::stmt_ptr{};
	return lox::stmt::make_expr({.expression = std::move(expr)});
}

std::optional<std::vector<lox::stmt_ptr>> lox::parser::parse_block()
{
	std::vector<lox::stmt_ptr> result;

	while (!check(lox::token_type::RIGHT_BRACE) && !empty())
	{
		lox::stmt_ptr decl = parse_declaration();
		if (!decl) return std::nullopt;
		result.push_back(std::move(decl));
	}

	if (!consume(lox::token_type::RIGHT_BRACE, u8"Expected '}' after block."))
		return std::nullopt;

	return result;
}

lox::stmt_ptr lox::parser::parse_branch_statement()
{
	if (!consume(lox::token_type::LEFT_PAREN, u8"Expected '(' after 'if'."))
		return lox::stmt_ptr{};

	lox::expr_ptr condition = parse_expression();
	if (!condition) return lox::stmt_ptr{};

	if (!consume(lox::token_type::RIGHT_PAREN,
	             u8"Expected '(' after if condition."))
		return lox::stmt_ptr{};

	lox::stmt_ptr then_branch = parse_statement();
	if (!then_branch) return lox::stmt_ptr{};

	lox::stmt_ptr else_branch;
	if (match({lox::token_type::ELSE}))
	{
		else_branch = parse_statement();
		if (!else_branch) return lox::stmt_ptr{};
	}

	return lox::stmt::make_branch({
	  .condition  = std::move(condition),
	  .then_brach = std::move(then_branch),
	  .else_brach = std::move(else_branch),
	});
}

lox::stmt_ptr lox::parser::parse_while_loop_statement()
{
	if (!consume(lox::token_type::LEFT_PAREN, u8"Expected '(' after 'while'."))
		return lox::stmt_ptr{};

	lox::expr_ptr condition = parse_expression();
	if (!condition) return lox::stmt_ptr{};

	if (!consume(lox::token_type::RIGHT_PAREN,
	             u8"Expected ')' after condition."))
		return lox::stmt_ptr{};

	lox::stmt_ptr body = parse_statement();
	if (!body) return lox::stmt_ptr{};

	return lox::stmt::make_loop({
	  .condition = std::move(condition),
	  .body      = std::move(body),
	});
}

lox::stmt_ptr lox::parser::parse_for_loop_statement()
{
	if (!consume(lox::token_type::LEFT_PAREN, u8"Expected '(' after 'for'."))
		return lox::stmt_ptr{};

	lox::stmt_ptr init;
	if (match({lox::token_type::VAR}))
	{
		init = parse_var_declaration();
		if (!init) return lox::stmt_ptr{};
	}
	else if (!match({lox::token_type::SEMICOLON})) // NOT a semicolon
	{
		init = parse_expression_statement();
		if (!init) return lox::stmt_ptr{};
	}

	lox::expr_ptr condition;
	if (!check(lox::token_type::SEMICOLON))
	{
		condition = parse_expression();
		if (!condition) return lox::stmt_ptr{};
	}
	if (!consume(lox::token_type::SEMICOLON,
	             u8"Expected ';' after loop condition."))
		return lox::stmt_ptr{};

	lox::expr_ptr increment;
	if (!check(lox::token_type::RIGHT_PAREN))
	{
		increment = parse_expression();
		if (!increment) return lox::stmt_ptr{};
	}
	if (!consume(lox::token_type::RIGHT_PAREN,
	             u8"Expected ')' after for clauses."))
		return lox::stmt_ptr{};

	lox::stmt_ptr body = parse_statement();
	if (!body) return lox::stmt_ptr{};

	if (increment)
	{
		std::vector<lox::stmt_ptr> statements;
		statements.push_back(std::move(body));
		statements.push_back(
		  lox::stmt::make_expr({.expression = std::move(increment)}));

		body = lox::stmt::make_block({.statements = std::move(statements)});
	}

	if (!condition)
		condition = lox::expr::make_literal({
		  .value = true,
		});

	body = lox::stmt::make_loop({
	  .condition = std::move(condition),
	  .body      = std::move(body),
	});

	if (init)
	{
		std::vector<lox::stmt_ptr> statements;
		statements.push_back(std::move(init));
		statements.push_back(std::move(body));

		body = lox::stmt::make_block({.statements = std::move(statements)});
	}

	return body;
}

lox::stmt_ptr lox::parser::parse_statement()
{
	if (match({lox::token_type::FOR})) return parse_for_loop_statement();

	if (match({lox::token_type::IF})) return parse_branch_statement();

	if (match({lox::token_type::PRINT})) return parse_print_statement();

	if (match({lox::token_type::WHILE})) return parse_while_loop_statement();

	if (match({lox::token_type::LEFT_BRACE}))
	{
		if (std::optional<std::vector<lox::stmt_ptr>> block = parse_block(); block)
			return lox::stmt::make_block({.statements = std::move(*block)});
		else
			return lox::stmt_ptr{};
	}

	return parse_expression_statement();
}

lox::stmt_ptr lox::parser::parse_var_declaration()
{
	const lox::token *name =
	  consume(lox::token_type::IDENTIFIER, u8"Expected variable name.");
	if (!name) return lox::stmt_ptr{};

	lox::expr_ptr init;
	if (match({lox::token_type::EQUAL}))
	{
		init = parse_expression();
		if (!init) return lox::stmt_ptr{};
	}

	if (!consume(lox::token_type::SEMICOLON,
	             u8"Expected ';' after variable declaration."))
		return lox::stmt_ptr{};

	return lox::stmt::make_var({.name = *name, .init = std::move(init)});
}

lox::stmt_ptr lox::parser::parse_declaration()
{
	auto result = [&]()
	{
		if (match({lox::token_type::VAR}))
			return parse_var_declaration();
		else
			return parse_statement();
	}();

	if (!result) sync();

	return result;
}

std::vector<lox::stmt_ptr> lox::parser::parse()
{
	std::vector<lox::stmt_ptr> result;
	while (!empty()) result.push_back(parse_declaration());
	return result;
}
