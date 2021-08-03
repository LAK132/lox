#include "evaluator.hpp"

std::nullopt_t lox::evaluator::error(const lox::token &token,
                                     std::u8string_view message)
{
	interpreter.error(token, message);
	return std::nullopt;
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
