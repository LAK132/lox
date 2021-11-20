#ifndef LOX_EVALUATOR_HPP
#define LOX_EVALUATOR_HPP

#include "environment.hpp"
#include "expr.hpp"
#include "interpreter.hpp"
#include "object.hpp"
#include "stmt.hpp"

#include <optional>
#include <source_location>
#include <string>

namespace lox
{
	struct evaluator
	{
		lox::interpreter &interpreter;
		lox::environment_ptr environment;
		std::optional<lox::object> block_ret_value;

		inline evaluator(lox::interpreter &i)
		: interpreter(i),
		  environment(i.global_environment),
		  block_ret_value(std::nullopt)
		{
		}

		std::nullopt_t error(
		  const lox::token &token,
		  std::u8string_view message,
		  const std::source_location srcloc = std::source_location::current());

		std::optional<std::u8string> execute_block(
		  std::span<const lox::stmt_ptr> statements,
		  const lox::environment_ptr &env);

		template<typename T>
		std::optional<lox::object> find_variable(const lox::token &name,
		                                         const T &expr);

		std::optional<lox::object> operator()(const lox::expr::assign &expr);
		std::optional<lox::object> operator()(const lox::expr::binary &expr);
		std::optional<lox::object> operator()(const lox::expr::call &expr);
		std::optional<lox::object> operator()(const lox::expr::get &expr);
		std::optional<lox::object> operator()(const lox::expr::grouping &expr);
		std::optional<lox::object> operator()(const lox::expr::literal &expr);
		std::optional<lox::object> operator()(const lox::expr::logical &expr);
		std::optional<lox::object> operator()(const lox::expr::set &expr);
		std::optional<lox::object> operator()(
		  const lox::expr::super_keyword &expr);
		std::optional<lox::object> operator()(const lox::expr::this_keyword &expr);
		std::optional<lox::object> operator()(const lox::expr::unary &expr);
		std::optional<lox::object> operator()(const lox::expr::variable &expr);

		std::optional<std::u8string> operator()(const lox::stmt::block &stmt);
		std::optional<std::u8string> operator()(const lox::stmt::klass &stmt);
		std::optional<std::u8string> operator()(const lox::stmt::expr &stmt);
		std::optional<std::u8string> operator()(const lox::stmt::branch &stmt);
		std::optional<std::u8string> operator()(const lox::stmt::print &stmt);
		std::optional<std::u8string> operator()(const lox::stmt::var &stmt);
		std::optional<std::u8string> operator()(const lox::stmt::loop &stmt);
		std::optional<std::u8string> operator()(
		  const lox::stmt::function_ptr &stmt);
		std::optional<std::u8string> operator()(const lox::stmt::ret &stmt);
	};
}

template<typename T>
std::optional<lox::object> lox::evaluator::find_variable(
  const lox::token &name, const T &expr)
{
	if (std::optional<size_t> maybe_distance = interpreter.find(expr);
	    maybe_distance)
	{
		if (auto it = environment->find(name, *maybe_distance); it != nullptr)
			return *it;
		else
			return error(name,
			             u8"Undefined local variable '" +
			               std::u8string(name.lexeme) + u8"'.");
	}
	else
	{
		if (auto it = interpreter.global_environment->find(name); it != nullptr)
			return *it;
		else
			return error(name,
			             u8"Undefined global variable '" +
			               std::u8string(name.lexeme) + u8"'.");
	}
}

#endif
