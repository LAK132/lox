#include "parser.hpp"

#include <lak/debug.hpp>

bool lox::parser::empty() const
{
	return current.type == lox::token_type::EOF_TOK;
}

lox::parse_result<> lox::parser::next()
{
	if (empty())
		return lak::err_t{
		  lox::parse_error::at(current.line, u8"Unexpected end of file."_str)};

	RES_TRY_ASSIGN(lox::token tok =, scanner.scan_token());

	previous = current;
	current  = tok;
	return lak::ok_t{};
}

bool lox::parser::check(lox::token_type type) const
{
	if (type != lox::token_type::EOF_TOK && empty()) return false;
	return current.type == type;
}

lox::parse_result<const lox::token &> lox::parser::consume(
  lox::token_type type, lak::u8string_view message_on_err)
{
	if (!check(type))
		return lak::err_t{lox::parse_error::at(current, message_on_err)};

	if (type == lox::token_type::EOF_TOK)
		previous = current;
	else
		RES_TRY(next());
	return lak::ok_t{previous};
}

lox::parse_result<> lox::parser::emit_constant(const lox::value &val)
{
	size_t constant = chunk.push_constant(val);

	if (constant > UINT8_MAX)
		return lak::err_t{lox::parse_error::at(
		  previous.line, u8"Too mand constants in one chunk."_str)};

	chunk.push_opcode(lox::opcode::OP_CONSTANT, previous.line);
	chunk.push_code(static_cast<uint8_t>(constant), previous.line);

	return lak::ok_t{};
}

lox::parse_result<> lox::parser::parse_number()
{
	return emit_constant(previous.literal);
}

lox::parse_result<> lox::parser::parse_grouping()
{
	RES_TRY(parse_expression());
	RES_TRY(
	  consume(lox::token_type::RIGHT_PAREN, u8"Expected ')' after expression."));
	return lak::ok_t{};
}

lox::parse_result<> lox::parser::parse_unary()
{
	lox::token op = previous;

	RES_TRY(parse_precedence(precedence::UNARY));

	switch (op.type)
	{
		case lox::token_type::BANG:
			chunk.push_opcode(lox::opcode::OP_NOT, op.line);
			break;

		case lox::token_type::MINUS:
			chunk.push_opcode(lox::opcode::OP_NEGATE, op.line);
			break;

		default:
			return lak::err_t{lox::parse_error::at(op, u8"Invalid operator."_str)};
	}

	return lak::ok_t{};
}

lox::parse_result<> lox::parser::parse_binary()
{
	lox::token op = previous;

	const parse_rule &rule = get_rule(op.type);

	RES_TRY(parse_precedence(rule.precedence + 1));

	switch (op.type)
	{
		case lox::token_type::BANG_EQUAL:
			chunk.push_opcode(lox::opcode::OP_EQUAL, op.line);
			chunk.push_opcode(lox::opcode::OP_NOT, op.line);
			break;

		case lox::token_type::EQUAL_EQUAL:
			chunk.push_opcode(lox::opcode::OP_EQUAL, op.line);
			break;

		case lox::token_type::GREATER:
			chunk.push_opcode(lox::opcode::OP_GREATER, op.line);
			break;

		case lox::token_type::GREATER_EQUAL:
			chunk.push_opcode(lox::opcode::OP_LESS, op.line);
			chunk.push_opcode(lox::opcode::OP_NOT, op.line);
			break;

		case lox::token_type::LESS:
			chunk.push_opcode(lox::opcode::OP_LESS, op.line);
			break;

		case lox::token_type::LESS_EQUAL:
			chunk.push_opcode(lox::opcode::OP_GREATER, op.line);
			chunk.push_opcode(lox::opcode::OP_NOT, op.line);
			break;

		case lox::token_type::PLUS:
			chunk.push_opcode(lox::opcode::OP_ADD, op.line);
			break;

		case lox::token_type::MINUS:
			chunk.push_opcode(lox::opcode::OP_SUBTRACT, op.line);
			break;

		case lox::token_type::STAR:
			chunk.push_opcode(lox::opcode::OP_MULTIPLY, op.line);
			break;

		case lox::token_type::SLASH:
			chunk.push_opcode(lox::opcode::OP_DIVIDE, op.line);
			break;

		default:
			return lak::err_t{
			  lox::parse_error::at(op, u8"Invalid binary operator."_str)};
	}

	return lak::ok_t{};
}

lox::parse_result<> lox::parser::parse_literal()
{
	switch (previous.type)
	{
		case lox::token_type::FALSE:
			chunk.push_opcode(lox::opcode::OP_FALSE, previous.line);
			break;

		case lox::token_type::NIL:
			chunk.push_opcode(lox::opcode::OP_NIL, previous.line);
			break;

		case lox::token_type::TRUE:
			chunk.push_opcode(lox::opcode::OP_TRUE, previous.line);
			break;

		default:
			return lak::err_t{
			  lox::parse_error::at(previous, u8"Invalid literal."_str)};
	}

	return lak::ok_t{};
}

