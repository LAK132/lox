#ifndef LOX_EVALUATOR_HPP
#define LOX_EVALUATOR_HPP

#include "expr.hpp"
#include "interpreter.hpp"
#include "object.hpp"

#include <optional>
#include <string>

namespace lox
{
	struct evaluator
	{
		lox::interpreter &interpreter;

		std::nullopt_t error(const lox::token &token, std::u8string_view message);

		std::optional<lox::object> operator()(const lox::expr::binary &expr);
		std::optional<lox::object> operator()(const lox::expr::grouping &expr);
		std::optional<lox::object> operator()(const lox::expr::literal &expr);
		std::optional<lox::object> operator()(const lox::expr::unary &expr);
	};
}

#endif
