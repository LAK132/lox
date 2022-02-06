#include "parser.hpp"

#include <lak/debug.hpp>

bool lox::parser::empty() const
{
	return current.type == lox::token_type::EOF_TOK;
}

lox::parse_result<> lox::parser::next()
{
	if (empty())
		return lak::err_t<lox::positional_error>{
		  {.line = current.line, .message = u8"Unexpected end of file."_str}};

	return scanner.scan_token().map(
	  [&](const lox::token &tok) -> lak::monostate
	  {
		  previous = lak::exchange(current, tok);
		  return {};
	  });
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
		return lak::err_t<lox::positional_error>{
		  {.line = current.line, .message = lak::u8string(message_on_err)}};

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
		return lak::err_t<lox::positional_error>{
		  {.line    = previous.line,
		   .message = u8"Too many constants in one chunk."_str}};

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
		case lox::token_type::MINUS:
			chunk.push_opcode(lox::opcode::OP_NEGATE, op.line);
			break;

		default:
			return lak::err_t<lox::positional_error>{
			  {.line = previous.line,
			   .message =
			     u8"Invalid operator '"_str + lak::u8string(op.lexeme) + u8"'"}};
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
			return lak::err_t<lox::positional_error>{
			  {.line    = op.line,
			   .message = u8"Invalid binary operator '"_str +
			              lak::u8string(op.lexeme) + u8"'"}};
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
		return lak::err_t<lox::positional_error>{
		  {.line = previous.line, .message = u8"Expected expression."_str}};

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

		case lox::token_type::NUMBER:
		{
			static constexpr parse_rule rule{
			  .prefix     = &lox::parser::parse_number,
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
