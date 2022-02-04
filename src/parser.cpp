#include "parser.hpp"

#include <lak/debug.hpp>

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
                                       lak::u8string_view message_on_err,
                                       const std::source_location srcloc)
{
	if (check(type)) return &next();
	interpreter.error(peek(), message_on_err, srcloc);
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

lak::result<lox::expr_ptr> lox::parser::parse_primary()
{
	using enum lox::token_type;
	if (match({FALSE}))
		return lak::ok_t{lox::expr::make_literal({.value = false})};
	if (match({TRUE}))
		return lak::ok_t{lox::expr::make_literal({.value = true})};
	if (match({NIL})) return lak::ok_t{lox::expr::make_literal({})};

	if (match({NUMBER, STRING}))
		return lak::ok_t{lox::expr::make_literal({.value = last().literal})};

	if (match({SUPER}))
	{
		lox::token keyword = last();

		if (!consume(DOT, u8"Expected '.' after 'super'.")) return lak::err_t{};

		const lox::token *method =
		  consume(IDENTIFIER, u8"Expected superclass method name.");
		if (!method) return lak::err_t{};

		return lak::ok_t{lox::expr::make_super({
		  .keyword = lak::move(keyword),
		  .method  = *method,
		})};
	}

	if (match({THIS}))
		return lak::ok_t{lox::expr::make_this({.keyword = last()})};

	if (match({IDENTIFIER}))
		return lak::ok_t{lox::expr::make_variable({.name = last()})};

	if (match({LEFT_PAREN}))
	{
		RES_TRY_ASSIGN(lox::expr_ptr e =, parse_expression());
		if (!consume(RIGHT_PAREN, u8"Expected ')' after expression."))
			return lak::err_t{};
		return lak::ok_t{lox::expr::make_grouping({.expression = lak::move(e)})};
	}

	interpreter.error(peek(), u8"Expected expression.");
	return lak::err_t{};
}

lak::result<lox::expr_ptr> lox::parser::parse_call()
{
	auto finish_call = [&](lox::expr_ptr &&callee) -> lak::result<lox::expr_ptr>
	{
		std::vector<lox::expr_ptr> arguments;
		if (!check(lox::token_type::RIGHT_PAREN))
		{
			do
			{
				if (arguments.size() >= 255)
				{
					interpreter.error(peek(), u8"Can't have more than 255 arguments.");
					return lak::err_t{};
				}
				RES_TRY_ASSIGN(lox::expr_ptr argument =, parse_expression());

				arguments.push_back(lak::move(argument));
			} while (match({lox::token_type::COMMA}));
		}

		const lox::token *paren =
		  consume(lox::token_type::RIGHT_PAREN, u8"Expected ')' after arguments.");
		if (!paren) return lak::err_t{};

		return lak::ok_t{lox::expr::make_call({
		  .callee    = lak::move(callee),
		  .paren     = *paren,
		  .arguments = lak::move(arguments),
		})};
	};

	RES_TRY_ASSIGN(lox::expr_ptr expr =, parse_primary());

	for (;;)
	{
		if (match({lox::token_type::LEFT_PAREN}))
		{
			RES_TRY_ASSIGN(expr =, finish_call(lak::move(expr)));
		}
		else if (match({lox::token_type::DOT}))
		{
			const lox::token *name = consume(lox::token_type::IDENTIFIER,
			                                 u8"Expected property name after '.'.");
			if (!name) return lak::err_t{};
			expr = lox::expr::make_get({
			  .object = lak::move(expr),
			  .name   = *name,
			});
		}
		else
			break;
	}

	return lak::move_ok(expr);
}

lak::result<lox::expr_ptr> lox::parser::parse_unary()
{
	using enum lox::token_type;
	if (match({BANG, MINUS}))
	{
		lox::token op = last();
		RES_TRY_ASSIGN(lox::expr_ptr right =, parse_unary());

		return lak::ok_t{lox::expr::make_unary({
		  .op    = op,
		  .right = lak::move(right),
		})};
	}

	return parse_call();
}

lak::result<lox::expr_ptr> lox::parser::parse_factor()
{
	RES_TRY_ASSIGN(lox::expr_ptr expr =, parse_unary());

	using enum lox::token_type;
	while (match({SLASH, STAR}))
	{
		lox::token op = last();
		RES_TRY_ASSIGN(lox::expr_ptr right =, parse_unary());

		expr = lox::expr::make_binary({
		  .left  = lak::move(expr),
		  .op    = op,
		  .right = lak::move(right),
		});
	}

	return lak::move_ok(expr);
}

lak::result<lox::expr_ptr> lox::parser::parse_term()
{
	RES_TRY_ASSIGN(lox::expr_ptr expr =, parse_factor());

	using enum lox::token_type;
	while (match({MINUS, PLUS}))
	{
		lox::token op = last();
		RES_TRY_ASSIGN(lox::expr_ptr right =, parse_factor());

		expr = lox::expr::make_binary({
		  .left  = lak::move(expr),
		  .op    = op,
		  .right = lak::move(right),
		});
	}

	return lak::move_ok(expr);
}

lak::result<lox::expr_ptr> lox::parser::parse_comparison()
{
	RES_TRY_ASSIGN(lox::expr_ptr expr =, parse_term());

	using enum lox::token_type;
	while (match({GREATER, GREATER_EQUAL, LESS, LESS_EQUAL}))
	{
		lox::token op = last();
		RES_TRY_ASSIGN(lox::expr_ptr right =, parse_term());

		expr = lox::expr::make_binary({
		  .left  = lak::move(expr),
		  .op    = op,
		  .right = lak::move(right),
		});
	}

	return lak::move_ok(expr);
}

lak::result<lox::expr_ptr> lox::parser::parse_equality()
{
	RES_TRY_ASSIGN(lox::expr_ptr expr =, parse_comparison());

	using enum lox::token_type;
	while (match({BANG_EQUAL, EQUAL_EQUAL}))
	{
		lox::token op = last();
		RES_TRY_ASSIGN(lox::expr_ptr right =, parse_comparison());

		expr = lox::expr::make_binary({
		  .left  = lak::move(expr),
		  .op    = op,
		  .right = lak::move(right),
		});
	}

	return lak::move_ok(expr);
}

lak::result<lox::expr_ptr> lox::parser::parse_and()
{
	RES_TRY_ASSIGN(lox::expr_ptr expr =, parse_equality());

	while (match({lox::token_type::AND}))
	{
		lox::token op = last();
		RES_TRY_ASSIGN(lox::expr_ptr right =, parse_equality());

		expr = lox::expr::make_logical({
		  .left  = lak::move(expr),
		  .op    = op,
		  .right = lak::move(right),
		});
	}

	return lak::move_ok(expr);
}

lak::result<lox::expr_ptr> lox::parser::parse_or()
{
	RES_TRY_ASSIGN(lox::expr_ptr expr =, parse_and());

	while (match({lox::token_type::OR}))
	{
		lox::token op = last();
		RES_TRY_ASSIGN(lox::expr_ptr right =, parse_and());

		expr = lox::expr::make_logical({
		  .left  = lak::move(expr),
		  .op    = op,
		  .right = lak::move(right),
		});
	}

	return lak::move_ok(expr);
}

lak::result<lox::expr_ptr> lox::parser::parse_assignment()
{
	RES_TRY_ASSIGN(lox::expr_ptr expr =, parse_or());

	if (match({lox::token_type::EQUAL}))
	{
		lox::token equals = last();
		RES_TRY_ASSIGN(lox::expr_ptr value =, parse_assignment());

		if_ref (const lox::expr::variable & var,
		        expr->value.template get<lox::expr::variable>())
		{
			return lak::ok_t{lox::expr::make_assign({
			  .name  = var.name,
			  .value = lak::move(value),
			})};
		}
		else if_ref (lox::expr::get & get,
		             expr->value.template get<lox::expr::get>())
		{
			return lak::ok_t{lox::expr::make_set({
			  .object = lak::move(get.object),
			  .name   = lak::move(get.name),
			  .value  = lak::move(value),
			})};
		}

		interpreter.error(equals, u8"Invalid assignment target.");
		return lak::err_t{};
	}

	return lak::move_ok(expr);
}

lak::result<lox::expr_ptr> lox::parser::parse_expression()
{
	return parse_assignment();
}

lak::result<lox::stmt_ptr> lox::parser::parse_print_statement()
{
	RES_TRY_ASSIGN(lox::expr_ptr value =, parse_expression());
	if (!consume(lox::token_type::SEMICOLON, u8"Expected ';' after value."))
		return lak::err_t{};
	return lak::ok_t{lox::stmt::make_print({.expression = lak::move(value)})};
}

lak::result<lox::stmt_ptr> lox::parser::parse_return_statement()
{
	lox::token keyword = last();
	lak::optional<lox::expr_ptr> value;

	if (!check(lox::token_type::SEMICOLON))
	{
		RES_TRY_ASSIGN(value =, parse_expression());
	}

	if (!consume(lox::token_type::SEMICOLON,
	             u8"Expected ';' after return value."))
		return lak::err_t{};

	return lak::ok_t{lox::stmt::make_ret({
	  .keyword = keyword,
	  .value   = lak::move(value),
	})};
}

lak::result<lox::stmt_ptr> lox::parser::parse_expression_statement()
{
	RES_TRY_ASSIGN(lox::expr_ptr expr =, parse_expression());
	if (!consume(lox::token_type::SEMICOLON, u8"Expected ';' after expression."))
		return lak::err_t{};
	return lak::ok_t{lox::stmt::make_expr({.expression = lak::move(expr)})};
}

lak::result<lox::stmt::function_ptr> lox::parser::parse_function_ptr(
  const lak::u8string &kind)
{
	const lox::token *name =
	  consume(lox::token_type::IDENTIFIER, u8"Expected " + kind + u8" name.");
	if (!name) return lak::err_t{};

	if (!consume(lox::token_type::LEFT_PAREN,
	             u8"Expected '(' after " + kind + u8" name."))
		return lak::err_t{};

	std::vector<lox::token> parameters;
	if (!check(lox::token_type::RIGHT_PAREN))
	{
		do
		{
			if (parameters.size() >= 255)
			{
				interpreter.error(peek(), u8"Can't have more than 255 parameters.");
				return lak::err_t{};
			}

			if_ref (const lox::token & param,
			        consume(lox::token_type::IDENTIFIER,
			                u8"Expected parameter name."))
				parameters.emplace_back(param);
			else
				return lak::err_t{};
		} while (match({lox::token_type::COMMA}));
	}

	if (!consume(lox::token_type::RIGHT_PAREN,
	             u8"Expected '(' after parameters."))
		return lak::err_t{};

	if (!consume(lox::token_type::LEFT_BRACE,
	             u8"Expected '{' before " + kind + u8" body."))
		return lak::err_t{};

	RES_TRY_ASSIGN(std::vector<lox::stmt_ptr> body =, parse_block());

	return lak::ok_t{lox::stmt::make_function_ptr({
	  .name       = *name,
	  .parameters = lak::move(parameters),
	  .body       = lak::move(body),
	})};
}

lak::result<lox::stmt_ptr> lox::parser::parse_function(
  const lak::u8string &kind)
{
	return parse_function_ptr(kind).map(lox::stmt::make_function_from_ptr);
}

lak::result<std::vector<lox::stmt_ptr>> lox::parser::parse_block()
{
	std::vector<lox::stmt_ptr> result;

	while (!check(lox::token_type::RIGHT_BRACE) && !empty())
	{
		RES_TRY_ASSIGN(lox::stmt_ptr decl =, parse_declaration());
		result.push_back(lak::move(decl));
	}

	if (!consume(lox::token_type::RIGHT_BRACE, u8"Expected '}' after block."))
		return lak::err_t{};

	return lak::move_ok(result);
}

lak::result<lox::stmt_ptr> lox::parser::parse_branch_statement()
{
	if (!consume(lox::token_type::LEFT_PAREN, u8"Expected '(' after 'if'."))
		return lak::err_t{};

	RES_TRY_ASSIGN(lox::expr_ptr condition =, parse_expression());

	if (!consume(lox::token_type::RIGHT_PAREN,
	             u8"Expected '(' after if condition."))
		return lak::err_t{};

	RES_TRY_ASSIGN(lox::stmt_ptr then_branch =, parse_statement());

	lak::optional<lox::stmt_ptr> else_branch;
	if (match({lox::token_type::ELSE}))
	{
		RES_TRY_ASSIGN(else_branch =, parse_statement());
	}

	return lak::ok_t{lox::stmt::make_branch({
	  .condition   = lak::move(condition),
	  .then_branch = lak::move(then_branch),
	  .else_branch = lak::move(else_branch),
	})};
}

lak::result<lox::stmt_ptr> lox::parser::parse_while_loop_statement()
{
	if (!consume(lox::token_type::LEFT_PAREN, u8"Expected '(' after 'while'."))
		return lak::err_t{};

	RES_TRY_ASSIGN(lox::expr_ptr condition =, parse_expression());

	if (!consume(lox::token_type::RIGHT_PAREN,
	             u8"Expected ')' after condition."))
		return lak::err_t{};

	RES_TRY_ASSIGN(lox::stmt_ptr body =, parse_statement());

	return lak::ok_t{lox::stmt::make_loop({
	  .condition = lak::move(condition),
	  .body      = lak::move(body),
	})};
}

lak::result<lox::stmt_ptr> lox::parser::parse_for_loop_statement()
{
	if (!consume(lox::token_type::LEFT_PAREN, u8"Expected '(' after 'for'."))
		return lak::err_t{};

	lak::optional<lox::stmt_ptr> init;
	if (match({lox::token_type::VAR}))
	{
		RES_TRY_ASSIGN(init =, parse_var_declaration());
	}
	else if (!match({lox::token_type::SEMICOLON})) // NOT a semicolon
	{
		RES_TRY_ASSIGN(init =, parse_expression_statement());
	}

	RES_TRY_ASSIGN(lox::expr_ptr condition =,
	               parse_expression().or_else(
	                 [](lak::monostate) -> lak::result<lox::expr_ptr>
	                 {
		                 return lak::ok_t<lox::expr_ptr>{
		                   lox::expr::make_literal({.value = true})};
	                 }));
	if (!consume(lox::token_type::SEMICOLON,
	             u8"Expected ';' after loop condition."))
		return lak::err_t{};

	lak::optional<lox::expr_ptr> increment;
	if (!check(lox::token_type::RIGHT_PAREN))
	{
		RES_TRY_ASSIGN(increment =, parse_expression());
	}
	if (!consume(lox::token_type::RIGHT_PAREN,
	             u8"Expected ')' after for clauses."))
		return lak::err_t{};

	RES_TRY_ASSIGN(lox::stmt_ptr body =, parse_statement());

	if_ref (lox::expr_ptr & inc, increment)
	{
		std::vector<lox::stmt_ptr> statements;
		statements.push_back(lak::move(body));
		statements.push_back(lox::stmt::make_expr({.expression = lak::move(inc)}));
		increment.reset();

		body = lox::stmt::make_block({.statements = lak::move(statements)});
	}

	body = lox::stmt::make_loop({
	  .condition = lak::move(condition),
	  .body      = lak::move(body),
	});

	if_ref (lox::stmt_ptr & i, init)
	{
		std::vector<lox::stmt_ptr> statements;
		statements.push_back(lak::move(i));
		statements.push_back(lak::move(body));
		init.reset();

		body = lox::stmt::make_block({.statements = lak::move(statements)});
	}

	return lak::move_ok(body);
}

lak::result<lox::stmt_ptr> lox::parser::parse_statement()
{
	if (match({lox::token_type::FOR})) return parse_for_loop_statement();

	if (match({lox::token_type::IF})) return parse_branch_statement();

	if (match({lox::token_type::PRINT})) return parse_print_statement();

	if (match({lox::token_type::RETURN})) return parse_return_statement();

	if (match({lox::token_type::WHILE})) return parse_while_loop_statement();

	if (match({lox::token_type::LEFT_BRACE}))
	{
		return parse_block().map(
		  [](std::vector<lox::stmt_ptr> &&block) -> lox::stmt_ptr
		  { return lox::stmt::make_block({.statements = lak::move(block)}); });
	}

	return parse_expression_statement();
}

lak::result<lox::stmt_ptr> lox::parser::parse_var_declaration()
{
	const lox::token *name =
	  consume(lox::token_type::IDENTIFIER, u8"Expected variable name.");
	if (!name) return lak::err_t{};

	lak::optional<lox::expr_ptr> init;
	if (match({lox::token_type::EQUAL}))
	{
		RES_TRY_ASSIGN(init =, parse_expression());
	}

	if (!consume(lox::token_type::SEMICOLON,
	             u8"Expected ';' after variable declaration."))
		return lak::err_t{};

	return lak::ok_t{
	  lox::stmt::make_var({.name = *name, .init = lak::move(init)})};
}

lak::result<lox::stmt_ptr> lox::parser::parse_class_declaration()
{
	const lox::token *name =
	  consume(lox::token_type::IDENTIFIER, u8"Expected class name.");
	if (!name) return lak::err_t{};

	lak::optional<lox::expr::variable> superclass;
	if (match({lox::token_type::LESS}))
	{
		if (!consume(lox::token_type::IDENTIFIER, u8"Expected superclass name."))
			return lak::err_t{};
		superclass = lox::expr::variable{.name = last()};
	}

	if (!consume(lox::token_type::LEFT_BRACE,
	             u8"Expected '{' before class body."))
		return lak::err_t{};

	std::vector<lox::stmt::function_ptr> methods;
	while (!check(lox::token_type::RIGHT_BRACE) && !empty())
	{
		RES_TRY_ASSIGN(lox::stmt::function_ptr method =,
		               parse_function_ptr(u8"method"));

		methods.emplace_back(lak::move(method));
	}

	if (!consume(lox::token_type::RIGHT_BRACE,
	             u8"Expected '}' after class body."))
		return lak::err_t{};

	return lak::ok_t{lox::stmt::make_type({
	  .name       = *name,
	  .superclass = lak::move(superclass),
	  .methods    = lak::move(methods),
	})};
}

lak::result<lox::stmt_ptr> lox::parser::parse_declaration()
{
	auto result = [&]()
	{
		if (match({lox::token_type::CLASS}))
			return parse_class_declaration();
		else if (match({lox::token_type::FUN}))
			return parse_function(u8"function");
		else if (match({lox::token_type::VAR}))
			return parse_var_declaration();
		else
			return parse_statement();
	}();

	if (result.is_err()) sync();

	return result;
}

lak::result<std::vector<lox::stmt_ptr>> lox::parser::parse()
{
	std::vector<lox::stmt_ptr> result;
	while (!empty())
	{
		RES_TRY_ASSIGN(lox::stmt_ptr stmt =, parse_declaration());
		result.push_back(lak::move(stmt));
	}
	return lak::move_ok(result);
}