lox::parse_result<> lox::parser::parse_expression()
{
	return parse_precedence(precedence::ASSIGNMENT);
}

lox::parse_result<> lox::parser::parse_precedence(precedence prec)
{
	RES_TRY(next());

	auto prefix{get_rule(previous.type).prefix};

	if (!prefix)
		return lak::err_t{
		  lox::parse_error::at(previous, u8"Expected expression."_str)};

	RES_TRY((this->*prefix)());

	while (get_rule(current.type).precedence >= prec)
	{
		RES_TRY(next());
		auto infix{get_rule(previous.type).infix};
		if (!infix) continue;
		RES_TRY((this->*infix)());
	}

	return lak::ok_t{};
}

const lox::parser::parse_rule &lox::parser::get_rule(lox::token_type type)
{
	switch (type)
	{
		case lox::token_type::LEFT_PAREN:
		{
			static constexpr parse_rule rule{
			  .prefix     = &lox::parser::parse_grouping,
			  .infix      = nullptr,
			  .precedence = lox::parser::precedence::NONE,
			};
			return rule;
		}

		case lox::token_type::MINUS:
		{
			static constexpr parse_rule rule{
			  .prefix     = &lox::parser::parse_unary,
			  .infix      = &lox::parser::parse_binary,
			  .precedence = lox::parser::precedence::TERM,
			};
			return rule;
		}

		case lox::token_type::PLUS:
		{
			static constexpr parse_rule rule{
			  .prefix     = nullptr,
			  .infix      = &lox::parser::parse_binary,
			  .precedence = lox::parser::precedence::TERM,
			};
			return rule;
		}

		case lox::token_type::SLASH: [[fallthrough]];
		case lox::token_type::STAR:
		{
			static constexpr parse_rule rule{
			  .prefix     = nullptr,
			  .infix      = &lox::parser::parse_binary,
			  .precedence = lox::parser::precedence::FACTOR,
			};
			return rule;
		}

		case lox::token_type::BANG:
		{
			static constexpr parse_rule rule{
			  .prefix     = &lox::parser::parse_unary,
			  .infix      = nullptr,
			  .precedence = lox::parser::precedence::NONE,
			};
			return rule;
		}

		case lox::token_type::BANG_EQUAL:
		{
			static constexpr parse_rule rule{
			  .prefix     = nullptr,
			  .infix      = &lox::parser::parse_binary,
			  .precedence = lox::parser::precedence::EQUALITY,
			};
			return rule;
		}

		case lox::token_type::EQUAL_EQUAL:
		{
			static constexpr parse_rule rule{
			  .prefix     = nullptr,
			  .infix      = &lox::parser::parse_binary,
			  .precedence = lox::parser::precedence::EQUALITY,
			};
			return rule;
		}

		case lox::token_type::GREATER: [[fallthrough]];
		case lox::token_type::GREATER_EQUAL: [[fallthrough]];
		case lox::token_type::LESS: [[fallthrough]];
		case lox::token_type::LESS_EQUAL:
		{
			static constexpr parse_rule rule{
			  .prefix     = nullptr,
			  .infix      = &lox::parser::parse_binary,
			  .precedence = lox::parser::precedence::COMPARISON,
			};
			return rule;
		}

		case lox::token_type::NUMBER:
		{
			static constexpr parse_rule rule{
			  .prefix     = &lox::parser::parse_number,
			  .infix      = nullptr,
			  .precedence = lox::parser::precedence::NONE,
			};
			return rule;
		}

		case lox::token_type::FALSE:
		{
			static constexpr parse_rule rule{
			  .prefix     = &lox::parser::parse_literal,
			  .infix      = nullptr,
			  .precedence = lox::parser::precedence::NONE,
			};
			return rule;
		}

		case lox::token_type::NIL:
		{
			static constexpr parse_rule rule{
			  .prefix     = &lox::parser::parse_literal,
			  .infix      = nullptr,
			  .precedence = lox::parser::precedence::NONE,
			};
			return rule;
		}

		case lox::token_type::TRUE:
		{
			static constexpr parse_rule rule{
			  .prefix     = &lox::parser::parse_literal,
			  .infix      = nullptr,
			  .precedence = lox::parser::precedence::NONE,
			};
			return rule;
		}

		default:
		{
			static constexpr parse_rule empty_rule{
			  .prefix     = nullptr,
			  .infix      = nullptr,
			  .precedence = lox::parser::precedence::NONE,
			};
			return empty_rule;
		}
	}
}
