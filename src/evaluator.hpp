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

		inline evaluator(lox::interpreter &i)
		: interpreter(i), environment(i.global_environment)
		{
		}

		std::nullopt_t error(const lox::token &token, std::u8string_view message);

		std::optional<lox::object> operator()(const lox::expr::assign &expr);
		std::optional<lox::object> operator()(const lox::expr::binary &expr);
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
	};
}

#endif
