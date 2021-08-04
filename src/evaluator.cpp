#include "evaluator.hpp"

#include <iostream>

std::nullopt_t lox::evaluator::error(const lox::token &token,
                                     std::u8string_view message)
{
	interpreter.error(token, message);
	return std::nullopt;
}

std::optional<lox::object> lox::evaluator::operator()(
  const lox::expr::assign &expr)
{
	std::optional<lox::object> maybe_value = expr.value->visit(*this);
	if (!maybe_value) return std::nullopt;

	if (auto it = environment->replace(expr.name, *maybe_value);
	    it != environment->end())
		return it->second;
	else
		return error(expr.name,
		             u8"Undefined variable '" + std::u8string(expr.name.lexeme) +
		               u8"'.");
}

std::optional<lox::object> lox::evaluator::operator()(
  const lox::expr::binary &expr)
{
	std::optional<lox::object> maybe_left = expr.left->visit(*this);
	if (!maybe_left) return std::nullopt;
	const auto &left = *maybe_left;

	std::optional<lox::object> maybe_right = expr.right->visit(*this);
	if (!maybe_right) return std::nullopt;
	const auto &right = *maybe_right;

	switch (expr.op.type)
	{
		using enum lox::token_type;

		case EQUAL_EQUAL: return lox::object{.value = left == right};

		case BANG_EQUAL: return lox::object{.value = left != right};

		case GREATER:
		{
			auto left_num  = left.get_number();
			auto right_num = right.get_number();
			if (!left_num || !right_num)
				return error(expr.op, u8"Operands must be numbers.");
			return lox::object{.value = *left_num > *right_num};
		}

		case GREATER_EQUAL:
		{
			auto left_num  = left.get_number();
			auto right_num = right.get_number();
			if (!left_num || !right_num)
				return error(expr.op, u8"Operands must be numbers.");
			return lox::object{.value = *left_num >= *right_num};
		}

		case LESS:
		{
			auto left_num  = left.get_number();
			auto right_num = right.get_number();
			if (!left_num || !right_num)
				return error(expr.op, u8"Operands must be numbers.");
			return lox::object{.value = *left_num < *right_num};
		}

		case LESS_EQUAL:
		{
			auto left_num  = left.get_number();
			auto right_num = right.get_number();
			if (!left_num || !right_num)
				return error(expr.op, u8"Operands must be numbers.");
			return lox::object{.value = *left_num <= *right_num};
		}

		case PLUS:
		{
			{
				auto left_num  = left.get_number();
				auto right_num = right.get_number();
				if (left_num && right_num)
					return lox::object{.value = *left_num + *right_num};
			}
			{
				auto left_string  = left.get_string();
				auto right_string = right.get_string();
				if (left_string && right_string)
					return lox::object{.value = *left_string + *right_string};
			}
			return error(expr.op, u8"Operands must be two numbers or two strings.");
		}

		case MINUS:
		{
			auto left_num  = left.get_number();
			auto right_num = right.get_number();
			if (!left_num || !right_num)
				return error(expr.op, u8"Operands must be numbers.");
			return lox::object{.value = *left_num - *right_num};
		}

		case SLASH:
		{
			auto left_num  = left.get_number();
			auto right_num = right.get_number();
			if (!left_num || !right_num)
				return error(expr.op, u8"Operands must be numbers.");
			return lox::object{.value = *left_num / *right_num};
		}

		case STAR:
		{
			auto left_num  = left.get_number();
			auto right_num = right.get_number();
			if (!left_num || !right_num)
				return error(expr.op, u8"Operands must be numbers.");
			return lox::object{.value = *left_num * *right_num};
		}
	}
	return lox::object{};
}

std::optional<lox::object> lox::evaluator::operator()(
  const lox::expr::grouping &expr)
{
	return expr.expression->visit(*this);
}

std::optional<lox::object> lox::evaluator::operator()(
  const lox::expr::literal &expr)
{
	return expr.value;
}

std::optional<lox::object> lox::evaluator::operator()(
  const lox::expr::logical &expr)
{
	std::optional<lox::object> maybe_left = expr.left->visit(*this);
	if (!maybe_left) return std::nullopt;
	const auto &left = *maybe_left;

	if (expr.op.type == lox::token_type::OR)
	{
		if (left.is_truthy()) return left;
	}
	else
	{
		if (!left.is_truthy()) return left;
	}

	return expr.right->visit(*this);
}

std::optional<lox::object> lox::evaluator::operator()(
  const lox::expr::unary &expr)
{
	std::optional<lox::object> maybe_right = expr.right->visit(*this);
	if (!maybe_right) return std::nullopt;
	const auto &right = *maybe_right;

	switch (expr.op.type)
	{
		using enum lox::token_type;

		case BANG: return lox::object{.value = !right.is_truthy()};

		case MINUS:
		{
			auto right_num = right.get_number();
			if (!right_num) return error(expr.op, u8"Operand must be a number.");
			return lox::object{.value = -*right_num};
		}

		default: return lox::object{};
	}
}

std::optional<lox::object> lox::evaluator::operator()(
  const lox::expr::variable &expr)
{
	if (auto it = environment->find(expr.name); it != environment->end())
		return it->second;
	else
		return std::nullopt;
}

std::optional<std::u8string> lox::evaluator::operator()(
  const lox::stmt::block &stmt)
{
	environment = lox::environment::make(environment);
	for (const auto &s : stmt.statements)
	{
		if (!s || !s->visit(*this))
		{
			environment = environment->enclosing;
			return std::nullopt;
		}
	}
	environment = environment->enclosing;
	return std::make_optional<std::u8string>();
}

std::optional<std::u8string> lox::evaluator::operator()(
  const lox::stmt::expr &stmt)
{
	if (auto expr = stmt.expression->visit(*this); !expr)
		return std::nullopt;
	else
		return expr->to_string() + u8"\n";
}

std::optional<std::u8string> lox::evaluator::operator()(
  const lox::stmt::branch &stmt)
{
	if (auto condition = stmt.condition->visit(*this); !condition)
		return std::nullopt;
	else if (condition->is_truthy())
		return stmt.then_brach->visit(*this);
	else if (stmt.else_brach)
		return stmt.else_brach->visit(*this);
	else
		return std::make_optional<std::u8string>();
}

std::optional<std::u8string> lox::evaluator::operator()(
  const lox::stmt::print &stmt)
{
	std::optional<lox::object> maybe_value = stmt.expression->visit(*this);
	if (!maybe_value) return std::nullopt;

	std::cout << lox::as_astring_view(maybe_value->to_string()) << "\n";

	return std::make_optional<std::u8string>();
}

std::optional<std::u8string> lox::evaluator::operator()(
  const lox::stmt::var &stmt)
{
	if (stmt.init)
	{
		std::optional<lox::object> maybe_value = stmt.init->visit(*this);
		if (!maybe_value) return std::nullopt;
		environment->emplace(stmt.name, std::move(*maybe_value));
	}
	else
	{
		environment->emplace(stmt.name, lox::object{});
	}

	return std::make_optional<std::u8string>();
}

std::optional<std::u8string> lox::evaluator::operator()(
  const lox::stmt::loop &stmt)
{
	std::optional<lox::object> maybe_condition = stmt.condition->visit(*this);

	std::u8string result;

	for (; maybe_condition && maybe_condition->is_truthy();
	     maybe_condition = stmt.condition->visit(*this))
	{
		std::optional<std::u8string> maybe_body = stmt.body->visit(*this);
		if (!maybe_body) return std::nullopt;
		result += *maybe_body;
	}

	if (!maybe_condition) return std::nullopt;

	return result;
}
