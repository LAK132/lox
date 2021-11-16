#ifndef LOX_EVALUATOR_HPP
#define LOX_EVALUATOR_HPP

#include "environment.hpp"
#include "expr.hpp"
#include "interpreter.hpp"
#include "object.hpp"
#include "stmt.hpp"

#include <optional>
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

		std::nullopt_t error(const lox::token &token, std::u8string_view message);

		std::optional<std::u8string> execute_block(
		  std::span<const lox::stmt_ptr> statements,
		  const lox::environment_ptr &env);

		std::optional<lox::object> operator()(const lox::expr::assign &expr);
		std::optional<lox::object> operator()(const lox::expr::binary &expr);
		std::optional<lox::object> operator()(const lox::expr::call &expr);
		std::optional<lox::object> operator()(const lox::expr::grouping &expr);
		std::optional<lox::object> operator()(const lox::expr::literal &expr);
		std::optional<lox::object> operator()(const lox::expr::logical &expr);
		std::optional<lox::object> operator()(const lox::expr::unary &expr);
		std::optional<lox::object> operator()(const lox::expr::variable &expr);

		std::optional<std::u8string> operator()(const lox::stmt::block &stmt);
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

#endif
