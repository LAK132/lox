#include "evaluator.hpp"

#include "callable.hpp"

#include <iostream>

std::nullopt_t lox::evaluator::error(const lox::token &token,
                                     std::u8string_view message,
                                     const std::source_location srcloc)
{
	interpreter.error(token, message, srcloc);
	return std::nullopt;
}

std::optional<std::u8string> lox::evaluator::execute_block(
  std::span<const lox::stmt_ptr> statements, const lox::environment_ptr &env)
{
	lox::environment_ptr previous = std::exchange(environment, env);

	for (const auto &s : statements)
	{
		if (!s || !s->visit(*this))
		{
			environment = previous;
			return std::nullopt;
		}

		if (block_ret_value) break;
	}

	environment = previous;
	return std::make_optional<std::u8string>();
}

std::optional<lox::object> lox::evaluator::operator()(
  const lox::expr::assign &expr)
{
	std::optional<lox::object> maybe_value = expr.value->visit(*this);
	if (!maybe_value) return std::nullopt;

	if (std::optional<size_t> maybe_distance = interpreter.find(expr);
	    maybe_distance)
	{
		if (auto it =
		      environment->replace(expr.name, *maybe_value, *maybe_distance);
		    it != nullptr)
			return *it;
	}
	else
	{
		if (auto it =
		      interpreter.global_environment->replace(expr.name, *maybe_value);
		    it != nullptr)
			return *it;
	}

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

		case EQUAL_EQUAL: return lox::object{left == right};

		case BANG_EQUAL: return lox::object{left != right};

		case GREATER:
		{
			auto left_num  = left.get_number();
			auto right_num = right.get_number();
			if (!left_num || !right_num)
				return error(expr.op, u8"Operands must be numbers.");
			return lox::object{*left_num > *right_num};
		}

		case GREATER_EQUAL:
		{
			auto left_num  = left.get_number();
			auto right_num = right.get_number();
			if (!left_num || !right_num)
				return error(expr.op, u8"Operands must be numbers.");
			return lox::object{*left_num >= *right_num};
		}

		case LESS:
		{
			auto left_num  = left.get_number();
			auto right_num = right.get_number();
			if (!left_num || !right_num)
				return error(expr.op, u8"Operands must be numbers.");
			return lox::object{*left_num < *right_num};
		}

		case LESS_EQUAL:
		{
			auto left_num  = left.get_number();
			auto right_num = right.get_number();
			if (!left_num || !right_num)
				return error(expr.op, u8"Operands must be numbers.");
			return lox::object{*left_num <= *right_num};
		}

		case PLUS:
		{
			{
				auto left_num  = left.get_number();
				auto right_num = right.get_number();
				if (left_num && right_num) return lox::object{*left_num + *right_num};
			}
			{
				auto left_string  = left.get_string();
				auto right_string = right.get_string();
				if (left_string && right_string)
					return lox::object{*left_string + *right_string};
			}
			return error(expr.op, u8"Operands must be two numbers or two strings.");
		}

		case MINUS:
		{
			auto left_num  = left.get_number();
			auto right_num = right.get_number();
			if (!left_num || !right_num)
				return error(expr.op, u8"Operands must be numbers.");
			return lox::object{*left_num - *right_num};
		}

		case SLASH:
		{
			auto left_num  = left.get_number();
			auto right_num = right.get_number();
			if (!left_num || !right_num)
				return error(expr.op, u8"Operands must be numbers.");
			return lox::object{*left_num / *right_num};
		}

		case STAR:
		{
			auto left_num  = left.get_number();
			auto right_num = right.get_number();
			if (!left_num || !right_num)
				return error(expr.op, u8"Operands must be numbers.");
			return lox::object{*left_num * *right_num};
		}
	}
	return lox::object{};
}

std::optional<lox::object> lox::evaluator::operator()(
  const lox::expr::call &expr)
{
	std::optional<lox::object> maybe_callee = expr.callee->visit(*this);
	if (!maybe_callee) return std::nullopt;

	const lox::callable *maybe_callable = maybe_callee->get_callable();
	if (!maybe_callable)
		return error(expr.paren, u8"Can only call functions and classes.");

	const lox::callable &callable = *maybe_callable;

	std::vector<lox::object> arguments;
	arguments.reserve(expr.arguments.size());
	for (const auto &argument : expr.arguments)
	{
		std::optional<lox::object> maybe_argument = argument->visit(*this);
		if (!maybe_argument) return std::nullopt;
		arguments.push_back(*maybe_argument);
	}

	if (arguments.size() != callable.arity())
		return error(
		  expr.paren,
		  lox::as_u8string_view("Expected " + std::to_string(callable.arity()) +
		                        " arguments but got " +
		                        std::to_string(arguments.size()) + "."));

	return callable(interpreter, std::move(arguments));
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

		case BANG: return lox::object{!right.is_truthy()};

		case MINUS:
		{
			auto right_num = right.get_number();
			if (!right_num) return error(expr.op, u8"Operand must be a number.");
			return lox::object{-*right_num};
		}

		default: return lox::object{};
	}
}

std::optional<lox::object> lox::evaluator::operator()(
  const lox::expr::variable &expr)
{
	if (std::optional<size_t> maybe_distance = interpreter.find(expr);
	    maybe_distance)
	{
		if (auto it = environment->find(expr.name, *maybe_distance); it != nullptr)
			return *it;
		else
			return error(expr.name,
			             u8"Undefined local variable '" +
			               std::u8string(expr.name.lexeme) + u8"'.");
	}
	else
	{
		if (auto it = interpreter.global_environment->find(expr.name);
		    it != nullptr)
			return *it;
		else
			return error(expr.name,
			             u8"Undefined global variable '" +
			               std::u8string(expr.name.lexeme) + u8"'.");
	}
}

std::optional<std::u8string> lox::evaluator::operator()(
  const lox::stmt::block &stmt)
{
	return execute_block(std::span(stmt.statements),
	                     lox::environment::make(environment));
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
		return stmt.then_branch->visit(*this);
	else if (stmt.else_branch)
		return stmt.else_branch->visit(*this);
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
		if (block_ret_value) break;
		result += *maybe_body;
	}

	if (!maybe_condition) return std::nullopt;

	return result;
}

std::optional<std::u8string> lox::evaluator::operator()(
  const lox::stmt::function_ptr &stmt)
{
	environment->emplace(stmt->name.lexeme,
	                     lox::object{std::make_shared<lox::callable>(
	                       lox::callable::make_interpreted({
	                         .func    = stmt,
	                         .closure = environment,
	                       }))});
	return std::make_optional<std::u8string>();
}

std::optional<std::u8string> lox::evaluator::operator()(
  const lox::stmt::ret &stmt)
{
	block_ret_value =
	  stmt.value ? stmt.value->visit(*this) : std::make_optional<lox::object>();
	if (block_ret_value)
		return std::make_optional<std::u8string>();
	else
		return std::nullopt;
}
