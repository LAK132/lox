#ifndef LOX_EXPR_HPP
#define LOX_EXPR_HPP

#include "object.hpp"
#include "token.hpp"

#include <memory>
#include <variant>

namespace lox
{
	struct expr
	{
		using expr_ptr = std::unique_ptr<lox::expr>;

		struct binary
		{
			expr_ptr left;
			lox::token op;
			expr_ptr right;
		};

		struct grouping
		{
			expr_ptr expression;
		};

		struct literal
		{
			lox::object value;
		};

		struct unary
		{
			lox::token op;
			expr_ptr right;
		};

		std::variant<binary, grouping, literal, unary> value;

		static expr_ptr make_binary(binary &&expr);
		static expr_ptr make_grouping(grouping &&expr);
		static expr_ptr make_literal(literal &&expr);
		static expr_ptr make_unary(unary &&expr);

		template<typename F>
		inline auto visit(F &&f)
		{
			return std::visit(f, value);
		}

		template<typename F>
		inline auto visit(F &&f) const
		{
			return std::visit(f, value);
		}
	};

	using expr_ptr = lox::expr::expr_ptr;
}

#endif
